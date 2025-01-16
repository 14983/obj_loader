#include "obj_loader.h"
#include <GL/glew.h>
#include <GLFW/glfw3.h>
#include <iostream>
#include <vector>
#include <cmath>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include "imgui_util.h"

objLoader obj;
const int window_width = 1600;
const int window_height = 900;
glm::vec3 light_pos = glm::vec3(0.0f, 0.0f, 5.0f);
glm::vec3 light_color = glm::vec3(1.0f, 1.0f, 1.0f);

// shaders
std::string vertexShaderSource, fragmentShaderSource;

// shader loader
std::string loadShaderFromFile(const std::string& filePath) {
    std::ifstream shaderFile;
    std::stringstream shaderStream;

    shaderFile.exceptions(std::ifstream::failbit | std::ifstream::badbit);

    try {
        shaderFile.open(filePath);
        shaderStream << shaderFile.rdbuf();
        shaderFile.close();
        return shaderStream.str();
    } catch (std::ifstream::failure& e) {
        std::cerr << "ERROR::SHADER::FILE_NOT_SUCCESSFULLY_READ: " << e.what() << std::endl;
        return "";
    }
}

// compile shader
GLuint compileShader(GLenum type, const std::string &source) {
    GLuint shader = glCreateShader(type);
    const char* cStr = source.c_str();
    glShaderSource(shader, 1, &cStr, nullptr);
    glCompileShader(shader);
    GLint success;
    glGetShaderiv(shader, GL_COMPILE_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetShaderInfoLog(shader, 512, nullptr, infoLog);
        std::cerr << "Shader compilation failed: " << infoLog << std::endl;
    }
    return shader;
}

// link shader
GLuint createShaderProgram(const std::string &vertexSource, const std::string &fragmentSource) {
    GLuint vertexShader = compileShader(GL_VERTEX_SHADER, vertexSource);
    GLuint fragmentShader = compileShader(GL_FRAGMENT_SHADER, fragmentSource);
    GLuint shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    GLint success;
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        char infoLog[512];
        glGetProgramInfoLog(shaderProgram, 512, nullptr, infoLog);
        std::cerr << "Shader program linking failed: " << infoLog << std::endl;
    }
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);
    return shaderProgram;
}

int main() {
    if (!glfwInit()) {
        std::cerr << "Failed to initialize GLFW" << std::endl;
        return -1;
    }

    // set major and minor version
    glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 3);
    glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 3);
    glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

    GLFWwindow* window = glfwCreateWindow(window_width, window_height, "obj_loader module", NULL, NULL);
    if (!window) {
        std::cerr << "Failed to create GLFW window" << std::endl;
        glfwTerminate();
        return -1;
    }

    glfwMakeContextCurrent(window);

    glewExperimental = GL_TRUE;
    if (glewInit() != GLEW_OK) {
        std::cerr << "Failed to initialize GLEW" << std::endl;
        return -1;
    }

    glViewport(0, 0, window_width, window_height);

    obj.load("res/model/shuttle/shuttle.obj");

    const float *vertices = obj.getVBO();
    size_t vertices_size = obj.getVBOSize();

    GLuint VAO, VBO;
    glGenVertexArrays(1, &VAO);
    glGenBuffers(1, &VBO);

    glBindVertexArray(VAO);
    glBindBuffer(GL_ARRAY_BUFFER, VBO);
    glBufferData(GL_ARRAY_BUFFER, vertices_size, vertices, GL_STATIC_DRAW);

    glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)0);
    glEnableVertexAttribArray(0);
    glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, 8 * sizeof(float), (void*)(3 * sizeof(float)));
    glEnableVertexAttribArray(1);

    glBindBuffer(GL_ARRAY_BUFFER, 0);
    glBindVertexArray(0);

    // create shader program
    vertexShaderSource = loadShaderFromFile("res/shader/model.vs");
    fragmentShaderSource = loadShaderFromFile("res/shader/model.fs");
    GLuint shaderProgram = createShaderProgram(vertexShaderSource, fragmentShaderSource);

    glEnable(GL_DEPTH_TEST);

    setupImGUI(window);

    while (!glfwWindowShouldClose(window)) {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        // user shader program
        glUseProgram(shaderProgram);

        // set matrices
        glm::mat4 model = glm::mat4(1.0f);
        glm::mat4 view = glm::lookAt(glm::vec3(0.0f, 0.0f, 10.0f), glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f, 1.0f, 0.0f));
        glm::mat4 projection = glm::perspective(glm::radians(45.0f), (float)window_width / window_height, 0.1f, 100.0f);

        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "model"), 1, GL_FALSE, &model[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "view"), 1, GL_FALSE, &view[0][0]);
        glUniformMatrix4fv(glGetUniformLocation(shaderProgram, "projection"), 1, GL_FALSE, &projection[0][0]);

        // set light and color
        glUniform3f(glGetUniformLocation(shaderProgram, "lightPos"), light_pos.x, light_pos.y, light_pos.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "lightColor"), light_color.x, light_color.y, light_color.z);
        glUniform3f(glGetUniformLocation(shaderProgram, "objectColor"), 0.5f, 0.5f, 0.8f);

        // bind VAO and draw
        glBindVertexArray(VAO);
        glDrawArrays(GL_TRIANGLES, 0, vertices_size / sizeof(float) / 8);

        // imgui
        // light control panel
        setupImGUIFrame();
        ImGui::SetNextWindowPos(ImVec2(0, 0));
        ImGui::SetNextWindowSize(ImVec2(200, window_height / 2));
        ImGui::Begin("Light Control Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        ImGui::SliderFloat("Light X", &light_pos.x, -10.0f, 10.0f);
        ImGui::SliderFloat("Light Y", &light_pos.y, -10.0f, 10.0f);
        ImGui::SliderFloat("Light Z", &light_pos.z, -10.0f, 10.0f);
        ImGui::End();

        // model control panel (for future use)
        ImGui::SetNextWindowPos(ImVec2(0, window_height / 2));
        ImGui::SetNextWindowSize(ImVec2(200, window_height / 2));
        ImGui::Begin("Model Control Panel", nullptr, ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove | ImGuiWindowFlags_NoCollapse);
        static char modelPath[128] = "";
        ImGui::InputText("Model Path", modelPath, 128);
        if (ImGui::Button("Load Model")) {
            std::cout << "Loading model: " << modelPath << std::endl;
            if (!obj.load(modelPath)) {
                ImGui::OpenPopup("Error");
            } else {
                vertices = obj.getVBO();
                vertices_size = obj.getVBOSize();
            }
        }
        if (ImGui::BeginPopupModal("Error", nullptr, ImGuiWindowFlags_AlwaysAutoResize)) {
            ImGui::Text("Error loading model");
            if (ImGui::Button("OK")) {
                ImGui::CloseCurrentPopup();
            }
            ImGui::EndPopup();
        }
        ImGui::End();
        endImGUIFrame();

        glfwSwapBuffers(window);
        glfwPollEvents();
    }

    glDeleteVertexArrays(1, &VAO);
    glDeleteBuffers(1, &VBO);
    glDeleteProgram(shaderProgram);

    clearImGUIContext();
    glfwTerminate();
    return 0;
}

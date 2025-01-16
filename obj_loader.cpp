#include "obj_loader.h"

#define push_vec3(src, tar) \
    tar.push_back(src.x); \
    tar.push_back(src.y); \
    tar.push_back(src.z);

#define push_vec2(src, tar) \
    tar.push_back(src.x); \
    tar.push_back(src.y);

bool objLoader::hasNormal() {
    return !(std::get<2>(this -> faces[0][0]) == -1);
}

bool objLoader::hasTexcoord() {
    return !(std::get<1>(this -> faces[0][0]) == -1);
}

bool objLoader::load(const std::string& filename) {
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }
    std::string line;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#' || line[0] == ' ') continue;
        std::istringstream iss(line);
        std::string prefix;
        iss >> prefix;
        if (prefix == "v") {
            glm::vec3 vertex;
            iss >> vertex.x >> vertex.y >> vertex.z;
            vertices.push_back(vertex);
        } else if (prefix == "vn") {
            glm::vec3 norm;
            iss >> norm.x >> norm.y >> norm.z;
            normals.push_back(norm);
        } else if (prefix == "vt") {
            glm::vec2 tex;
            iss >> tex.x >> tex.y;
            texcoord.push_back(tex);
        } else if (prefix == "f") {
            faces.resize(faces.size() + 1);
            std::string vertexInfo;
            while (iss >> vertexInfo) {
                vertexInfo += "/";
                std::vector<int> v_n_t_index;
                while (vertexInfo.size() > 0) {
                    size_t next_idx = vertexInfo.find("/");
                    if (next_idx == std::string::npos) next_idx = vertexInfo.size();
                    if (next_idx == 0) v_n_t_index.push_back(-1);
                    else v_n_t_index.push_back(std::stoi(vertexInfo.substr(0, next_idx)) - 1);
                    vertexInfo = vertexInfo.substr(next_idx + 1);
                }
                while (v_n_t_index.size() < 3) v_n_t_index.push_back(-1);
                faces[faces.size()-1].push_back(std::make_tuple(v_n_t_index[0], v_n_t_index[1], v_n_t_index[2]));
            }
        } else {
            std::cerr << "Unsupported format:" << prefix << std::endl;
        }
    }
    file.close();
#if OBJ_LOADER_DBG
    // print vertices
    std::cout << "Vertices: " << std::endl;
    for (auto& vertex : vertices) {
        std::cout << "(" << vertex.x << ", " << vertex.y << ", " << vertex.z << ") ";
    }
    std::cout << std::endl;
    // print normals
    std::cout << "Normals: " << std::endl;
    if (hasNormal()) {
        for (auto& norm : normals) {
            std::cout << "(" << norm.x << ", " << norm.y << ", " << norm.z << ") ";
        }
        std::cout << std::endl;
    }
    // print texcoord
    std::cout << "Texcoord: " << std::endl;
    if (hasTexcoord()) {
        for (auto& tex : texcoord) {
            std::cout << "(" << tex.x << ", " << tex.y << ") ";
        }
        std::cout << std::endl;
    }
    // print faces
    std::cout << "Faces: " << std::endl;
    std::cout << this -> hasNormal() << " " << this -> hasTexcoord() << std::endl;
    for (auto& face : faces) {
        for (auto& vertex : face) {
            std::cout << "(" << std::get<0>(vertex) << ", " << std::get<1>(vertex) << ", " << std::get<2>(vertex) << ") ";
        }
        std::cout << std::endl;
    }
#endif
    // construct vbo
    std::vector<float> tmp_vbo(0);
    for (auto& face : faces) {
        std::vector<std::tuple<int, int, int>> face_vertices(3);
        face_vertices[0] = face[0];
        for (size_t i = 1; i + 1 < face.size(); i++) {
            face_vertices[1] = face[i];
            face_vertices[2] = face[i+1];
            // construct triangle
            glm::vec3 default_normal = glm::normalize(
                glm::cross(
                    vertices[std::get<0>(face_vertices[1])] - vertices[std::get<0>(face_vertices[0])],
                    vertices[std::get<0>(face_vertices[2])] - vertices[std::get<0>(face_vertices[1])]
                )
            );
            for (int j = 0; j < 3; j++) {
                push_vec3(vertices[std::get<0>(face_vertices[j])], tmp_vbo);
                if (hasNormal()) {
                    push_vec3(normals[std::get<2>(face_vertices[j])], tmp_vbo);
                } else {
                    push_vec3(default_normal, tmp_vbo);
                }
                if (hasTexcoord()) {
                    push_vec2(texcoord[std::get<1>(face_vertices[j])], tmp_vbo);
                } else {
                    push_vec2(glm::vec2(0.0f, 0.0f), tmp_vbo);
                }
            }
        }
    }
    // copy to vbo
    this -> vbo_size = tmp_vbo.size() * sizeof(float);
    if (this -> vbo) delete[] this -> vbo;
    this -> vbo = new float[tmp_vbo.size()];
    for (size_t i = 0; i < tmp_vbo.size(); i++)
        this -> vbo[i] = tmp_vbo[i];
#if OBJ_LOADER_DBG
    // print tmp_vbo
    for (size_t i = 0; i < tmp_vbo.size(); i += 8) {
        std::cout << "v: " << tmp_vbo[i] << " " << tmp_vbo[i+1] << " " << tmp_vbo[i+2] << std::endl;
        std::cout << "n: " << tmp_vbo[i+3] << " " << tmp_vbo[i+4] << " " << tmp_vbo[i+5] << std::endl;
        std::cout << "t: " << tmp_vbo[i+6] << " " << tmp_vbo[i+7] << std::endl;
    }
#endif
    return true;
}

const float* objLoader::getVBO() {
    return this -> vbo;
}

size_t objLoader::getVBOSize() {
    return this -> vbo_size;
}

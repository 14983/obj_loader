#include "obj_loader.h"

#define push_vec3(src, tar) \
    tar.push_back(src.x); \
    tar.push_back(src.y); \
    tar.push_back(src.z);

#define push_vec2(src, tar) \
    tar.push_back(src.x); \
    tar.push_back(src.y);

objLoader::~objLoader() {
    if (this -> vbo) delete[] this -> vbo;
}

bool objLoader::hasNormal() {
    return !(std::get<2>(this -> faces[0][0]) == -1);
}

bool objLoader::hasTexcoord() {
    return !(std::get<1>(this -> faces[0][0]) == -1);
}

bool objLoader::load(const std::string& filename) {
    if (this -> vbo) delete[] this -> vbo;
    this -> vbo_size = 0;
    vertices.clear();
    normals.clear();
    texcoord.clear();
    faces.clear();
    material_lib.materials.clear();
    group_index.clear();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        this -> vbo = nullptr;
        return false;
    }
    std::string line;
    std::string current_group = "default";
    material current_material;
    // a group is activated if it has faces
    bool group_activated = false;
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
            if (!group_activated) {
                group_activated = true;
                group_index.push_back(std::make_tuple(faces.size(), current_group, current_material));
            }
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
        } else if (prefix == "mtllib") {
            std::string mtl_filename;
            iss >> mtl_filename;
            // construct mtl file path
            size_t last_slash_pos = filename.find_last_of("/");
            std::string mtl_path = filename.substr(0, last_slash_pos + 1) + mtl_filename;
            std::cout << "Loading material library: " << mtl_path << std::endl;
            // load mtl file
            this -> material_lib.load(mtl_path);
        } else if (prefix == "usemtl") {
            std::string material_name;
            iss >> material_name;
            if ((this -> material_lib.materials).find(material_name) == (this -> material_lib.materials).end()) {
                std::cerr << "Material not found: " << material_name << std::endl;
                current_material = material();
            } else {
                current_material = this -> material_lib.materials[material_name];
            }
        } else if (prefix == "g") {
            iss >> current_group;
            group_activated = false;
        } else {
            std::cerr << "Unsupported format:" << prefix << std::endl;
        }
    }
    file.close();
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
    this -> vbo = new float[tmp_vbo.size()];
    for (size_t i = 0; i < tmp_vbo.size(); i++)
        this -> vbo[i] = tmp_vbo[i];
    return true;
}

const float* objLoader::getVBO() {
    return this -> vbo;
}

size_t objLoader::getVBOSize() {
    return this -> vbo_size;
}

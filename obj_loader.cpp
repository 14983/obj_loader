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
            // std::cerr << "Unsupported format:" << prefix << std::endl;
        }
    }
    file.close();
    // construct vbo
    std::vector<float> tmp_vbo(0);
    size_t group_idx = 0;
    for (size_t i = 0; i < faces.size(); i++) {
        auto face = faces[i];
        // convert group index to vbo offset
        if (group_idx < group_index.size() && std::get<0>(group_index[group_idx]) == (int)i) {
            group_index[group_idx] = std::make_tuple(tmp_vbo.size() / 8, std::get<1>(group_index[group_idx]), std::get<2>(group_index[group_idx]));
            group_idx++;
        }
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

const std::vector<std::tuple<int, std::string, material>>& objLoader::getGroupIndices() {
    return this -> group_index;
}

void objLoader::applyMaterial(size_t idx, const material& mat) {
    this -> group_index[idx] = std::make_tuple(std::get<0>(this -> group_index[idx]), std::get<1>(this -> group_index[idx]), mat);
}

bool objLoader::save(const std::string& filename) {
    // obj file
    std::ofstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }
    // mtl file
    size_t last_slash_pos = filename.find_last_of("/");
    std::string mtl_filename = filename.substr(0, last_slash_pos + 1) + this -> material_lib.name + ".mtl";
    std::ofstream mtl_file(mtl_filename);
    if (!mtl_file.is_open()) {
        std::cerr << "Cannot open file: " << mtl_filename << std::endl;
        file.close();
        return false;
    }
    // write vertices
    file << "mtllib " << this -> material_lib.name << ".mtl" << std::endl;
    for (size_t i = 0; i < this -> getVBOSize() / sizeof(float); i += 8)
        file << "v " << vbo[i] << " " << vbo[i+1] << " " << vbo[i+2] << std::endl;
    // write normals
    // not implemented
    // write texcoords
    // not implemented
    // write faces
    for (size_t i = 0; i < this -> group_index.size(); i++) {
        auto group = this -> group_index[i];
        file << "g " << std::get<1>(group) << std::endl;
        file << "usemtl " << std::get<1>(group)+"-material" << std::endl;
        size_t start_index = std::get<0>(this -> group_index[i]); // count of vertex
            size_t end_index = (i == this -> group_index.size() - 1) ? (this -> getVBOSize()) / sizeof(float) / 8 : std::get<0>(this -> group_index[i + 1]); // count of vertex
        for (size_t j = start_index; j < end_index; j += 3) {
            file << "f " << (j+1) << " " << (j+2) << " " << (j+3) << std::endl;
        }
    }
    file.close();
    // write mtl file
    for (size_t i = 0; i < this -> group_index.size(); i++) {
        auto group = this -> group_index[i];
        auto name = std::get<1>(group)+"-material";
        auto mat = std::get<2>(group);
        mtl_file << "newmtl " << name << std::endl;
        mtl_file << "Ka " << mat.ambient.x << " " << mat.ambient.y << " " << mat.ambient.z << std::endl;
        mtl_file << "Kd " << mat.diffuse.x << " " << mat.diffuse.y << " " << mat.diffuse.z << std::endl;
        mtl_file << "Ks " << mat.specular.x << " " << mat.specular.y << " " << mat.specular.z << std::endl;
        mtl_file << "Ns " << mat.shininess << std::endl;
        mtl_file << std::endl;
    }
    mtl_file.close();
    return true;
}

void objLoader::applyTransform(size_t idx, const glm::mat4& transform) {
    auto group = this -> group_index[idx];
    size_t start_index = std::get<0>(group) * 8; // count of vertex
    size_t end_index = (idx == this -> group_index.size() - 1) ? (this -> getVBOSize()) / sizeof(float) : (std::get<0>(this -> group_index[idx + 1]) * 8); // count of vertex
    for (size_t i = start_index; i < end_index; i += 8) {
        glm::vec3 vertex(vbo[i], vbo[i+1], vbo[i+2]);
        glm::vec3 normal(vbo[i+3], vbo[i+4], vbo[i+5]);
        glm::vec2 texcoord(vbo[i+6], vbo[i+7]);
        vertex = transform * glm::vec4(vertex, 1.0f);
        normal = glm::normalize(glm::transpose(glm::inverse(glm::mat3(transform))) * normal);
        vbo[i] = vertex.x;
        vbo[i+1] = vertex.y;
        vbo[i+2] = vertex.z;
        vbo[i+3] = normal.x;
        vbo[i+4] = normal.y;
        vbo[i+5] = normal.z;
        vbo[i+6] = texcoord.x;
        vbo[i+7] = texcoord.y;
    }
}
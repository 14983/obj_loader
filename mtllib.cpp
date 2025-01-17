#include "mtllib.h"

bool mtl_file::load(const std::string& filename, bool append) {
    if (!append) this -> materials.clear();
    std::ifstream file(filename);
    if (!file.is_open()) {
        std::cerr << "Cannot open file: " << filename << std::endl;
        return false;
    }
    std::string line;
    std::string prefix;
    std::string matname;
    material mat;
    bool dirty = false;
    while (std::getline(file, line)) {
        if (line.empty() || line[0] == '#') {
            if (dirty) {
                this -> append(matname, mat);
                dirty = false;
            }
            continue;
        }
        std::istringstream iss(line);
        iss >> prefix;
        if (prefix == "newmtl") {
            if (dirty) {
                this -> append(matname, mat);
                dirty = false;
            }
            iss >> matname;
            mat = material();
            dirty = true;
        } else if (prefix == "Ka") {
            float r, g, b;
            iss >> r >> g >> b;
            mat.ambient = glm::vec3(r, g, b);
        } else if (prefix == "Kd") {
            float r, g, b;
            iss >> r >> g >> b;
            mat.diffuse = glm::vec3(r, g, b);
        } else if (prefix == "Ks") {
            float r, g, b;
            iss >> r >> g >> b;
            mat.specular = glm::vec3(r, g, b);
        } else if (prefix == "Ns") {
            float ns;
            iss >> ns;
            mat.shininess = ns;
        } else {
            std::cerr << "Unsupported material property: " << prefix << std::endl;
        }
    }
    if (dirty) this -> append(matname, mat);
    return true;
}

void mtl_file::append(const std::string& matname, const material &mat) {
    (this -> materials)[matname] = mat;
}

bool mtl_file::save(const std::string& filename) {
    return true;
}
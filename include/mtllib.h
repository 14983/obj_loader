#ifndef __MTLLIB_H__
#define __MTLLIB_H__

#include <glm/glm.hpp>
#include <string>
#include <map>
#include <fstream>
#include <iostream>
#include <sstream>

class material { // map unimplemented
public:
    glm::vec3 ambient;
    glm::vec3 diffuse;
    glm::vec3 specular;
    float shininess;
    material(glm::vec3 a = glm::vec3(0.0), glm::vec3 d = glm::vec3(1.0), glm::vec3 s = glm::vec3(0.0), float sh = 32.0)
        : ambient(a), diffuse(d), specular(s), shininess(sh) {}
    material(const material& m)
        : ambient(m.ambient), diffuse(m.diffuse), specular(m.specular), shininess(m.shininess) {}
};

class mtl_file {
public:
    std::string name;
    std::map<std::string, material> materials;
    mtl_file(const std::string& filename)
        : name(filename) {}
    bool load(const std::string& filename, bool append = false);
    void append(const std::string& matname, const material &mat);
    bool save(const std::string& filename);
};

#endif

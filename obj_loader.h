#ifndef __OBJ_LOADER_H__
#define __OBJ_LOADER_H__

#include <glm/glm.hpp>
#include <vector>
#include <tuple>
#include <string>
#include <iostream>
#include <fstream>
#include <sstream>

class objLoader {
public:
    objLoader(): vbo(NULL), vertices(0), normals(0), texcoord(0){}
    bool load(const std::string &filename);
    bool hasNormal();
    bool hasTexcoord();
    const float *getVBO();
    size_t getVBOSize();
private:
    float *vbo;
    size_t vbo_size;
    std::vector<glm::vec3> vertices;
    std::vector<glm::vec3> normals;
    std::vector<glm::vec2> texcoord;
    std::vector<std::vector<std::tuple<int, int, int>>> faces;
};

#endif

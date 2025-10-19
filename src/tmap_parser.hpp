#ifndef TMAP_PARSER_HPP
#define TMAP_PARSER_HPP

#include <vector>
#include <string>
#include <cstdint>

struct Vec3 {
    float x, y, z;
};

struct Vec2 {
    float u, v;
};

struct Mesh {
    std::string name;
    std::vector<Vec3> vertices;
    std::vector<Vec3> normals;
    std::vector<Vec2> uvs;
    std::string material;
};

struct TMAPData {
    uint32_t version;
    std::vector<Mesh> meshes;
    Vec3 spawnPosition;
    Vec3 spawnRotation;
    Vec3 mapOffset;
};

bool loadTMAP(const std::string& filename, TMAPData& outData);

#endif // TMAP_PARSER_HPP
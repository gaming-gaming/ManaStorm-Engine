#include "tmap_parser.hpp"
#include <fstream>
#include <iostream>

std::string readString(std::ifstream& file) {
    uint16_t length;
    file.read(reinterpret_cast<char*>(&length), sizeof(length));
    
    std::string str(length, '\0');
    file.read(&str[0], length);
    return str;
}

Vec3 readVec3(std::ifstream& file) {
    Vec3 v;
    file.read(reinterpret_cast<char*>(&v.x), sizeof(float));
    file.read(reinterpret_cast<char*>(&v.y), sizeof(float));
    file.read(reinterpret_cast<char*>(&v.z), sizeof(float));
    return v;
}

Vec2 readVec2(std::ifstream& file) {
    Vec2 v;
    file.read(reinterpret_cast<char*>(&v.u), sizeof(float));
    file.read(reinterpret_cast<char*>(&v.v), sizeof(float));
    return v;
}

Mesh readMesh(std::ifstream& file) {
    Mesh mesh;
    
    mesh.name = readString(file);
    
    // Read vertices
    uint32_t vertCount;
    file.read(reinterpret_cast<char*>(&vertCount), sizeof(vertCount));
    mesh.vertices.reserve(vertCount);
    for(uint32_t i = 0; i < vertCount; i++) {
        mesh.vertices.push_back(readVec3(file));
    }
    
    // Read normals
    uint32_t normalCount;
    file.read(reinterpret_cast<char*>(&normalCount), sizeof(normalCount));
    mesh.normals.reserve(normalCount);
    for(uint32_t i = 0; i < normalCount; i++) {
        mesh.normals.push_back(readVec3(file));
    }
    
    // Read UVs
    uint32_t uvCount;
    file.read(reinterpret_cast<char*>(&uvCount), sizeof(uvCount));
    mesh.uvs.reserve(uvCount);
    for(uint32_t i = 0; i < uvCount; i++) {
        mesh.uvs.push_back(readVec2(file));
    }
    
    // Read material
    mesh.material = readString(file);
    
    return mesh;
}

bool loadTMAP(const std::string& filename, TMAPData& outData) {
    std::cout << "TMAP: Loading " << filename << std::endl;
    
    std::ifstream file(filename, std::ios::binary);
    
    if(!file.is_open()) {
        std::cerr << "TMAP: Failed to open file" << std::endl;
        return false;
    }
    
    // Check magic bytes
    char magic[4];
    file.read(magic, 4);
    if(std::string(magic, 4) != "TMAP") {
        std::cerr << "TMAP: Invalid magic bytes" << std::endl;
        return false;
    }
    std::cout << "TMAP: Magic bytes OK" << std::endl;
    
    // Read version
    file.read(reinterpret_cast<char*>(&outData.version), sizeof(outData.version));
    std::cout << "TMAP: Version " << outData.version << std::endl;
    
    // Read mesh count
    uint32_t meshCount;
    file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
    std::cout << "TMAP: Mesh count: " << meshCount << std::endl;
    
    // Read meshes
    outData.meshes.reserve(meshCount);
    for(uint32_t i = 0; i < meshCount; i++) {
        Mesh mesh = readMesh(file);
        std::cout << "TMAP:   Mesh " << i << ": " << mesh.name 
                  << " (" << mesh.vertices.size() << " verts)" << std::endl;
        outData.meshes.push_back(mesh);
    }
    
    // Read spawn data
    outData.spawnPosition = readVec3(file);
    outData.spawnRotation = readVec3(file);
    outData.mapOffset = readVec3(file);
    
    std::cout << "TMAP: Spawn at (" << outData.spawnPosition.x << ", " 
              << outData.spawnPosition.y << ", " << outData.spawnPosition.z << ")" << std::endl;
    
    file.close();
    
    std::cout << "TMAP: Load successful!" << std::endl;
    return true;
}
#include <fstream>
#include <vector>
#include <string>
#include <cstdint>
#include <iostream>

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

TMAPData loadTMAP(const std::string& filename) {
    TMAPData data;
    std::ifstream file(filename, std::ios::binary);
    
    if(!file.is_open()) {
        std::cerr << "Failed to open " << filename << std::endl;
        return data;
    }
    
    // Check magic bytes
    char magic[4];
    file.read(magic, 4);
    if(std::string(magic, 4) != "TMAP") {
        std::cerr << "Invalid TMAP file: wrong magic bytes" << std::endl;
        return data;
    }
    
    // Read version
    file.read(reinterpret_cast<char*>(&data.version), sizeof(data.version));
    
    // Read mesh count
    uint32_t meshCount;
    file.read(reinterpret_cast<char*>(&meshCount), sizeof(meshCount));
    
    // Read meshes
    data.meshes.reserve(meshCount);
    for(uint32_t i = 0; i < meshCount; i++) {
        data.meshes.push_back(readMesh(file));
    }
    
    // Read spawn data
    data.spawnPosition = readVec3(file);
    data.spawnRotation = readVec3(file);
    data.mapOffset = readVec3(file);
    
    file.close();
    
    std::cout << "Loaded TMAP v" << data.version << " with " << meshCount << " meshes" << std::endl;
    
    return data;
}

// Test function to print loaded data
void printTMAPData(const TMAPData& data) {
    std::cout << "\n=== TMAP Data ===" << std::endl;
    std::cout << "Version: " << data.version << std::endl;
    std::cout << "Meshes: " << data.meshes.size() << std::endl;
    
    for(size_t i = 0; i < data.meshes.size(); i++) {
        const Mesh& m = data.meshes[i];
        std::cout << "\nMesh " << i << ": " << m.name << std::endl;
        std::cout << "  Material: " << m.material << std::endl;
        std::cout << "  Vertices: " << m.vertices.size() << std::endl;
        std::cout << "  Normals: " << m.normals.size() << std::endl;
        std::cout << "  UVs: " << m.uvs.size() << std::endl;
    }
    
    std::cout << "\nSpawn: (" << data.spawnPosition.x << ", " 
              << data.spawnPosition.y << ", " << data.spawnPosition.z << ")" << std::endl;
    std::cout << "Rotation: (" << data.spawnRotation.x << ", " 
              << data.spawnRotation.y << ", " << data.spawnRotation.z << ")" << std::endl;
}

// Convert mesh data to flat vertex array for OpenGL
// Returns vertices in format: [x,y,z, nx,ny,nz, u,v] per vertex
std::vector<float> meshToVertexArray(const Mesh& mesh) {
    std::vector<float> vertices;
    size_t vertCount = mesh.vertices.size();
    vertices.reserve(vertCount * 8); // 3 pos + 3 normal + 2 uv
    
    for(size_t i = 0; i < vertCount; i++) {
        // Position
        vertices.push_back(mesh.vertices[i].x);
        vertices.push_back(mesh.vertices[i].y);
        vertices.push_back(mesh.vertices[i].z);
        
        // Normal (use corresponding normal or first one if mismatch)
        size_t normalIdx = (i < mesh.normals.size()) ? i : 0;
        if(mesh.normals.size() > 0) {
            vertices.push_back(mesh.normals[normalIdx].x);
            vertices.push_back(mesh.normals[normalIdx].y);
            vertices.push_back(mesh.normals[normalIdx].z);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(1.0f);
            vertices.push_back(0.0f);
        }
        
        // UV (use corresponding UV or first one if mismatch)
        size_t uvIdx = (i < mesh.uvs.size()) ? i : 0;
        if(mesh.uvs.size() > 0) {
            vertices.push_back(mesh.uvs[uvIdx].u);
            vertices.push_back(mesh.uvs[uvIdx].v);
        } else {
            vertices.push_back(0.0f);
            vertices.push_back(0.0f);
        }
    }
    
    return vertices;
}
#include "render.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

#include "TextureManager.hpp"
#include "../game_process.hpp"

GLuint shaderProgram = 0;
TextureManager* g_textureManager = nullptr;
std::string g_materialsBasePath = "";

struct RenderMesh {
    GLuint VAO;
    GLuint VBO;
    int vertexCount;
    GLuint textureID; // The texture for this mesh
};

std::vector<RenderMesh> g_worldMeshes;
glm::vec3 g_cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 g_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 g_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool InitRenderer() {
    std::cout << "InitRenderer: Starting..." << std::endl;
    
    // Create texture manager
    g_textureManager = new TextureManager();
    
    // Updated vertex shader - now with UVs!
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        layout (location = 1) in vec2 aTexCoord;
        
        out vec2 TexCoord;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
            TexCoord = aTexCoord;
        }
    )";
    
    // Updated fragment shader - now samples from texture!
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        in vec2 TexCoord;
        
        uniform sampler2D texture1;
        
        void main() {
            FragColor = texture(texture1, TexCoord);
        }
    )";
    
    GLuint vertexShader = glCreateShader(GL_VERTEX_SHADER);
    glShaderSource(vertexShader, 1, &vertexShaderSource, NULL);
    glCompileShader(vertexShader);
    
    GLint success;
    GLchar infoLog[512];
    glGetShaderiv(vertexShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(vertexShader, 512, NULL, infoLog);
        std::cerr << "Vertex shader failed: " << infoLog << std::endl;
        return false;
    }
    
    GLuint fragmentShader = glCreateShader(GL_FRAGMENT_SHADER);
    glShaderSource(fragmentShader, 1, &fragmentShaderSource, NULL);
    glCompileShader(fragmentShader);
    
    glGetShaderiv(fragmentShader, GL_COMPILE_STATUS, &success);
    if (!success) {
        glGetShaderInfoLog(fragmentShader, 512, NULL, infoLog);
        std::cerr << "Fragment shader failed: " << infoLog << std::endl;
        return false;
    }
    
    shaderProgram = glCreateProgram();
    glAttachShader(shaderProgram, vertexShader);
    glAttachShader(shaderProgram, fragmentShader);
    glLinkProgram(shaderProgram);
    
    glGetProgramiv(shaderProgram, GL_LINK_STATUS, &success);
    if (!success) {
        glGetProgramInfoLog(shaderProgram, 512, NULL, infoLog);
        std::cerr << "Shader linking failed: " << infoLog << std::endl;
        return false;
    }
    
    glDeleteShader(vertexShader);
    glDeleteShader(fragmentShader);

    std::cout << "InitRenderer: SUCCESS" << std::endl;
    return true;
}

void SetMaterialsPath(const std::string& basePath) {
    g_materialsBasePath = basePath;
    std::cout << "Renderer: Materials base path set to " << g_materialsBasePath << std::endl;
}

void UploadTMAPMeshes(const TMAPData& mapData) {
    std::cout << "UploadTMAPMeshes: Uploading " << mapData.meshes.size() << " meshes" << std::endl;
    
    for (auto& mesh : g_worldMeshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
    }
    g_worldMeshes.clear();
    
    for (const auto& mesh : mapData.meshes) {
        if (mesh.vertices.empty()) {
            std::cout << "UploadTMAPMeshes: Skipping empty mesh " << mesh.name << std::endl;
            continue;
        }
        
        // Check if we have UVs
        if (mesh.uvs.size() != mesh.vertices.size()) {
            std::cerr << "UploadTMAPMeshes: Warning - mesh " << mesh.name
                      << " has mismatched vertex/UV counts ("
                      << mesh.vertices.size() << " verts, " << mesh.uvs.size() << " UVs)" << std::endl;
        }
        
        RenderMesh rMesh;
        rMesh.vertexCount = mesh.vertices.size();
        
        // Load the texture for this mesh's material
        rMesh.textureID = g_textureManager->loadMaterialTexture(mesh.material, g_materialsBasePath);
        
        // Create interleaved vertex data: [x,y,z,u,v, x,y,z,u,v, ...]
        std::vector<float> interleavedData;
        interleavedData.reserve(mesh.vertices.size() * 5); // 3 for position, 2 for UV
        
        for (size_t i = 0; i < mesh.vertices.size(); i++) {
            // Position
            interleavedData.push_back(mesh.vertices[i].x);
            interleavedData.push_back(mesh.vertices[i].y);
            interleavedData.push_back(mesh.vertices[i].z);
            
            // UV (use 0,0 if we don't have enough UVs)
            if (i < mesh.uvs.size()) {
                interleavedData.push_back(mesh.uvs[i].u);
                interleavedData.push_back(mesh.uvs[i].v);
            } else {
                interleavedData.push_back(0.0f);
                interleavedData.push_back(0.0f);
            }
        }
        
        glGenVertexArrays(1, &rMesh.VAO);
        glGenBuffers(1, &rMesh.VBO);
        
        glBindVertexArray(rMesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, rMesh.VBO);
        
        glBufferData(GL_ARRAY_BUFFER, 
                     interleavedData.size() * sizeof(float), 
                     interleavedData.data(), 
                     GL_STATIC_DRAW);
        
        // Position attribute (location = 0)
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);
        
        // UV attribute (location = 1)
        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 5 * sizeof(float), (void*)(3 * sizeof(float)));
        glEnableVertexAttribArray(1);
        
        glBindVertexArray(0);
        
        g_worldMeshes.push_back(rMesh);
        
        std::cout << "UploadTMAPMeshes:   " << mesh.name 
                  << " (" << rMesh.vertexCount << " verts, material: " << mesh.material << ")" << std::endl;
    }
    
    g_cameraPos = glm::vec3(mapData.spawnPosition.x, 
                            mapData.spawnPosition.y + 1.0f,
                            mapData.spawnPosition.z + 3.0f);
    
    std::cout << "UploadTMAPMeshes: Complete!" << std::endl;
}

void SetCameraPosition(const glm::vec3& position) {
    g_cameraPos = position;
}

void SetCameraRotation(const glm::vec3& rotation) {
    glm::vec3 front;
    float pitch = glm::clamp(rotation.x, -89.9f, 89.9f);
    float yaw = glm::mod(rotation.y, 360.0f);
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    g_cameraFront = glm::normalize(front);
}

void RenderFrame(int width, int height) {
    glClearColor(0.1f, 0.1f, 0.15f, 1.0f);
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

    if (g_worldMeshes.empty()) {
        return;
    }
    
    glUseProgram(shaderProgram);
    
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(g_cameraPos, g_cameraPos + g_cameraFront, g_cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(90.0f * fov_multiplier), static_cast<float>(width) / height, 0.1f, 100.0f);
    
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    for (const auto& mesh : g_worldMeshes) {
        // Bind the texture for this mesh
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, mesh.textureID);
        
        glBindVertexArray(mesh.VAO);
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    }
    
    glBindVertexArray(0);
}

void CleanupRenderer() {
    for (auto& mesh : g_worldMeshes) {
        glDeleteVertexArrays(1, &mesh.VAO);
        glDeleteBuffers(1, &mesh.VBO);
    }
    g_worldMeshes.clear();
    
    if (g_textureManager) {
        delete g_textureManager;
        g_textureManager = nullptr;
    }
    
    if (shaderProgram) {
        glDeleteProgram(shaderProgram);
        shaderProgram = 0;
    }
}
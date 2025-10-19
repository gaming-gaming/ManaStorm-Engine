#include "render.hpp"

#include <GL/glew.h>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <iostream>
#include <vector>

GLuint shaderProgram = 0;

struct RenderMesh {
    GLuint VAO;
    GLuint VBO;
    int vertexCount;
};

std::vector<RenderMesh> g_worldMeshes;
glm::vec3 g_cameraPos = glm::vec3(0.0f, 2.0f, 5.0f);
glm::vec3 g_cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
glm::vec3 g_cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);

bool InitRenderer() {
    std::cout << "InitRenderer: Starting..." << std::endl;
    
    const char* vertexShaderSource = R"(
        #version 330 core
        layout (location = 0) in vec3 aPos;
        
        uniform mat4 model;
        uniform mat4 view;
        uniform mat4 projection;
        
        void main() {
            gl_Position = projection * view * model * vec4(aPos, 1.0);
        }
    )";
    
    const char* fragmentShaderSource = R"(
        #version 330 core
        out vec4 FragColor;
        
        void main() {
            FragColor = vec4(1.0, 0.5, 0.2, 1.0);
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
        
        RenderMesh rMesh;
        rMesh.vertexCount = mesh.vertices.size();
        
        glGenVertexArrays(1, &rMesh.VAO);
        glGenBuffers(1, &rMesh.VBO);
        
        glBindVertexArray(rMesh.VAO);
        glBindBuffer(GL_ARRAY_BUFFER, rMesh.VBO);
        
        glBufferData(GL_ARRAY_BUFFER, 
                     mesh.vertices.size() * sizeof(Vec3), 
                     mesh.vertices.data(), 
                     GL_STATIC_DRAW);
        
        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vec3), (void*)0);
        glEnableVertexAttribArray(0);
        
        glBindVertexArray(0);
        
        g_worldMeshes.push_back(rMesh);
        
        std::cout << "UploadTMAPMeshes:   " << mesh.name 
                  << " (" << rMesh.vertexCount << " verts)" << std::endl;
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
    float pitch = glm::clamp(rotation.x, -89.9f, 89.9f); // Clamp pitch to avoid gimbal lock
    float yaw = glm::mod(rotation.y, 360.0f); // Normalize yaw to [0, 360)
    front.x = cos(glm::radians(yaw)) * cos(glm::radians(pitch));
    front.y = sin(glm::radians(pitch));
    front.z = sin(glm::radians(yaw)) * cos(glm::radians(pitch));
    g_cameraFront = glm::normalize(front);
}

void RenderFrame(int width, int height) {
    glClearColor(0.0f, 0.0f, 0.0f, 1.0f); // Black background
    glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    
    if (g_worldMeshes.empty()) {
        return;
    }
    
    glUseProgram(shaderProgram);
    
    glm::mat4 model = glm::mat4(1.0f);
    glm::mat4 view = glm::lookAt(g_cameraPos, g_cameraPos + g_cameraFront, g_cameraUp);
    glm::mat4 projection = glm::perspective(glm::radians(90.0f), static_cast<float>(width) / height, 0.1f, 100.0f); // FOV; TODO: Make FOV change based on speed (that feels really cool)
    
    GLint modelLoc = glGetUniformLocation(shaderProgram, "model");
    GLint viewLoc = glGetUniformLocation(shaderProgram, "view");
    GLint projLoc = glGetUniformLocation(shaderProgram, "projection");
    
    glUniformMatrix4fv(modelLoc, 1, GL_FALSE, glm::value_ptr(model));
    glUniformMatrix4fv(viewLoc, 1, GL_FALSE, glm::value_ptr(view));
    glUniformMatrix4fv(projLoc, 1, GL_FALSE, glm::value_ptr(projection));
    
    for (const auto& mesh : g_worldMeshes) {
        glBindVertexArray(mesh.VAO);
        glDrawArrays(GL_TRIANGLES, 0, mesh.vertexCount);
    }
    
    glBindVertexArray(0);
}
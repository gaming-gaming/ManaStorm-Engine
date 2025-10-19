#include "TextureManager.hpp"
#include <iostream>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

TextureManager::TextureManager() : defaultTexture(0) {
    createDefaultTexture();
}

TextureManager::~TextureManager() {
    cleanup();
}

void TextureManager::createDefaultTexture() {
    // Create a simple 1x1 white texture as default
    unsigned char whitePixel[4] = {255, 255, 255, 255};
    
    glGenTextures(1, &defaultTexture);
    glBindTexture(GL_TEXTURE_2D, defaultTexture);
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, 1, 1, 0, GL_RGBA, GL_UNSIGNED_BYTE, whitePixel);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);
    glBindTexture(GL_TEXTURE_2D, 0);

    std::cout << "TextureManager: Created default texture (ID: " << defaultTexture << ")" << std::endl;
}

GLuint TextureManager::loadTextureFromFile(const std::string& filepath) {
    std::cout << "TextureManager: Loading " << filepath << std::endl;

    int width, height, channels;
    stbi_set_flip_vertically_on_load(true); // OpenGL expects texture origin at bottom-left
    unsigned char* data = stbi_load(filepath.c_str(), &width, &height, &channels, 0);
    
    if (!data) {
        std::cerr << "TextureManager: Failed to load texture: " << filepath << std::endl;
        std::cerr << "STB Error: " << stbi_failure_reason() << std::endl;
        return 0;
    }
    
    GLenum format;
    if (channels == 1)
        format = GL_RED;
    else if (channels == 3)
        format = GL_RGB;
    else if (channels == 4)
        format = GL_RGBA;
    else {
        std::cerr << "TextureManager: Unsupported channel count: " << channels << std::endl;
        stbi_image_free(data);
        return 0;
    }
    
    GLuint textureID;
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexImage2D(GL_TEXTURE_2D, 0, format, width, height, 0, format, GL_UNSIGNED_BYTE, data);
    glGenerateMipmap(GL_TEXTURE_2D);
    
    // Set texture parameters
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR_MIPMAP_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glBindTexture(GL_TEXTURE_2D, 0);
    stbi_image_free(data);

    std::cout << "TextureManager:   Loaded successfully (ID: " << textureID
              << ", " << width << "x" << height << ", " << channels << " channels)" << std::endl;

    return textureID;
}

GLuint TextureManager::loadMaterialTexture(const std::string& materialName, const std::string& materialsBasePath) {
    // Check if already loaded
    auto it = textureCache.find(materialName);
    if (it != textureCache.end()) {
        std::cout << "TextureManager: Using cached texture for " << materialName << std::endl;
        return it->second;
    }
    
    // Build path: materialsBasePath/material_name/albedo.png
    std::string texturePath = materialsBasePath + "/" + materialName + "/albedo.png";
    
    GLuint textureID = loadTextureFromFile(texturePath);
    
    if (textureID != 0) {
        textureCache[materialName] = textureID;
        return textureID;
    } else {
        std::cerr << "TextureManager: Failed to load material " << materialName
                  << ", using default texture" << std::endl;
        textureCache[materialName] = defaultTexture; // Cache the default so we don't keep trying
        return defaultTexture;
    }
}

GLuint TextureManager::getTexture(const std::string& materialName) {
    auto it = textureCache.find(materialName);
    if (it != textureCache.end()) {
        return it->second;
    }
    return 0;
}

GLuint TextureManager::getDefaultTexture() {
    return defaultTexture;
}

void TextureManager::cleanup() {
    for (auto& pair : textureCache) {
        if (pair.second != defaultTexture) { // Don't delete default texture multiple times
            glDeleteTextures(1, &pair.second);
        }
    }
    textureCache.clear();
    
    if (defaultTexture != 0) {
        glDeleteTextures(1, &defaultTexture);
        defaultTexture = 0;
    }

    std::cout << "TextureManager: Cleaned up all textures" << std::endl;
}
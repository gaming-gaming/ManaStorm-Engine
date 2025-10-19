#ifndef TEXTURE_MANAGER_HPP
#define TEXTURE_MANAGER_HPP

#include <string>
#include <unordered_map>
#include <GL/glew.h>

class TextureManager {
public:
    TextureManager();
    ~TextureManager();
    
    // Load a texture from a material folder
    // materialName: e.g. 'default', 'stone', etc.
    // materialsBasePath: e.g. '../../MyGame/materials'
    GLuint loadMaterialTexture(const std::string& materialName, const std::string& materialsBasePath);
    
    // Get a texture that was already loaded (returns 0 if not found)
    GLuint getTexture(const std::string& materialName);
    
    // Get a default white texture for materials that don't have textures
    GLuint getDefaultTexture();
    
    // Clean up all textures
    void cleanup();
    
private:
    std::unordered_map<std::string, GLuint> textureCache;
    GLuint defaultTexture;
    
    GLuint loadTextureFromFile(const std::string& filepath);
    void createDefaultTexture();
};

#endif // TEXTURE_MANAGER_HPP
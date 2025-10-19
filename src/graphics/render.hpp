#ifndef RENDER_HPP
#define RENDER_HPP

#include <glm/glm.hpp>
#include <string>

#include "../tmap_parser.hpp"

bool InitRenderer();
void RenderFrame(int width, int height);
void SetCameraPosition(const glm::vec3& position);
void SetCameraRotation(const glm::vec3& rotation);
void SetMaterialsPath(const std::string& basePath);
void UploadTMAPMeshes(const TMAPData& mapData);
void CleanupRenderer();

#endif // RENDER_HPP
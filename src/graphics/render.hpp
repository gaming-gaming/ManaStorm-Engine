#ifndef RENDER_HPP
#define RENDER_HPP

#include <glm/glm.hpp>

#include "../tmap_parser.hpp"

bool InitRenderer();
void RenderFrame(int width, int height);
void SetCameraPosition(const glm::vec3& position);
void SetCameraRotation(const glm::vec3& rotation);
void UploadTMAPMeshes(const TMAPData& mapData);

#endif // RENDER_HPP
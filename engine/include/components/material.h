#pragma once

#include <string>
#include <glm/glm.hpp>
#include "engine/export.h"

struct Material
{
    std::string texturePath;
    glm::vec4 color = glm::vec4(1.0f);
    float roughness = 0.5f;
    float metallic = 0.0f;
};

#pragma once
#include <cstdint>
#include <glm/glm.hpp>

struct Light
{
    glm::vec3 color = glm::vec3(1.0f);
};

struct GPULight
{
    glm::vec4 position; // .w = type (0 = directional, 1 = point, etc.)
    glm::vec4 color;    // .w = intensity
};
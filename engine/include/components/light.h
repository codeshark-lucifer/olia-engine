#pragma once
#include <cstdint>
#include <glm/glm.hpp>

struct Light
{
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;
};


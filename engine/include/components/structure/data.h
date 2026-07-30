#pragma once
#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

namespace Engine
{
    struct Vertex
    {
        glm::vec3 position;
        glm::vec3 normal;
        glm::vec2 uv;

        // Skeletal Animation Data
        glm::ivec4 boneIndices = glm::ivec4(0);
        glm::vec4 boneWeights  = glm::vec4(0.0f);
    };
} // namespace Engine
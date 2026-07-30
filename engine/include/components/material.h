#pragma once

#include <string>
#include <memory>
#include <glm/glm.hpp>
#include "engine/export.h"

namespace Engine { class Texture; }

struct Material
{
    glm::vec4 color{110.0f / 255.0f, 160.0f / 255.0f, 220.0f / 255.0f, 1.0f};

    float roughness = 0.5f;
    float metallic = 0.0f;

    std::string albedoTexture;
    std::shared_ptr<Engine::Texture> texture = nullptr; // <--- ADD THIS
};
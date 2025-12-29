#pragma once
#include <engine/ecs/entity.hpp>
#include <glm/glm.hpp>

enum class LightType
{
    Directional,
    Point,
    Spot
};

class Light : public Component
{
public:
    LightType type = LightType::Directional;
    glm::vec3 color = glm::vec3(1.0f);
    float intensity = 1.0f;

    // Directional or Spot light
    glm::vec3 direction = glm::vec3(-1.0f, -1.0f, -1.0f);

    // Point or Spot light
    float range = 10.0f;
};

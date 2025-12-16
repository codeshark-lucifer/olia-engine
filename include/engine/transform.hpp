#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>

class Entity; // forward declaration

class Transform
{
public:
    glm::vec3 position{0.0f};
    glm::vec3 rotation{0.0f};
    glm::vec3 scale{1.0f};

    std::weak_ptr<Entity> entity;

    Transform() = default;
    explicit Transform(std::weak_ptr<Entity> en) : entity(en) {}

    glm::mat4 GetLocalMatrix() const;

    glm::mat4 GetWorldMatrix() const;
};

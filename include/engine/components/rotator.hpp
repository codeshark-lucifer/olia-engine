#pragma once
#include <engine/ecs/entity.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <memory>
#include <iostream>

class Rotator : public Component
{
public:
    // Rotation speed in degrees per second
    glm::vec3 rotationSpeed{0.0f, 90.0f, 0.0f}; // Y-axis 90 deg/sec by default

    Rotator() = default;
    Rotator(const glm::vec3 &speed) : rotationSpeed(speed) {}

    void Update(const float &deltaTime) override
    {
        if (!enabled)
            return;

        if (auto e = entity.lock())
        {
            // Convert rotation speed to quaternion
            glm::vec3 deltaRadians = glm::radians(rotationSpeed * deltaTime);
            glm::quat deltaRot = glm::quat(deltaRadians);

            // Apply rotation
            e->rotation = glm::normalize(deltaRot * e->rotation);
        }
    }
};

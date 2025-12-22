#pragma once
#include <engine/ec/entity.hpp>
#include <glm/glm.hpp>

class Rigidbody : public Component
{
public:
    float mass{1.0f};
    float inverseMass{1.0f};
    float inverseInertia{1.0f};
    float restitution{0.3f};

    glm::vec3 velocity{0.0f};
    glm::vec3 angularVelocity{0.0f};

    glm::vec3 force{0.0f};
    glm::vec3 torque{0.0f};

    void Integrate(const glm::vec3 &gravity, const float &deltaTime)
    {
        if (inverseMass == 0.0f)
            return;

        force += gravity * mass;
        velocity += force * inverseMass * deltaTime;

        // torque += gravity * mass; // Removed incorrect application of gravity as torque
        angularVelocity += torque * inverseInertia * deltaTime;

        force = glm::vec3(0.0f);
        torque = glm::vec3(0.0f);
    }

public:
    Rigidbody() = default;
};
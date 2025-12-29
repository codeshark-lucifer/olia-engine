#pragma once
#include <engine/ecs/entity.hpp>
#include <iostream>

#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/quaternion.hpp>
#include <glm/gtc/matrix_transform.hpp>

class Rigidbody : public Component
{
public:
    glm::vec3 position;        // p
    glm::vec3 velocity;        // v
    glm::quat orientation;     // q
    glm::vec3 angularVelocity; // omega

    float mass;               // m
    glm::mat3 inertiaBody;    // I_body
    glm::mat3 inertiaInvBody; // I_body^-1

    glm::vec3 force;
    glm::vec3 torque;

    bool isUseGravity = true;

    float linearDamping;
    float angularDamping;

public:
    Rigidbody()
    {
        position = glm::vec3(0.0f);
        velocity = glm::vec3(0.0f);
        orientation = glm::quat(1.0f, 0.0f, 0.0f, 0.0f);
        angularVelocity = glm::vec3(0.0f);
        mass = 1.0f;
        force = glm::vec3(0.0f);
        linearDamping = 0.98f;
    }

    void OnAttach() override
    {
        if (auto e = entity.lock())
        {
            position = e->transform.position;
            orientation = e->transform.rotation;
        }
    }

    void Integrate(const glm::vec3 &gravity, float deltaTime)
    {
        if (isUseGravity)
            force += gravity * mass;
            
        // Position
        glm::vec3 acceleration = force / mass; // a = F / m
        velocity += acceleration * deltaTime;  // v = v + a*dt
        position += velocity * deltaTime;      // p = p + v*dt

        // Apply damping
        velocity *= linearDamping;
        if (glm::length(velocity) < 0.01f)
            velocity = glm::vec3(0.0f);

        force = glm::vec3(0.0f);
    }
};

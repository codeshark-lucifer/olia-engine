#pragma once
#include <engine/ecs/entity.hpp>
#include <engine/components/physics/box.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <glm/gtx/string_cast.hpp> // For glm::to_string
#include <memory>
#include <vector>
#include <iostream> // For std::cout

class Rigidbody : public Component
{
public:
    float mass = 1.0f;
    float inverseMass = 1.0f;
    float restitution = 0.8f; // bounciness
    glm::mat3 inertia{1.0f};
    glm::mat3 inverseInertia{1.0f};

    glm::vec3 position{};
    glm::quat rotation{1, 0, 0, 0};

    glm::vec3 velocity{};
    glm::vec3 angularVelocity{};
    glm::vec3 force{};
    glm::vec3 torque{};

    bool useGravity = true;
    float linearFriction = 0.2f;
    float angularFriction = 0.2f;

public:
    Rigidbody() = default;

    void SetMass(float m, glm::vec3 size = glm::vec3(1.0f))
    {
        mass = m;
        if (mass > 0.0f)
        {
            inverseMass = 1.0f / mass;
            float Ixx = 0.0833f * mass * (size.y * size.y + size.z * size.z);
            float Iyy = 0.0833f * mass * (size.x * size.x + size.z * size.z);
            float Izz = 0.0833f * mass * (size.x * size.x + size.y * size.y);
            inertia = glm::mat3(Ixx, 0, 0, 0, Iyy, 0, 0, 0, Izz);
            inverseInertia = glm::inverse(inertia);
        }
        else
        {
            inverseMass = 0.0f;
            inverseInertia = glm::mat3(0.0f);
        }
    }

    void OnAttach() override
    {
        if (auto e = entity.lock())
        {
            position = e->position;
            if (auto box = e->GetComponent<BoxCollider>())
                SetMass(mass, box->size);
            else
                SetMass(mass);
        }
    }

    void AddForce(const glm::vec3 &f) { force += f; }
    void AddTorque(const glm::vec3 &t) { torque += t; }

    void Integrate(float dt, const glm::vec3 &gravity)
    {
        if (mass <= 0.0f)
            return;

        // ---- Linear motion ----
        if (useGravity)
        {
            force += gravity * mass;
        }

        glm::vec3 acceleration = force * inverseMass;
        velocity += acceleration * dt;
        position += velocity * dt;

        // Apply friction
        velocity *= (1.0f - linearFriction * dt);

        // ---- Angular motion ----
        glm::vec3 angularAccel = inverseInertia * torque;
        angularVelocity += angularAccel * dt;

        glm::quat deltaRot(0, angularVelocity.x, angularVelocity.y, angularVelocity.z);
        rotation += deltaRot * rotation * (0.5f * dt);
        rotation = glm::normalize(rotation);
        
        angularVelocity *= (1.0f - angularFriction * dt);

        // ---- Sync entity ----
        if (auto e = entity.lock())
        {
            e->position = position;
            e->rotation = rotation;
        }

        // Reset accumulators
        force = glm::vec3(0.0f);
        torque = glm::vec3(0.0f);
        
    }
};

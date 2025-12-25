#pragma once
#include <engine/ecs/entity.hpp>
#include <engine/components/physics/box.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/quaternion.hpp>
#include <memory>
#include <vector>

struct AABB
{
    glm::vec3 min;
    glm::vec3 max;
};

inline AABB GetVolume(std::shared_ptr<BoxCollider> box)
{
    auto e = box->entity.lock();
    glm::vec3 center = (e ? e->position : glm::vec3(0.0f)) + box->center;
    glm::vec3 half = box->size * 0.5f;
    return {center - half, center + half};
}

inline bool CheckBoxOverlap(const AABB &A, const AABB &B)
{
    return (A.min.x <= B.max.x && A.max.x >= B.min.x) &&
           (A.min.y <= B.max.y && A.max.y >= B.min.y) &&
           (A.min.z <= B.max.z && A.max.z >= B.min.z);
}

inline bool IsSupported(std::shared_ptr<BoxCollider> box, const std::vector<std::shared_ptr<Collider>> &colliders)
{
    auto e = box->entity.lock();
    if (!e)
        return false;

    glm::vec3 center = e->position + box->center;
    glm::vec3 half = box->size * 0.5f;

    // Use bottom center point only
    glm::vec3 probePoint = center - glm::vec3(0, half.y + 0.01f, 0);

    for (auto &c : colliders)
    {
        if (c->type != ColliderType::Cube)
            continue;
        auto otherBox = std::dynamic_pointer_cast<BoxCollider>(c);
        if (otherBox == box)
            continue;

        AABB v = GetVolume(otherBox);
        if (probePoint.x >= v.min.x && probePoint.x <= v.max.x &&
            probePoint.y >= v.min.y && probePoint.y <= v.max.y &&
            probePoint.z >= v.min.z && probePoint.z <= v.max.z)
            return true;
    }
    return false;
}

class Rigidbody : public Component
{
public:
    float mass = 1.0f;
    float inverseMass = 1.0f;
    float restitution = 0.5f; // bounciness
    float inertia = 1.0f;
    float inverseInertia = 1.0f;

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

    void OnAttach() override
    {
        if (auto e = entity.lock())
            position = e->position;
    }

    void AddForce(const glm::vec3 &f) { force += f; }
    void AddTorque(const glm::vec3 &t) { torque += t; }

    void Integrate(float dt, const glm::vec3 &gravity, const std::vector<std::shared_ptr<Collider>> &colliders)
    {
        if (mass <= 0.0f)
            return;

        inverseMass = 1.0f / mass;
        inverseInertia = (inertia > 0.0f) ? 1.0f / inertia : 0.0f;

        // ---- Linear motion ----
        if (useGravity)
        {
            bool supported = false;
            if (auto box = std::dynamic_pointer_cast<BoxCollider>(entity.lock()->GetComponent<BoxCollider>()))
                supported = IsSupported(box, colliders);

            if (!supported)
                force += gravity * mass;
        }

        glm::vec3 acceleration = force * inverseMass;
        velocity += acceleration * dt;
        position += velocity * dt;

        // Apply friction
        velocity *= (1.0f - linearFriction * dt);

        // ---- Angular motion ----
        glm::vec3 angularAccel = torque * inverseInertia;
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

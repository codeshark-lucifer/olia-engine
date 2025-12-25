#pragma once
#include <engine/ecs/system.hpp>
#include <engine/components/physics/rigidbody.hpp>
#include <engine/components/physics/collider.hpp>
#include <engine/components/physics/box.hpp>
#include <engine/components/physics/sphere.hpp>
#include <glm/glm.hpp>
#include <vector>
#include <memory>
#include <algorithm>

class PhysicsSystem : public System
{
public:
    glm::vec3 gravity{0.0f, -9.81f, 0.0f};
    const float fixedTimeStep = 0.016f;
    float accumulator = 0.0f;

    PhysicsSystem() : System("PhysicsSystem") {}

    void Update(const std::vector<std::shared_ptr<Entity>> &entities, const float &deltaTime) override
    {
        accumulator += deltaTime;

        while (accumulator >= fixedTimeStep)
        {
            std::vector<std::shared_ptr<Collider>> colliders;
            for (auto &en : entities)
                if (en->HasComponent<Collider>())
                    colliders.push_back(en->GetComponent<Collider>());

            // Integrate physics
            for (auto &entity : entities)
            {
                if (auto rb = entity->GetComponent<Rigidbody>())
                    rb->Integrate(fixedTimeStep, gravity, colliders);

                UpdateChildren(entity, fixedTimeStep, colliders);
            }

            // Solve collisions
            SolveCollisions(colliders);

            accumulator -= fixedTimeStep;
        }
    }

private:
    void UpdateChildren(std::shared_ptr<Entity> entity, float dt, const std::vector<std::shared_ptr<Collider>> &colliders)
    {
        for (auto &child : entity->children)
        {
            if (auto rb = child->GetComponent<Rigidbody>())
                rb->Integrate(dt, gravity, colliders);
            UpdateChildren(child, dt, colliders);
        }
    }

    void SolveCollisions(std::vector<std::shared_ptr<Collider>> colliders)
    {
        for (size_t i = 0; i < colliders.size(); i++)
            for (size_t j = i + 1; j < colliders.size(); j++)
                ResolveCollision(colliders[i], colliders[j]);
    }

    void ResolveCollision(std::shared_ptr<Collider> A, std::shared_ptr<Collider> B)
    {
        if (A->type == ColliderType::Cube && B->type == ColliderType::Cube)
            BoxBox(std::dynamic_pointer_cast<BoxCollider>(A), std::dynamic_pointer_cast<BoxCollider>(B));
        else if (A->type == ColliderType::Cube && B->type == ColliderType::Sphere)
            BoxSphere(std::dynamic_pointer_cast<BoxCollider>(A), std::dynamic_pointer_cast<SphereCollider>(B));
        else if (A->type == ColliderType::Sphere && B->type == ColliderType::Cube)
            BoxSphere(std::dynamic_pointer_cast<BoxCollider>(B), std::dynamic_pointer_cast<SphereCollider>(A));
        else if (A->type == ColliderType::Sphere && B->type == ColliderType::Sphere)
            SphereSphere(std::dynamic_pointer_cast<SphereCollider>(A), std::dynamic_pointer_cast<SphereCollider>(B));
    }

private:
    void BoxBox(std::shared_ptr<BoxCollider> A, std::shared_ptr<BoxCollider> B)
    {
        auto eA = A->entity.lock();
        auto eB = B->entity.lock();
        if (!eA || !eB)
            return;

        auto rbA = eA->GetComponent<Rigidbody>();
        auto rbB = eB->GetComponent<Rigidbody>();
        if (!rbA && !rbB)
            return;

        // World-space AABB
        glm::vec3 minA = eA->position + A->center - A->size * 0.5f;
        glm::vec3 maxA = eA->position + A->center + A->size * 0.5f;
        glm::vec3 minB = eB->position + B->center - B->size * 0.5f;
        glm::vec3 maxB = eB->position + B->center + B->size * 0.5f;

        // Check overlap
        if (!(maxA.x > minB.x && minA.x < maxB.x &&
              maxA.y > minB.y && minA.y < maxB.y &&
              maxA.z > minB.z && minA.z < maxB.z))
            return;

        // Compute penetration
        glm::vec3 overlap(
            std::min(maxA.x, maxB.x) - std::max(minA.x, minB.x),
            std::min(maxA.y, maxB.y) - std::max(minA.y, minB.y),
            std::min(maxA.z, maxB.z) - std::max(minA.z, minB.z));

        // Smallest penetration axis
        int axis = 0;
        if (overlap.y < overlap.x)
            axis = 1;
        if (overlap.z < overlap[axis])
            axis = 2;

        glm::vec3 normal(0.0f);
        normal[axis] = 1.0f;

        glm::vec3 delta = (eB->position - eA->position);
        if (glm::dot(delta, normal) < 0)
            normal = -normal;

        float penetration = overlap[axis];

        float invMassA = rbA ? 1.0f / rbA->mass : 0.0f;
        float invMassB = rbB ? 1.0f / rbB->mass : 0.0f;
        float totalInvMass = invMassA + invMassB;
        if (totalInvMass <= 0.0f)
            return;

        glm::vec3 correction = normal * (penetration / totalInvMass);
        if (rbA)
            rbA->position -= correction * invMassA;
        if (rbB)
            rbB->position += correction * invMassB;

        // Velocity impulse
        glm::vec3 relVel = (rbB ? rbB->velocity : glm::vec3(0)) - (rbA ? rbA->velocity : glm::vec3(0));
        float velAlongNormal = glm::dot(relVel, normal);
        if (velAlongNormal > 0)
            return;

        float e = std::min(rbA ? rbA->restitution : 0.0f, rbB ? rbB->restitution : 0.0f);
        float j = -(1.0f + e) * velAlongNormal / totalInvMass;
        glm::vec3 impulse = j * normal;

        if (rbA)
            rbA->velocity -= impulse * invMassA;
        if (rbB)
            rbB->velocity += impulse * invMassB;
    }

    void SphereSphere(std::shared_ptr<SphereCollider> A, std::shared_ptr<SphereCollider> B)
    {
        auto eA = A->entity.lock();
        auto eB = B->entity.lock();
        if (!eA || !eB)
            return;

        auto rbA = eA->GetComponent<Rigidbody>();
        auto rbB = eB->GetComponent<Rigidbody>();

        glm::vec3 delta = (eB->position + B->center) - (eA->position + A->center);
        float dist = glm::length(delta);
        float r = A->radius + B->radius;
        if (dist <= 0.0f || dist >= r)
            return;

        glm::vec3 normal = delta / dist;
        float penetration = r - dist;

        float invMassA = rbA ? 1.0f / rbA->mass : 0.0f;
        float invMassB = rbB ? 1.0f / rbB->mass : 0.0f;
        float totalInvMass = invMassA + invMassB;
        if (totalInvMass <= 0)
            return;

        glm::vec3 correction = normal * (penetration / totalInvMass);
        if (rbA)
            rbA->position -= correction * invMassA;
        if (rbB)
            rbB->position += correction * invMassB;

        glm::vec3 relVel = (rbB ? rbB->velocity : glm::vec3(0)) - (rbA ? rbA->velocity : glm::vec3(0));
        float velAlongNormal = glm::dot(relVel, normal);
        if (velAlongNormal > 0)
            return;

        float e = std::min(rbA ? rbA->restitution : 0.0f, rbB ? rbB->restitution : 0.0f);
        float j = -(1.0f + e) * velAlongNormal / totalInvMass;
        glm::vec3 impulse = j * normal;

        if (rbA)
            rbA->velocity -= impulse * invMassA;
        if (rbB)
            rbB->velocity += impulse * invMassB;
    }

    void BoxSphere(std::shared_ptr<BoxCollider> box, std::shared_ptr<SphereCollider> sphere)
    {
        auto eBox = box->entity.lock();
        auto eSphere = sphere->entity.lock();
        if (!eBox || !eSphere)
            return;

        glm::vec3 boxMin = eBox->position + box->center - box->size * 0.5f;
        glm::vec3 boxMax = eBox->position + box->center + box->size * 0.5f;
        glm::vec3 spherePos = eSphere->position + sphere->center;

        glm::vec3 closest = glm::clamp(spherePos, boxMin, boxMax);
        glm::vec3 delta = spherePos - closest;
        float dist = glm::length(delta);
        if (dist <= 0.0f || dist >= sphere->radius)
            return;

        glm::vec3 normal = delta / dist;
        float penetration = sphere->radius - dist;

        auto rbBox = eBox->GetComponent<Rigidbody>();
        auto rbSphere = eSphere->GetComponent<Rigidbody>();
        float invMassBox = rbBox ? 1.0f / rbBox->mass : 0.0f;
        float invMassSphere = rbSphere ? 1.0f / rbSphere->mass : 0.0f;
        float totalInvMass = invMassBox + invMassSphere;
        if (totalInvMass <= 0.0f)
            return;

        glm::vec3 correction = normal * (penetration / totalInvMass);
        if (rbBox)
            rbBox->position -= correction * invMassBox;
        if (rbSphere)
            rbSphere->position += correction * invMassSphere;

        glm::vec3 relVel = (rbSphere ? rbSphere->velocity : glm::vec3(0)) - (rbBox ? rbBox->velocity : glm::vec3(0));
        float velAlongNormal = glm::dot(relVel, normal);
        if (velAlongNormal > 0)
            return;

        float e = std::min(rbBox ? rbBox->restitution : 0.0f, rbSphere ? rbSphere->restitution : 0.0f);
        float j = -(1.0f + e) * velAlongNormal / totalInvMass;
        glm::vec3 impulse = j * normal;

        if (rbBox)
            rbBox->velocity -= impulse * invMassBox;
        if (rbSphere)
            rbSphere->velocity += impulse * invMassSphere;
    }
};

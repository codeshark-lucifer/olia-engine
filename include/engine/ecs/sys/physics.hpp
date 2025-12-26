#pragma once
#include <engine/ecs/system.hpp>
#include <engine/components/physics/rigidbody.hpp>
#include <engine/components/physics/collider.hpp>
#include <engine/components/physics/box.hpp>
#include <engine/components/physics/sphere.hpp>
#include <engine/components/collision/collision.hpp>
#include <glm/glm.hpp>
#include <glm/gtx/string_cast.hpp> // For glm::to_string
#include <vector>
#include <memory>
#include <algorithm>
#include <iostream> // For std::cout

class PhysicsSystem : public System
{
public:
    glm::vec3 gravity{0.0f, -9.81f, 0.0f};
    const float fixedTimeStep = 0.016f;
    float accumulator = 0.0f;

    // Baumgarte stabilization constants
    const float BAUMSGARTE_SLOP = 0.01f;
    const float BAUMSGARTE_ERP = 0.2f;

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
                    rb->Integrate(fixedTimeStep, gravity);

                UpdateChildren(entity, fixedTimeStep);
            }

            // Solve collisions
            SolveCollisions(colliders);

            accumulator -= fixedTimeStep;
        }
    }

private:
    void UpdateChildren(std::shared_ptr<Entity> entity, float dt)
    {
        for (auto &child : entity->children)
        {
            if (auto rb = child->GetComponent<Rigidbody>())
                rb->Integrate(dt, gravity);
            UpdateChildren(child, dt);
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
        if (!eA || !eB) return;

        auto rbA = eA->GetComponent<Rigidbody>();
        auto rbB = eB->GetComponent<Rigidbody>();

        glm::vec3 aMin = eA->position + A->center - A->size;
        glm::vec3 aMax = eA->position + A->center + A->size;
        glm::vec3 bMin = eB->position + B->center - B->size;
        glm::vec3 bMax = eB->position + B->center + B->size;

        // Compute overlap on each axis
        glm::vec3 overlap;
        overlap.x = std::min(aMax.x, bMax.x) - std::max(aMin.x, bMin.x);
        overlap.y = std::min(aMax.y, bMax.y) - std::max(aMin.y, bMin.y);
        overlap.z = std::min(aMax.z, bMax.z) - std::max(aMin.z, bMin.z);

        if (overlap.x <= 0 || overlap.y <= 0 || overlap.z <= 0)
            return; // No collision

        // Find axis of minimum penetration
        int axis = 0;
        float minOverlap = overlap.x;
        if (overlap.y < minOverlap) { axis = 1; minOverlap = overlap.y; }
        if (overlap.z < minOverlap) { axis = 2; minOverlap = overlap.z; }

        glm::vec3 normal(0.0f);
        if (axis == 0) normal.x = (eB->position.x > eA->position.x) ? 1.0f : -1.0f;
        else if (axis == 1) normal.y = (eB->position.y > eA->position.y) ? 1.0f : -1.0f;
        else normal.z = (eB->position.z > eA->position.z) ? 1.0f : -1.0f;

        float invMassA = rbA ? 1.0f / rbA->mass : 0.0f;
        float invMassB = rbB ? 1.0f / rbB->mass : 0.0f;
        float totalInvMass = invMassA + invMassB;
        if (totalInvMass <= 0) return;

        // Positional correction (demo mode, reduce damping)
        const float percent = 0.5f; // 50% penetration
        const float slop = 0.001f;
        glm::vec3 correction = normal * std::max(minOverlap - slop, 0.0f) * percent / totalInvMass;
        if (rbA) rbA->position -= correction * invMassA;
        if (rbB) rbB->position += correction * invMassB;

        // Relative velocity
        glm::vec3 relVel = (rbB ? rbB->velocity : glm::vec3(0)) - (rbA ? rbA->velocity : glm::vec3(0));
        float velAlongNormal = glm::dot(relVel, normal);

        // Demo-bounce: enforce small impulse if objects are nearly resting
        if (velAlongNormal > -0.05f) velAlongNormal = -0.05f;

        // Restitution
        float e = std::min(rbA ? rbA->restitution : 1.0f, rbB ? rbB->restitution : 1.0f);
        float j = -(1.0f + e) * velAlongNormal / totalInvMass;
        glm::vec3 impulse = j * normal;

        if (rbA) rbA->velocity -= impulse * invMassA;
        if (rbB) rbB->velocity += impulse * invMassB;

        // std::cout << "[BoxBox Demo] Impulse: " << glm::to_string(impulse) << "\n";
    }

    void SphereSphere(std::shared_ptr<SphereCollider> A, std::shared_ptr<SphereCollider> B)
    {
        auto eA = A->entity.lock();
        auto eB = B->entity.lock();
        if (!eA || !eB) return;

        auto rbA = eA->GetComponent<Rigidbody>();
        auto rbB = eB->GetComponent<Rigidbody>();

        glm::vec3 delta = (eB->position + B->center) - (eA->position + A->center);
        float dist = glm::length(delta);
        float radiusSum = A->radius + B->radius;
        if (dist <= 0.0f || dist >= radiusSum) return;

        glm::vec3 normal = delta / dist;
        float penetration = radiusSum - dist;

        float invMassA = rbA ? 1.0f / rbA->mass : 0.0f;
        float invMassB = rbB ? 1.0f / rbB->mass : 0.0f;
        float totalInvMass = invMassA + invMassB;
        if (totalInvMass <= 0.0f) return;

        // Positional correction (smaller to not damp bounce)
        const float percent = 0.5f; // 50% of penetration
        const float slop = 0.001f;
        glm::vec3 correction = normal * std::max(penetration - slop, 0.0f) * percent / totalInvMass;
        if (rbA) rbA->position -= correction * invMassA;
        if (rbB) rbB->position += correction * invMassB;

        // Relative velocity
        glm::vec3 relVel = (rbB ? rbB->velocity : glm::vec3(0)) - (rbA ? rbA->velocity : glm::vec3(0));
        float velAlongNormal = glm::dot(relVel, normal);

        // Demo-bounce: ensure small impulses even if object is resting
        if (velAlongNormal > -0.05f) velAlongNormal = -0.05f;

        float e = std::min(rbA ? rbA->restitution : 1.0f, rbB ? rbB->restitution : 1.0f);
        float j = -(1.0f + e) * velAlongNormal / totalInvMass;
        glm::vec3 impulse = j * normal;

        if (rbA) rbA->velocity -= impulse * invMassA;
        if (rbB) rbB->velocity += impulse * invMassB;

        // std::cout << "[SphereSphere Demo] Impulse: " << glm::to_string(impulse) << "\n";
    }

    void BoxSphere(std::shared_ptr<BoxCollider> box, std::shared_ptr<SphereCollider> sphere)
    {
        auto eBox = box->entity.lock();
        auto eSphere = sphere->entity.lock();
        if (!eBox || !eSphere) return;

        glm::vec3 boxMin = eBox->position + box->center - box->size;
        glm::vec3 boxMax = eBox->position + box->center + box->size;
        glm::vec3 spherePos = eSphere->position + sphere->center;

        glm::vec3 closest = glm::clamp(spherePos, boxMin, boxMax);
        glm::vec3 delta = spherePos - closest;
        float dist = glm::length(delta);
        if (dist <= 0.0f || dist >= sphere->radius) return;

        glm::vec3 normal = delta / dist;
        float penetration = sphere->radius - dist;

        auto rbBox = eBox->GetComponent<Rigidbody>();
        auto rbSphere = eSphere->GetComponent<Rigidbody>();
        float invMassBox = rbBox ? 1.0f / rbBox->mass : 0.0f;
        float invMassSphere = rbSphere ? 1.0f / rbSphere->mass : 0.0f;
        float totalInvMass = invMassBox + invMassSphere;
        if (totalInvMass <= 0.0f) return;

        // Positional correction (reduce damping)
        const float percent = 0.5f;
        const float slop = 0.001f;
        glm::vec3 correction = normal * std::max(penetration - slop, 0.0f) * percent / totalInvMass;
        if (rbBox) rbBox->position -= correction * invMassBox;
        if (rbSphere) rbSphere->position += correction * invMassSphere;

        // Relative velocity
        glm::vec3 relVel = (rbSphere ? rbSphere->velocity : glm::vec3(0)) - (rbBox ? rbBox->velocity : glm::vec3(0));
        float velAlongNormal = glm::dot(relVel, normal);

        // Demo-bounce: enforce small upward impulse even if resting
        if (velAlongNormal > -0.05f) velAlongNormal = -0.05f;

        float e = std::min(rbBox ? rbBox->restitution : 1.0f, rbSphere ? rbSphere->restitution : 1.0f);
        float j = -(1.0f + e) * velAlongNormal / totalInvMass;
        glm::vec3 impulse = j * normal;

        if (rbBox) rbBox->velocity -= impulse * invMassBox;
        if (rbSphere) rbSphere->velocity += impulse * invMassSphere;

        // std::cout << "[BoxSphere Demo] Impulse: " << glm::to_string(impulse) << "\n";
    }

};

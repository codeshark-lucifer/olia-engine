#pragma once
#include <engine/ecs/system.hpp>
#include <engine/components/physics/rigidbody.hpp>
#include <engine/components/physics/collision.hpp>
#include <engine/components/physics/sphere.hpp>
#include <engine/components/physics/box.hpp>

class PhysicsSystem : public System
{
public:
    glm::vec3 gravity{0.0f, -9.81f, 0.0f};

    PhysicsSystem() : System("PhysicsSystem") {}

    void Update(std::vector<std::shared_ptr<Entity>> &entities, float deltaTime) override
    {
        accumulator += deltaTime;

        while (accumulator >= fixedTimeStep)
        {
            // Collect all colliders
            std::vector<std::shared_ptr<Collider>> colliders;
            for (auto &entity : entities)
                if (auto c = entity->GetComponent<Collider>())
                    colliders.push_back(c);

            // Integrate dynamic rigidbodies
            for (auto &entity : entities)
                IntegrateEntity(entity, fixedTimeStep);

            // Solve collisions
            SolveCollisions(colliders);

            accumulator -= fixedTimeStep;
        }
    }

private:
    float accumulator = 0.0f;
    float fixedTimeStep = 1.0f / 60.0f;

    void IntegrateEntity(std::shared_ptr<Entity> entity, float dt)
    {
        if (auto rb = entity->GetComponent<Rigidbody>())
        {
            rb->Integrate(gravity, dt);
            entity->transform.position = rb->position;
            entity->transform.rotation = rb->orientation;
        }

        for (auto &child : entity->GetChildren())
            IntegrateEntity(child, dt);
    }

    void SolveCollisions(std::vector<std::shared_ptr<Collider>> &colliders)
    {
        for (size_t i = 0; i < colliders.size(); ++i)
        {
            for (size_t j = i + 1; j < colliders.size(); ++j)
                ResolveCollision(colliders[i], colliders[j]);
        }
    }

    void ResolveCollision(std::shared_ptr<Collider> &A, std::shared_ptr<Collider> &B)
    {
        if (!A || !B) return;

        if (A->type == CollisionType::Sphere && B->type == CollisionType::Sphere)
        {
            auto a = std::dynamic_pointer_cast<SphereCollider>(A);
            auto b = std::dynamic_pointer_cast<SphereCollider>(B);
            if(!a || !b) return;
            SphereSphere(a, b);
        }
        else if (A->type == CollisionType::AABB && B->type == CollisionType::AABB)
        {
            auto a = std::dynamic_pointer_cast<BoxCollider>(A);
            auto b = std::dynamic_pointer_cast<BoxCollider>(B);
            if(!a || !b) return;
            BoxBox(a, b);
        }
        else
        {
            // Sphere-Box
            std::shared_ptr<SphereCollider> sphere;
            std::shared_ptr<BoxCollider> box;
            if (A->type == CollisionType::Sphere)
            {
                sphere = std::dynamic_pointer_cast<SphereCollider>(A);
                box = std::dynamic_pointer_cast<BoxCollider>(B);
            }
            else
            {
                sphere = std::dynamic_pointer_cast<SphereCollider>(B);
                box = std::dynamic_pointer_cast<BoxCollider>(A);
            }
            if (sphere && box)
                SphereBox(sphere, box);
        }
    }

    // ----------------------------
    // Collision Functions
    // ----------------------------

    void SphereSphere(std::shared_ptr<SphereCollider> &A,
                      std::shared_ptr<SphereCollider> &B)
    {
        auto enA = A->entity.lock();
        auto enB = B->entity.lock();
        if (!enA || !enB) return;

        auto rbA = enA->GetComponent<Rigidbody>();
        auto rbB = enB->GetComponent<Rigidbody>();

        glm::vec3 posA = enA->transform.position;
        glm::vec3 posB = enB->transform.position;

        float rA = A->radius;
        float rB = B->radius;

        glm::vec3 n = posB - posA;
        float dist = glm::length(n);
        if (dist <= 0.0f) n = glm::vec3(0,1,0);
        else n /= dist;

        float penetration = rA + rB - dist;
        if (penetration <= 0.0f) return;

        glm::vec3 velA = rbA ? rbA->velocity : glm::vec3(0);
        glm::vec3 velB = rbB ? rbB->velocity : glm::vec3(0);
        glm::vec3 relVel = velB - velA;
        float velAlongNormal = glm::dot(relVel, n);
        if (velAlongNormal > 0) velAlongNormal = 0.0f;

        float invA = rbA ? 1.0f / rbA->mass : 0.0f;
        float invB = rbB ? 1.0f / rbB->mass : 0.0f;

        float e = 0.5f;
        float j = -(1 + e) * velAlongNormal / (invA + invB);
        glm::vec3 impulse = j * n;

        if (rbA) rbA->velocity -= impulse * invA;
        if (rbB) rbB->velocity += impulse * invB;

        // Positional correction
        glm::vec3 correction = n * penetration / (invA + invB) * 0.8f;
        if (rbA) rbA->position -= correction * invA;
        if (rbB) rbB->position += correction * invB;
    }

    void BoxBox(std::shared_ptr<BoxCollider> &A,
                std::shared_ptr<BoxCollider> &B)
    {
        auto enA = A->entity.lock();
        auto enB = B->entity.lock();
        if (!enA || !enB) return;

        auto rbA = enA->GetComponent<Rigidbody>();
        auto rbB = enB->GetComponent<Rigidbody>();

        glm::vec3 posA = enA->transform.position;
        glm::vec3 posB = enB->transform.position;
        glm::vec3 halfA = A->halfSize;
        glm::vec3 halfB = B->halfSize;

        glm::vec3 delta = posB - posA;
        glm::vec3 overlap = halfA + halfB - glm::abs(delta);
        if (overlap.x <= 0 || overlap.y <= 0 || overlap.z <= 0) return;

        // Find axis of minimum penetration
        glm::vec3 n;
        float penetration;
        if (overlap.x < overlap.y && overlap.x < overlap.z) { n = glm::vec3(delta.x < 0 ? -1.f : 1.f,0,0); penetration = overlap.x; }
        else if (overlap.y < overlap.z) { n = glm::vec3(0, delta.y < 0 ? -1.f : 1.f,0); penetration = overlap.y; }
        else { n = glm::vec3(0,0, delta.z < 0 ? -1.f : 1.f); penetration = overlap.z; }

        glm::vec3 velA = rbA ? rbA->velocity : glm::vec3(0);
        glm::vec3 velB = rbB ? rbB->velocity : glm::vec3(0);
        float invA = rbA ? 1.0f / rbA->mass : 0.0f;
        float invB = rbB ? 1.0f / rbB->mass : 0.0f;

        float velAlongNormal = glm::dot(velB - velA, n);
        if (velAlongNormal > 0) velAlongNormal = 0.0f;

        float e = 0.5f;
        float j = -(1 + e) * velAlongNormal / (invA + invB);
        glm::vec3 impulse = j * n;

        if (rbA) rbA->velocity -= impulse * invA;
        if (rbB) rbB->velocity += impulse * invB;

        // Angular velocity
        const glm::vec3 r_local(0.1f, 0.1f, 0.1f); // A small, arbitrary offset vector
        if (rbA)
        {
            glm::vec3 rA_world = rbA->orientation * r_local; // Rotate offset to world space
            rbA->angularVelocity += glm::cross(rA_world, -impulse) * 0.1f;
        }
        if (rbB)
        {
            glm::vec3 rB_world = rbB->orientation * r_local;
            rbB->angularVelocity += glm::cross(rB_world, impulse) * 0.1f;
        }

        // Positional correction
        glm::vec3 correction = n * penetration / (invA + invB) * 0.8f;
        if (rbA) rbA->position -= correction * invA;
        if (rbB) rbB->position += correction * invB;
    }

    void SphereBox(std::shared_ptr<SphereCollider> &sphere,
                   std::shared_ptr<BoxCollider> &box)
    {
        auto enS = sphere->entity.lock();
        auto enB = box->entity.lock();
        if (!enS || !enB) return;

        auto rbS = enS->GetComponent<Rigidbody>();
        auto rbB = enB->GetComponent<Rigidbody>();

        glm::vec3 spherePos = enS->transform.position;
        glm::vec3 boxPos = enB->transform.position;
        glm::vec3 half = box->halfSize;

        glm::vec3 closest = glm::clamp(spherePos, boxPos - half, boxPos + half);
        glm::vec3 delta = spherePos - closest;
        float dist2 = glm::dot(delta, delta);
        float radius = sphere->radius;
        if (dist2 > radius * radius) return;

        float dist = sqrt(dist2);
        glm::vec3 n = dist > 0 ? delta / dist : glm::vec3(0,1,0);
        float penetration = radius - dist;

        glm::vec3 velS = rbS ? rbS->velocity : glm::vec3(0);
        glm::vec3 velB = rbB ? rbB->velocity : glm::vec3(0);
        float invS = rbS ? 1.0f / rbS->mass : 0.0f;
        float invB = rbB ? 1.0f / rbB->mass : 0.0f;

        float velAlongNormal = glm::dot(velS - velB, n);
        if (velAlongNormal > 0) velAlongNormal = 0.0f;

        float e = 0.5f;
        float j = -(1 + e) * velAlongNormal / (invS + invB);
        glm::vec3 impulse = j * n;

        if (rbS) rbS->velocity += impulse * invS;
        if (rbB) rbB->velocity -= impulse * invB;

        // Angular velocity
        if (rbS)
        {
            glm::vec3 rS = -n * sphere->radius;
            rbS->angularVelocity += glm::cross(rS, impulse);
        }
        if (rbB)
        {
            glm::vec3 rB = closest - rbB->position;
            rbB->angularVelocity += glm::cross(rB, -impulse);
        }

        glm::vec3 correction = n * penetration / (invS + invB) * 0.8f;
        if (rbS) rbS->position += correction * invS;
        if (rbB) rbB->position -= correction * invB;
    }
};

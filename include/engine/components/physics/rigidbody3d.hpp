#pragma once
#include <engine/ecs/component.hpp>
#include <engine/ecs/physics.component.hpp>
#include <engine/systems/physics.hpp>
#include <glm/glm.hpp>

enum class ForceMode
{
    Force,          // Continuous force
    Impulse,        // Instant impulse
    Acceleration,  // Ignores mass
    VelocityChange // Instant velocity change
};

class RigidBody3D : public Component
{
public:
    /* =========================
       Unity-like Properties
       ========================= */

    float mass = 1.0f;
    bool useGravity = true;
    bool isKinematic = false;

    bool freezePositionX = false;
    bool freezePositionY = false;
    bool freezePositionZ = false;

    bool freezeRotationX = false;
    bool freezeRotationY = false;
    bool freezeRotationZ = false;

public:
    RigidBody3D(float m = 1.0f) : mass(m) {}

    /* =========================
       Unity-like Methods
       ========================= */

    void AddForce(const glm::vec3& force, ForceMode mode = ForceMode::Force)
    {
        if (auto phys = GetPhysics())
        {
            btRigidBody* body = phys->body;
            if (!body || isKinematic)
                return;

            btVector3 f(force.x, force.y, force.z);

            switch (mode)
            {
                case ForceMode::Force:
                    body->applyCentralForce(f);
                    break;

                case ForceMode::Impulse:
                    body->applyCentralImpulse(f);
                    break;

                case ForceMode::Acceleration:
                    body->applyCentralForce(f * body->getMass());
                    break;

                case ForceMode::VelocityChange:
                    body->setLinearVelocity(body->getLinearVelocity() + f);
                    break;
            }
        }
    }

    void AddTorque(const glm::vec3& torque, ForceMode mode = ForceMode::Force)
    {
        if (auto phys = GetPhysics())
        {
            btRigidBody* body = phys->body;
            if (!body || isKinematic)
                return;

            btVector3 t(torque.x, torque.y, torque.z);

            if (mode == ForceMode::Impulse)
                body->applyTorqueImpulse(t);
            else
                body->applyTorque(t);
        }
    }

    void SetVelocity(const glm::vec3& velocity)
    {
        if (auto phys = GetPhysics())
            if (phys->body)
                phys->body->setLinearVelocity(
                    btVector3(velocity.x, velocity.y, velocity.z)
                );
    }

    glm::vec3 GetVelocity() const
    {
        if (auto phys = GetPhysics())
            if (phys->body)
            {
                btVector3 v = phys->body->getLinearVelocity();
                return { v.x(), v.y(), v.z() };
            }

        return glm::vec3(0.0f);
    }

    void MovePosition(const glm::vec3& position)
    {
        if (auto phys = GetPhysics())
        {
            if (phys->body && isKinematic)
            {
                btTransform t = phys->body->getWorldTransform();
                t.setOrigin(btVector3(position.x, position.y, position.z));
                phys->body->setWorldTransform(t);
            }
        }
    }

    void Sleep()
    {
        if (auto phys = GetPhysics())
            if (phys->body)
                phys->body->forceActivationState(ISLAND_SLEEPING);
    }

    void WakeUp()
    {
        if (auto phys = GetPhysics())
            if (phys->body)
                phys->body->activate(true);
    }

private:
    std::shared_ptr<PhysicsComponent> GetPhysics() const
    {
        if (auto e = entity.lock())
            return e->GetComponent<PhysicsComponent>();
        return nullptr;
    }
};

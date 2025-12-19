#pragma once
#include <engine/ec/entity.hpp>
#include <glm/glm.hpp>
#include <imgui.h> // Include ImGui header

class Rigidbody : public Component
{
public:
    float mass = 1.0f;
    float inverseMass = 1.0f;

    glm::vec3 velocity{0.0f};
    glm::vec3 forceAccumulator{0.0f};

    glm::vec3 gravity{0.0f, -9.81f, 0.0f};

    bool useGravity = true;
    bool isKinematic = false;

    Rigidbody()
    {
        RecalculateMass();
    }

    void RecalculateMass()
    {
        inverseMass = (mass > 0.0f) ? 1.0f / mass : 0.0f;
    }

    void AddForce(const glm::vec3& force)
    {
        forceAccumulator += force;
    }

    void FixedUpdate(float dt)
    {
        if (isKinematic)
            return;

        auto e = entity.lock();
        if (!e || !e->transform)
            return;

        // Gravity
        if (useGravity)
            forceAccumulator += gravity * mass;

        // Acceleration
        glm::vec3 acceleration = forceAccumulator * inverseMass;

        // Integrate velocity
        velocity += acceleration * dt;

        // Integrate position
        e->transform->position += velocity * dt;

        // Clear forces
        forceAccumulator = glm::vec3(0.0f);
    }

    void DrawImGuiControls() override
    {
        ImGui::InputFloat("Mass", &mass);
        RecalculateMass(); // Recalculate inverse mass if mass changes
        ImGui::Checkbox("Use Gravity", &useGravity);
        ImGui::Checkbox("Is Kinematic", &isKinematic);
        ImGui::InputFloat3("Velocity", &velocity.x, "%.3f", ImGuiInputTextFlags_ReadOnly); // Display velocity
        // ImGui::InputFloat3("Force Accumulator", &forceAccumulator.x, "%.3f", ImGuiInputTextFlags_ReadOnly); // Display force accumulator
    }
};
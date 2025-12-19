#pragma once
#include <engine/ec/component.hpp>
#include <engine/components/transform.hpp>
#include <engine/ec/entity.hpp>

class Rotator : public Component
{
public:
    Rotator() = default;
    ~Rotator() = default;

    bool enabled = true; // Added for ImGui control

    void Update(float dt)
    {
        if (!enabled) return; // Only rotate if enabled

        // Access the entity's transform
        auto e = entity.lock();
        if (e && e->transform)
        {
            // Rotate around Y-axis for now (e.g., 45 degrees per second)
            // Convert degrees to radians for glm::vec3 rotation
            e->transform->rotation.y += glm::radians(45.0f) * dt;
        }
    }

    void DrawImGuiControls() override
    {
        ImGui::Checkbox("Enabled", &enabled);
    }
};
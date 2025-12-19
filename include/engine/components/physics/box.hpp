#pragma once
#include <engine/ec/entity.hpp>
#include <engine/components/physics/collider.hpp>
#include <glm/glm.hpp> // Required for glm::vec3
#include <imgui.h>     // Required for ImGui controls

class BoxCollider : public Collider
{
public:
    glm::vec3 size{1.0f};   // local size
    glm::vec3 offset{0.0f};

    BoxCollider() : size(1.0f, 1.0f, 1.0f) {}
    BoxCollider(const glm::vec3& s) : size(s) {}

    glm::vec3 Min() const
    {
        auto e = entity.lock();
        // Ensure entity and its transform exist before dereferencing
        if (e && e->transform) {
            return e->transform->position + offset - size * 0.5f;
        }
        return glm::vec3(0.0f); // Return default if entity or transform is invalid
    }

    glm::vec3 Max() const
    {
        auto e = entity.lock();
        // Ensure entity and its transform exist before dereferencing
        if (e && e->transform) {
            return e->transform->position + offset + size * 0.5f;
        }
        return glm::vec3(0.0f); // Return default if entity or transform is invalid
    }

    bool CheckCollision(Collider* other) override
    {
        auto box = dynamic_cast<BoxCollider*>(other);
        if (!box) return false;

        return (Min().x <= box->Max().x && Max().x >= box->Min().x) &&
               (Min().y <= box->Max().y && Max().y >= box->Min().y) &&
               (Min().z <= box->Max().z && Max().z >= box->Min().z);
    }

    void DrawImGuiControls() override
    {
        ImGui::InputFloat3("Size", &size.x);
        ImGui::InputFloat3("Offset", &offset.x);
    }
};
#include <engine/components/rotator.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/constants.hpp> // For glm::half_pi etc.

void Rotator::Update(float dt)
{
    // Access the entity's transform
    auto e = entity.lock();
    if (e && e->transform) {
        // Rotate around Y-axis for now (e.g., 45 degrees per second)
        // Convert degrees to radians for glm::vec3 rotation
        e->transform->rotation.y += glm::radians(45.0f) * dt;
    }
}
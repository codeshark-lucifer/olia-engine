#pragma once
#include <engine/ecs/component.hpp>
#include <engine/input.hpp>
#include <glm/glm.hpp>

class Looker : public Component
{
public:
    Looker(float sensitivity = 0.1f, float speed = 5.0f);

    void Update(float deltaTime) override;

private:
    float sensitivity;
    float speed = 5.0f;

    bool isFirstMouse = false;

    float yaw = 0.0f;
    float pitch = 0.0f;

    glm::vec3 cameraFront = glm::vec3(0.0f, 0.0f, -1.0f);
    glm::vec3 cameraUp = glm::vec3(0.0f, 1.0f, 0.0f);
};

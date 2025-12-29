#include <engine/components/looker.hpp>
#include <engine/ecs/entity.hpp>
#include <engine/input.hpp>
#include <glm/gtx/rotate_vector.hpp>

Looker::Looker(float sensitivity, float speed)
    : sensitivity(sensitivity), speed(speed)
{
}

void Looker::Update(float deltaTime)
{
    auto &input = InputManager::Get();
    auto en = entity.lock();
    if (!en)
        return;

    isFirstMouse = input.IsMouseDown(SDL_BUTTON_RIGHT);

    {
        // handle cursor
        if (isFirstMouse)
        {
            InputManager::Get().SetMouseLock(true);
        }
        else
        {
            InputManager::Get().SetMouseLock(false);
        }
    }
    if (!isFirstMouse)
        return;
    {
        // handle rotation
        yaw = input.GetMouseDX() * sensitivity;
        pitch = input.GetMouseDY() * sensitivity;

        glm::quat qYaw = glm::angleAxis(glm::radians(yaw), glm::vec3(0, -1, 0));
        glm::quat qPitch = glm::angleAxis(glm::radians(pitch), glm::vec3(-1, 0, 0));

        en->transform.rotation = glm::normalize(qYaw * en->transform.rotation * qPitch);
    }

    { // Get local axes
        glm::vec3 forward = en->transform.forward();
        glm::vec3 right = en->transform.right();
        glm::vec3 up = glm::vec3(0, 1, 0); // world up (or en->up())

        float x = 0.0f, y = 0.0f, z = 0.0f;

        // Input
        if (input.IsKeyDown(SDLK_W))
            z += 1.0f;
        if (input.IsKeyDown(SDLK_S))
            z -= 1.0f;
        if (input.IsKeyDown(SDLK_D))
            x += 1.0f;
        if (input.IsKeyDown(SDLK_A))
            x -= 1.0f;
        if (input.IsKeyDown(SDLK_E))
            y += 1.0f;
        if (input.IsKeyDown(SDLK_Q))
            y -= 1.0f;
        if (input.IsKeyDown(SDLK_SPACE))
            y += 1.0f;
        if (input.IsKeyDown(SDLK_LSHIFT))
            y -= 1.0f;

        // Combine in LOCAL space
        glm::vec3 direction =
            forward * z + right * x + up * y;

        // Prevent faster diagonal movement
        if (glm::length(direction) > 0.0f)
            direction = glm::normalize(direction);

        // Apply movement
        en->transform.position += direction * speed * deltaTime;
    }

}

#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/component.hpp>

class Camera
{
public:
    Camera(int w, int h)
    {
        this->width = w;
        this->height = h;
    }
    ~Camera() = default;

    void OnResize(int w, int h)
    {
        this->width = w;
        this->height = h;
    }

    glm::mat4 GetProjection()
    {
        return glm::perspective(glm::radians(fov), (float)width / (float)height, near, far);
    }

    glm::mat4 GetView()
    {
        direction = glm::normalize(position - target);
        right = glm::normalize(glm::cross(up, direction));
        return glm::lookAt(
            position,
            target,
            up);
    }

    glm::vec3 GetPosition() {
        return position;
    }

private:
    int width, height;
    glm::vec3 position = glm::vec3(0.0f, 0.0f, -5.0f);
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::vec3(0.0f);

    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 direction = glm::vec3(0.0f);

    float fov = 60.0f;
    float near = 0.3f;
    float far = 300.0f;
};
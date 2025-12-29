#pragma once
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/ecs/entity.hpp>
#include <engine/shader.hpp>

class Camera : public Component
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
        if (this->width == w && this->height == h)
            return;
        this->width = w;
        this->height = h;

        glViewport(0, 0, w, h);
    }

    glm::mat4 GetProjection()
    {
        return glm::perspective(glm::radians(fov), (float)width / (float)height, near, far);
    }

    glm::mat4 GetView()
    {
        if (auto en = entity.lock())
        {
            glm::vec3 pos = en->transform.position;
            glm::vec3 forward = en->transform.rotation * glm::vec3(0, 0, -1);
            glm::vec3 upDir = en->transform.rotation * glm::vec3(0, 1, 0);

            return glm::lookAt(pos, pos + forward, upDir);
        }

        return glm::mat4(1.0f);
    }

    void SetUniform(const Shader &shader)
    {
        shader.Use();
        shader.SetUniform("projection", GetProjection());
        shader.SetUniform("view", GetView());
        shader.SetUniform("viewPos", entity.lock()->transform.position);
    }

private:
    int width, height;
    glm::vec3 up = glm::vec3(0.0f, 1.0f, 0.0f);
    glm::vec3 right = glm::vec3(0.0f);

    glm::vec3 target = glm::vec3(0.0f, 0.0f, 0.0f);
    glm::vec3 direction = glm::vec3(0.0f);

    float fov = 60.0f;
    float near = 0.3f;
    float far = 300.0f;
};
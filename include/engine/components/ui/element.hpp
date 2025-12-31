#pragma once
#include <engine/ecs/entity.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>

class UIElement : public Component
{
public:
    glm::vec2 position{0.0f, 0.0f}; // pixels
    glm::vec2 size{100.0f, 100.0f}; // pixels

    glm::mat4 GetModelMatrix() const
    {
        glm::mat4 model(1.0f);
        model = glm::translate(model, glm::vec3(position, 0.0f));
        model = glm::scale(model, glm::vec3(size, 1.0f));
        return model;
    }

    virtual void Render() = 0;
};

#pragma once
#include <glad/glad.h>
#include <engine/ecs/entity.hpp>
#include <engine/shader.hpp>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <engine/components/ui/element.hpp>

class Canvas : public Component
{
public:
    Canvas(int w, int h)
        : width(w), height(h)
    {
        shader = std::make_shared<Shader>("assets/shaders/uielement.glsl");

        view = glm::mat4(1.0f);
        projection = glm::ortho(
            0.0f, (float)width,
            0.0f, (float)height,
            -1.0f, 1.0f);
    }

    void Render()
    {
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        
        glViewport(0, 0, width, height);
        
        shader->Use();
        shader->SetUniform("uView", view);
        shader->SetUniform("uProjection", projection);
        shader->SetUniform("uTexture", 0);
        
        DrawRecursive(entity.lock());
        glDisable(GL_BLEND);
    }

    void OnResize(int w, int h)
    {
        width = w;
        height = h;

        projection = glm::ortho(
            0.0f, (float)w,
            0.0f, (float)h,
            -1.0f, 1.0f);
    }

private:
    int width = 0, height = 0;
    glm::mat4 view;
    glm::mat4 projection;
    std::shared_ptr<Shader> shader;

    void DrawRecursive(const std::shared_ptr<Entity> &node)
    {
        for (auto &child : node->GetChildren())
        {
            if (auto element = child->GetComponent<UIElement>())
            {
                shader->SetUniform("uModel", element->GetModelMatrix());
                element->Render();
            }
            DrawRecursive(child);
        }
    }
};

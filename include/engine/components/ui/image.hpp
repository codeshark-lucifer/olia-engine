#pragma once
#include <engine/components/ui/element.hpp>
#include <engine/buffers/vao.hpp>
#include <engine/texture2D.hpp>
#include <memory>

class Image : public UIElement
{
public:
    Image(const std::string& path)
    {
        texture = std::make_shared<Texture2D>(path, Type::DIFFUSE, false);
        vao = CreateQuad();
    }

    void Render() override
    {
        texture->Bind(0);
        vao->Bind();
        glDrawElements(GL_TRIANGLES, 6, GL_UNSIGNED_INT, 0);
        vao->Unbind();
    }

private:
    std::shared_ptr<Texture2D> texture;
    std::shared_ptr<VAO> vao;

    std::shared_ptr<VAO> CreateQuad()
    {
        float vertices[] = {
            // x, y,   u, v
            0.0f, 0.0f, 0.0f, 0.0f,
            1.0f, 0.0f, 1.0f, 0.0f,
            1.0f, 1.0f, 1.0f, 1.0f,
            0.0f, 1.0f, 0.0f, 1.0f
        };

        unsigned int indices[] = { 0, 1, 2, 2, 3, 0 };

        auto vao = std::make_shared<VAO>();
        vao->Bind();

        unsigned int vbo, ebo;
        glGenBuffers(1, &vbo);
        glGenBuffers(1, &ebo);

        glBindBuffer(GL_ARRAY_BUFFER, vbo);
        glBufferData(GL_ARRAY_BUFFER, sizeof(vertices), vertices, GL_STATIC_DRAW);

        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ebo);
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, sizeof(indices), indices, GL_STATIC_DRAW);

        glVertexAttribPointer(0, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, 4 * sizeof(float), (void*)(2 * sizeof(float)));
        glEnableVertexAttribArray(1);

        vao->Unbind();
        return vao;
    }
};

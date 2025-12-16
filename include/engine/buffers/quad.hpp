#pragma once
#include <memory>
#include <engine/shader.hpp>
#include <engine/buffers/vao.hpp>
#include <engine/buffers/vbo.hpp>
#include <engine/buffers/ebo.hpp>

class Quad
{
public:
    Quad()
    {
        std::vector<Vertex> vertices = {
            // position                // normal           // uv
            {{-1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 0.0f}}, // bottom-left
            {{1.0f, -1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 0.0f}},  // bottom-right
            {{1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {1.0f, 1.0f}},   // top-right
            {{-1.0f, 1.0f, 0.0f}, {0.0f, 0.0f, 1.0f}, {0.0f, 1.0f}}   // top-left
        };
        std::vector<uint32_t> indices = {
            0, 1, 2,
            2, 3, 0};

        vao = std::make_unique<VAO>();
        vbo = std::make_unique<VBO>(vertices);
        ebo = std::make_unique<EBO>(indices);

        vao->Bind();
        ebo->Bind();
        vbo->Bind();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, TexCoord)));
        glEnableVertexAttribArray(1);

        vao->Unbind();
    }
    ~Quad() = default;

    void Draw(const Shader &shader, const unsigned int &texture)
    {
        shader.Use();
        glActiveTexture(GL_TEXTURE0);
        glBindTexture(GL_TEXTURE_2D, texture);
        shader.SetUniform("screenTexture", 0);

        vao->Bind();
        glDrawElements(GL_TRIANGLES, ebo->size(), GL_UNSIGNED_INT, 0);
        vao->Unbind();
    }

private:
    std::unique_ptr<VAO> vao = nullptr;
    std::unique_ptr<VBO> vbo = nullptr;
    std::unique_ptr<EBO> ebo = nullptr;
};
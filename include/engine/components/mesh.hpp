#pragma once
#include <memory>
#include <engine/shader.hpp>

#include <engine/buffers/vao.hpp>
#include <engine/buffers/vbo.hpp>
#include <engine/buffers/ebo.hpp>

#include <engine/texture2D.hpp>

class Mesh
{
public:
    Mesh(const std::vector<Vertex> &vertices, const std::vector<unsigned int> &indices, const std::vector<std::shared_ptr<Texture2D>> &textures)
    {
        for (auto &t : textures)
        {
            this->textures.push_back(std::move(t));
        }
        vao = std::make_unique<VAO>();
        vbo = std::make_unique<VBO<Vertex>>(vertices);
        ebo = std::make_unique<EBO>(indices);

        vao->Bind();
        ebo->Bind();
        vbo->Bind();

        glVertexAttribPointer(0, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)0);
        glEnableVertexAttribArray(0);

        glVertexAttribPointer(1, 3, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, normal)));
        glEnableVertexAttribArray(1);

        glVertexAttribPointer(2, 2, GL_FLOAT, GL_FALSE, sizeof(Vertex), (void *)(offsetof(Vertex, uv)));
        glEnableVertexAttribArray(2);

        vao->Unbind();

        if (this->textures.empty())
        {
            this->textures.push_back(LoadDefaultTexture());
        }
    }

    std::vector<glm::vec3> GetPoints()
    {
        return vbo->GetPoints();
    }

    std::vector<uint32_t> GetIndices()
    {
        return ebo->GetIndices();
    }

    void DrawDepth()
    {
        vao->Bind();
        glDrawElements(GL_TRIANGLES, ebo->size(), GL_UNSIGNED_INT, 0);
        vao->Unbind();
    }

    void Draw(const Shader &shader)
    {
        shader.Use();

        unsigned int diffuseNr = 1;
        unsigned int specularNr = 1;
        for (unsigned int i = 0; i < textures.size(); i++)
        {
            textures[i]->Bind(i);
            std::string number;
            std::string name;
            if (textures[i]->type == Type::DIFFUSE)
            {
                name = "diffuse_texture";
                number = std::to_string(diffuseNr++);
            }
            else if (textures[i]->type == Type::SPECULAR)
            {
                name = "specular_texture";
                number = std::to_string(specularNr++);
            }

            shader.SetUniform(name + number, (int)i);
        }

        // draw buffer
        vao->Bind();
        glDrawElements(GL_TRIANGLES, ebo->size(), GL_UNSIGNED_INT, 0);
        vao->Unbind();
    }

private:
    std::unique_ptr<VAO> vao = nullptr;
    std::unique_ptr<VBO<Vertex>> vbo = nullptr;
    std::unique_ptr<EBO> ebo = nullptr;
    std::vector<std::shared_ptr<Texture2D>> textures;
    std::shared_ptr<Texture2D> LoadDefaultTexture()
    {
        return std::make_shared<Texture2D>("assets/textures/default_sprite.png");
    }
};
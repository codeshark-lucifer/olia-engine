#pragma once
#include <glad/glad.h>
#include <glm/glm.hpp>
#include <vector>
#include <iostream>

struct Vertex
{
    glm::vec3 Position;
    glm::vec3 Normal;
    glm::vec2 TexCoord;
};

class VBO
{
public:
    VBO(const std::vector<Vertex> &vertices)
    {
        this->vertices = vertices;
        glGenBuffers(1, &ID);
        Bind();
        glBufferData(GL_ARRAY_BUFFER, vertices.size() * sizeof(Vertex), vertices.data(), GL_STATIC_DRAW);
    }
    
    ~VBO() {
        glDeleteBuffers(1, &ID);
    }

    void Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, ID);
    };

private:
    unsigned int ID;
    std::vector<Vertex> vertices;
};
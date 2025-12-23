#pragma once
#include <vector>
#include <glad/glad.h>

struct Vertex
{
    glm::vec3 position;
    glm::vec3 normal;
    glm::vec2 uv;
};

template <typename T>
class VBO
{
public:
    VBO(const std::vector<T> &data)
        : count(static_cast<unsigned int>(data.size()))
    {
        glGenBuffers(1, &id);
        Bind();
        glBufferData(
            GL_ARRAY_BUFFER,
            data.size() * sizeof(T),
            data.data(),
            GL_STATIC_DRAW);
    }

    ~VBO()
    {
        if (id)
            glDeleteBuffers(1, &id);
    }

    void Bind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, id);
    }

    void Unbind() const
    {
        glBindBuffer(GL_ARRAY_BUFFER, 0);
    }

    unsigned int Size() const { return count; }
    unsigned int GetID() const { return id; }

private:
    unsigned int id = 0;
    unsigned int count = 0;
};

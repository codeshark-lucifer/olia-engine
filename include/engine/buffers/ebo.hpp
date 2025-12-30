#pragma once
#include <vector>
#include <cstdint>
#include <glad/glad.h>

class EBO
{
public:
    EBO(const std::vector<unsigned int> &indices)
        : count(static_cast<unsigned int>(indices.size()))
    {
        this->indices = indices;
        glGenBuffers(1, &id);
        Bind();
        glBufferData(
            GL_ELEMENT_ARRAY_BUFFER,
            indices.size() * sizeof(unsigned int),
            indices.data(),
            GL_STATIC_DRAW);
    }

    ~EBO()
    {
        if (id)
            glDeleteBuffers(1, &id);
    }

    void Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, id);
    }

    void Unbind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, 0);
    }

    unsigned int size() const { return count; }
    unsigned int GetID() const { return id; }
    const std::vector<uint32_t> &GetIndices() const
    {
        return indices;
    }

private:
    unsigned int id = 0;
    unsigned int count = 0;
    std::vector<unsigned int> indices;
};

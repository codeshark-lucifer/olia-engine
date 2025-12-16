#pragma once
#include <glad/glad.h>
#include <vector>

class EBO
{
public:
    EBO(const std::vector<unsigned int> &indices)
    {
        this->indices = indices;
        glGenBuffers(1, &ID);
        Bind();
        glBufferData(GL_ELEMENT_ARRAY_BUFFER, indices.size() * sizeof(unsigned int), indices.data(), GL_STATIC_DRAW);
    }
    
    ~EBO()
    {
        glDeleteBuffers(1, &ID);
    }

    void Bind() const
    {
        glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, ID);
    }

    size_t size() const
    {
        return indices.size();
    }

private:
    unsigned int ID;
    std::vector<unsigned int> indices;
};
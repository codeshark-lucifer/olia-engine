#pragma once
#include <glad/glad.h>

class VAO
{
public:
    VAO()
    {
        glGenVertexArrays(1, &id);
    }

    ~VAO()
    {
        if (id)
            glDeleteVertexArrays(1, &id);
    }

    void Bind() const
    {
        glBindVertexArray(id);
    }

    void Unbind() const
    {
        glBindVertexArray(0);
    }

    unsigned int GetID() const { return id; }

private:
    unsigned int id = 0;
};

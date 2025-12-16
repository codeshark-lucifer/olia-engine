#pragma once
#include <glad/glad.h>

class VAO
{
public:
    VAO()
    {
        glGenVertexArrays(1, &ID);
        Bind();
    }
    
    ~VAO()
    {
        glDeleteVertexArrays(1, &ID);
    }

    void Bind() const
    {
        glBindVertexArray(ID);
    }

    void Unbind() const
    {
        glBindVertexArray(0);
    }

private:
    unsigned int ID;
};
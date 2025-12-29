#pragma once

#include <string>
#include <unordered_map>
#include <glad/glad.h>
#include <glm/glm.hpp>

class Shader
{
public:
    Shader(const std::string& filepath);
    Shader(const std::string& vertexPath, const std::string& fragmentPath);
    ~Shader();

    void Use() const;

    template<typename T>
    void SetUniform(const std::string& name, const T& value) const;

private:
    unsigned int Compile(GLenum type, const std::string& source);
    void LinkProgram();

    void CheckShaderCompile(unsigned int shader, const std::string& type);
    void CheckProgramLink(unsigned int program);

    std::string ReadFile(const std::string& filepath);
    unsigned int GetUniformLocation(const std::string& name) const;

private:
    unsigned int ID = 0;
    unsigned int vertexShader = 0;
    unsigned int fragmentShader = 0;

    mutable std::unordered_map<std::string, GLint> m_UniformCache;
};

#include "shader.inl"
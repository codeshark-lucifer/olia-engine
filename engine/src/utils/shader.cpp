#include <olia/utils/shader.h>

#include <glad/gl.h>

#include <iostream>

namespace Olia
{

    Shader::Shader()
    {
    }

    Shader::~Shader()
    {
        if (m_ID)
        {
            glDeleteProgram(m_ID);
        }
    }

    bool Shader::Create(
        const std::string &vertexSource,
        const std::string &fragmentSource)
    {

        uint32_t vertex =
            CompileShader(
                GL_VERTEX_SHADER,
                vertexSource);

        uint32_t fragment =
            CompileShader(
                GL_FRAGMENT_SHADER,
                fragmentSource);

        if (!vertex || !fragment)
        {
            return false;
        }

        m_ID = glCreateProgram();

        glAttachShader(
            m_ID,
            vertex);

        glAttachShader(
            m_ID,
            fragment);

        glLinkProgram(
            m_ID);

        CheckLinkError(
            m_ID);

        glDeleteShader(vertex);
        glDeleteShader(fragment);

        return true;
    }

    void Shader::Bind() const
    {
        glUseProgram(m_ID);
    }

    void Shader::Unbind() const
    {
        glUseProgram(0);
    }

    uint32_t Shader::CompileShader(
        uint32_t type,
        const std::string &source)
    {

        uint32_t shader =
            glCreateShader(type);

        const char *src =
            source.c_str();

        glShaderSource(
            shader,
            1,
            &src,
            nullptr);

        glCompileShader(shader);

        CheckCompileError(
            shader,
            type == GL_VERTEX_SHADER ? "VERTEX" : "FRAGMENT");

        int success;

        glGetShaderiv(
            shader,
            GL_COMPILE_STATUS,
            &success);

        if (!success)
        {
            glDeleteShader(shader);
            return 0;
        }

        return shader;
    }

    void Shader::CheckCompileError(
        uint32_t shader,
        const std::string &type)
    {

        int success;

        glGetShaderiv(
            shader,
            GL_COMPILE_STATUS,
            &success);

        if (!success)
        {

            char info[1024];

            glGetShaderInfoLog(
                shader,
                1024,
                nullptr,
                info);

            std::cerr
                << "Shader Compilation Error ["
                << type
                << "]\n"
                << info
                << std::endl;
        }
    }

    void Shader::CheckLinkError(
        uint32_t program)
    {

        int success;

        glGetProgramiv(
            program,
            GL_LINK_STATUS,
            &success);

        if (!success)
        {

            char info[1024];

            glGetProgramInfoLog(
                program,
                1024,
                nullptr,
                info);

            std::cerr
                << "Shader Link Error\n"
                << info
                << std::endl;
        }
    }

    int Shader::GetUniformLocation(
        const std::string &name)
    {

        auto it =
            m_UniformCache.find(name);

        if (it != m_UniformCache.end())
        {
            return it->second;
        }

        int location =
            glGetUniformLocation(
                m_ID,
                name.c_str());

        m_UniformCache[name] =
            location;

        return location;
    }

    void Shader::SetInt(
        const std::string &name,
        int value)
    {

        glUniform1i(
            GetUniformLocation(name),
            value);
    }

    void Shader::SetFloat(
        const std::string &name,
        float value)
    {

        glUniform1f(
            GetUniformLocation(name),
            value);
    }

    void Shader::SetVec2(
        const std::string &name,
        const glm::vec2 &value)
    {

        glUniform2f(
            GetUniformLocation(name),
            value.x,
            value.y);
    }

    void Shader::SetVec3(
        const std::string &name,
        const glm::vec3 &value)
    {

        glUniform3f(
            GetUniformLocation(name),
            value.x,
            value.y,
            value.z);
    }

    void Shader::SetVec4(
        const std::string &name,
        const glm::vec4 &value)
    {

        glUniform4f(
            GetUniformLocation(name),
            value.x,
            value.y,
            value.z,
            value.w);
    }

    void Shader::SetMat4(
        const std::string &name,
        const glm::mat4 &value)
    {

        glUniformMatrix4fv(
            GetUniformLocation(name),
            1,
            GL_FALSE,
            &value[0][0]);
    }

}
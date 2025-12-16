#pragma once
#include "shader.hpp"

// fallback
template<typename T>
void Shader::SetUniform(const std::string&, const T&) const
{
    static_assert(sizeof(T) == 0, "Unsupported uniform type");
}

// -------- Specializations --------

template<>
inline void Shader::SetUniform<int>(const std::string& name, const int& value) const
{
    glUniform1i(GetUniformLocation(name), value);
}

template<>
inline void Shader::SetUniform<float>(const std::string& name, const float& value) const
{
    glUniform1f(GetUniformLocation(name), value);
}

template<>
inline void Shader::SetUniform<glm::vec2>(const std::string& name, const glm::vec2& value) const
{
    glUniform2f(GetUniformLocation(name), value.x, value.y);
}

template<>
inline void Shader::SetUniform<glm::vec3>(const std::string& name, const glm::vec3& value) const
{
    glUniform3f(GetUniformLocation(name), value.x, value.y, value.z);
}

template<>
inline void Shader::SetUniform<glm::vec4>(const std::string& name, const glm::vec4& value) const
{
    glUniform4f(GetUniformLocation(name), value.x, value.y, value.z, value.w);
}

template<>
inline void Shader::SetUniform<glm::mat4>(const std::string& name, const glm::mat4& value) const
{
    glUniformMatrix4fv(
        GetUniformLocation(name),
        1,
        GL_FALSE,
        &value[0][0]
    );
}

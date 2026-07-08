#pragma once

#include <string>
#include <unordered_map>

#include <glm/glm.hpp>


namespace Olia
{

class Shader
{

private:

    uint32_t m_ID = 0;


    mutable std::unordered_map<std::string, int> m_UniformCache;



public:

    Shader();

    ~Shader();



    bool Create(
        const std::string& vertexSource,
        const std::string& fragmentSource
    );



    void Bind() const;

    void Unbind() const;



    // Uniform

    void SetInt(
        const std::string& name,
        int value
    );


    void SetFloat(
        const std::string& name,
        float value
    );


    void SetVec2(
        const std::string& name,
        const glm::vec2& value
    );


    void SetVec3(
        const std::string& name,
        const glm::vec3& value
    );


    void SetVec4(
        const std::string& name,
        const glm::vec4& value
    );


    void SetMat4(
        const std::string& name,
        const glm::mat4& value
    );



private:


    uint32_t CompileShader(
        uint32_t type,
        const std::string& source
    );


    int GetUniformLocation(
        const std::string& name
    );


    void CheckCompileError(
        uint32_t shader,
        const std::string& type
    );


    void CheckLinkError(
        uint32_t program
    );

};

}
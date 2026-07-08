#pragma once

#include <vector>

#include <glad/gl.h>
#include <glm/glm.hpp>

namespace Olia
{
    class Shader;

    struct Vertex
    {
        glm::vec3 position;
        glm::vec4 color;
        glm::vec2 texCoords;
        float texIndex;
    };

    class Batch
    {
    public:
        Batch() = default;
        ~Batch();

        void Init();

        void Begin();

        void DrawQuad(
            const glm::vec3 &position,
            const glm::vec2 &size,
            const glm::vec4 &color,
            uint32_t textureID = 0,
            float rotation = 0.0f);

        bool HasTextureSpace(uint32_t textureID) const;

        void End();

        void Flush(Shader &shader);

    private:
        GLuint m_VAO = 0;
        GLuint m_VBO = 0;
        GLuint m_EBO = 0;

        std::vector<Vertex> m_Vertices;
        std::vector<uint32_t> m_Indices;
        std::vector<uint32_t> m_TextureSlots;
    };

}
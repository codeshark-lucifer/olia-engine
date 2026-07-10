#pragma once
#include <core/built-in.h>
#include <renderer/batch.h>

namespace Olia
{
    class Renderer2D
    {
    public:
        void Init();

        void BeginScene(const Camera2D &);

        void DrawSprite(
            const Transform &,
            const SpriteRenderer &);

        void EndScene();

        void DrawQuad(
            const glm::vec3 &position,
            const glm::vec2 &size,
            const glm::vec4 &color,
            uint32_t textureID = 0,
            float rotation = 0.0f);

        void DrawQuad(
            const glm::vec3 &position,
            const glm::vec2 &size,
            const glm::vec4 &color,
            uint32_t textureID,
            const glm::vec2 texCoords[4],
            float rotation = 0.0f);

    private:
        Batch batch;
    };
} // namespace Olia

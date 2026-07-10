#include <renderer/renderer2D.h>

#include <glm/gtc/matrix_transform.hpp>

#include <olia/olia.h>

namespace Olia
{
    void Renderer2D::Init()
    {
        batch.Init();
    }

    void Renderer2D::BeginScene(const Camera2D &camera)
    {
        glm::mat4 projection = glm::ortho(
            0.0f,
            camera.width,
            camera.height,
            0.0f,
            -1.0f,
            1.0f);

        context.shader->Bind();
        context.shader->SetMat4("u_Transform", projection);

        batch.Begin();
    }

    void Renderer2D::DrawSprite(
        const Transform &transform,
        const SpriteRenderer &sprite)
    {
        uint32_t textureID = 0;
        if (sprite.texture)
        {
            textureID = sprite.texture->id;
        }

        if (sprite.useTexCoords)
        {
            batch.DrawQuad(
                transform.position,
                sprite.size,
                sprite.color,
                textureID,
                sprite.texCoords,
                transform.rotation.z);
        }
        else
        {
            batch.DrawQuad(
                transform.position,
                sprite.size,
                sprite.color,
                textureID,
                transform.rotation.z);
        }
    }

    void Renderer2D::EndScene()
    {
        batch.End();
        batch.Flush(*context.shader);
    }

    void Renderer2D::DrawQuad(
        const glm::vec3 &position,
        const glm::vec2 &size,
        const glm::vec4 &color,
        uint32_t textureID,
        float rotation)
    {
        batch.DrawQuad(position, size, color, textureID, rotation);
    }

    void Renderer2D::DrawQuad(
        const glm::vec3 &position,
        const glm::vec2 &size,
        const glm::vec4 &color,
        uint32_t textureID,
        const glm::vec2 texCoords[4],
        float rotation)
    {
        batch.DrawQuad(position, size, color, textureID, texCoords, rotation);
    }
}
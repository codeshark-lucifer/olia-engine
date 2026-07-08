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

        batch.DrawQuad(
            transform.position,
            sprite.size,
            sprite.color,
            textureID,
            transform.rotation.z);
    }

    void Renderer2D::EndScene()
    {
        batch.End();
        batch.Flush(*context.shader);
    }
}
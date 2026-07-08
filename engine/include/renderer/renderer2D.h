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

    private:
        Batch batch;
    };
} // namespace Olia

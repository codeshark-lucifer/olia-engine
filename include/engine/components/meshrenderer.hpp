#pragma once
#include <engine/ecs/entity.hpp>
#include <engine/components/meshfilter.hpp>

class MeshRenderer : public Component
{
public:
    MeshRenderer() {}
    void Render(const Shader &shader)
    {
        auto en = entity.lock();
        if (en->HasComponent<MeshFilter>())
        {
            shader.Use();
            shader.SetUniform("model", en->matrix());
            en->GetComponent<MeshFilter>()->mesh->Draw(shader);
        }
    }
};
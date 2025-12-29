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
        auto render = en->GetComponent<MeshFilter>();
        if (render)
        {
            shader.Use();
            shader.SetUniform("model", en->WorldMatrix());
            en->GetComponent<MeshFilter>()->mesh->Draw(shader);
        }
    }
};
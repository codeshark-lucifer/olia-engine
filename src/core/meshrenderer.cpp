#include <engine/meshrenderer.hpp>
#include <engine/utils/entity.hpp>
#include <engine/meshfilter.hpp>
#include <engine/shader.hpp>

void MeshRenderer::Render(Shader &shader)
{
    shader.Use();

    auto go = entity.lock();
    if (!go || !go->transform)
        return;

    shader.SetUniform("model", go->transform->GetWorldMatrix());

    auto comps = go->GetComponents<MeshFilter>();
    for (auto &c : comps)
    {
        if (!c || !c->mesh)
            continue;
        c->mesh->Draw(shader);
    }
}

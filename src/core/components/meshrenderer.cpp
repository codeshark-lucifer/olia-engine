#include <engine/components/meshrenderer.hpp>
#include <engine/components/meshfilter.hpp>
#include <engine/components/mesh.hpp>
#include <engine/shader.hpp>
#include <engine/ec/entity.hpp>

void MeshRenderer::Render(const Shader& shader)
{
    auto en = entity.lock();
    if (!en) {
        return;
    }

    auto filter = en->GetComponent<MeshFilter>();
    if (!filter) {
        return;
    }
    if (!filter->mesh) {
        return;
    }

    // Get the transform component and set the model matrix uniform
    if (en->transform) { // Check if transform exists
        shader.SetUniform("model", en->transform->GetWorldMatrix());
    }

    filter->mesh->Draw(shader);
}

#include "engine/olia.h"
#include "core/ecs/ecs.h"

#include "components/mesh-renderer.h"
#include "engine/engine.h"

Entity CreateMesh(std::vector<Vertex> &vertices, std::vector<uint16_t> &indices){
    Entity entity = ecs->Create();
    std::unique_ptr<Mesh> mesh = std::make_unique<Mesh>();
    mesh->vertices = vertices;
    mesh->indices = indices;

    auto& renderer = ecs->Add<MeshRenderer>(entity);
    renderer.SetMesh(std::move(mesh));
    return entity;
}
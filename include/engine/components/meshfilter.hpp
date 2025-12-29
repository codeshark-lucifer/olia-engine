#pragma once
#include <engine/ecs/component.hpp>
#include <engine/components/mesh.hpp>

class MeshFilter: public Component
{
public:
    MeshFilter(const std::shared_ptr<Mesh> &m) : mesh(m) {}
    std::shared_ptr<Mesh> mesh = nullptr;
};
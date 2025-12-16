#pragma once
#include <memory>
#include <engine/component.hpp>
#include <engine/mesh.hpp>

class Component;
class MeshFilter : public Component
{
public:
    MeshFilter() = default;
    MeshFilter(const std::shared_ptr<Mesh> &m) : mesh(m) {}

    std::shared_ptr<Mesh> mesh = nullptr;
};

#pragma once
#include <engine/ec/component.hpp>
#include <memory>

class Mesh;

class MeshFilter : public Component
{
public:
    std::shared_ptr<Mesh> mesh;

    MeshFilter(const std::shared_ptr<Mesh>& m)
        : mesh(m) {}
};

#pragma once
#include "component.hpp"

class Shader;
class MeshFilter;
class Entity;

class MeshRenderer : public Component
{
public:
    ~MeshRenderer() = default;
    void Render(Shader &shader);
};

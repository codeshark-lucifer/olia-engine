#pragma once
#include <engine/ec/component.hpp>
#include <memory>

class Shader;
class MeshFilter;

class MeshRenderer : public Component
{
public:
    void Render(const Shader& shader);
};
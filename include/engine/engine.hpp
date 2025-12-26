#pragma once
#include <memory>
#include <engine/window/platform.hpp>
#include <engine/scene.hpp>
#include <engine/ecs/entity.hpp>
#include <engine/input.hpp>
#include <glm/glm.hpp>

class Engine
{
public:
    Engine();
    ~Engine() = default;
    void Run();

private:
    std::shared_ptr<Scene> scene = nullptr;
};
#pragma once
#include <memory>
#include <engine/window/platform.hpp>
#include <engine/scene.hpp>

class Engine
{
public:
    Engine();
    ~Engine() = default;
    void Run();

private:
    std::shared_ptr<Platform> platform = nullptr;
    std::shared_ptr<Scene> scene = nullptr;
};
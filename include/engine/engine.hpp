#pragma once

#include <memory>
#include <engine/render/platform.hpp>
#include <engine/scene.hpp>

class Engine
{
public:
    Engine();
    ~Engine();

    void Run();
    void callback(int w, int h);

private:
    std::unique_ptr<Platform> platform;
    std::unique_ptr<Scene> scene;
    std::weak_ptr<Entity> selectedEntity;
};

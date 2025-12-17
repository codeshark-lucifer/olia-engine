#pragma once
#include <memory>
#include <engine/graphics/platform.hpp>
#include <engine/scene.hpp>
#include <engine/texture2D.hpp>
#include <engine/mesh.hpp>
#include <engine/meshfilter.hpp>
#include <engine/meshrenderer.hpp>
#include <engine/assetloader.hpp>
#include <engine/input/input.hpp>

class Engine
{
public:
    Engine();
    ~Engine() = default;

    void Run() const;

    static void callback(int w, int h);

private:
    std::unique_ptr<Platform> platform = nullptr;
    std::unique_ptr<Scene> scene = nullptr;

    static Engine *instance;
};
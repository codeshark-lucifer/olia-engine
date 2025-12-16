#include "engine/engine.hpp"
#include <engine/mesh.hpp>
#include <engine/meshfilter.hpp>
#include <engine/meshrenderer.hpp>
#include <engine/assetloader.hpp>
#include <engine/components/rotator.hpp>

Engine::Engine()
{
    instance = this;
    platform = std::make_unique<Platform>();
    platform->callback = callback;

    scene = std::make_unique<Scene>("SampleScene");

    model = std::make_unique<asset::Model>("assets/models/cube.fbx");
    scene->AddEntity(model->root);
    model->root->AddComponent<Rotator>(); // Add Rotator component
}

Engine *Engine::instance = nullptr;
void Engine::callback(int w, int h)
{
    if (instance && instance->scene)
    {
        instance->scene->OnResize(w, h);
    }
}

void Engine::Run() const
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    scene->Begin();

    glClearColor(0.1f, 0.1f, 0.1f, 1.0f);
    Uint32 lastTime = SDL_GetTicks(); // time at last frame
    Uint32 currentTime;
    float deltaTime;
    while (!platform->ShouldClose())
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
        platform->PollEvent();

        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f; // milliseconds → seconds
        lastTime = currentTime;

        scene->Update(deltaTime);

        platform->SwapBuffers();
    }
}

#include <engine/engine.hpp>
#include <engine/config.h>
#include <engine/components/camera.hpp>
#include <iostream>
#include <engine/model.hpp>

Engine::Engine()
{
    platform = std::make_shared<Platform>(SCREEN_WIDTH, SCREEN_HEIGHT, APPLICATION_TITLE);
    scene = std::make_shared<Scene>(SCREEN_WIDTH, SCREEN_HEIGHT, "SampleScene");
    platform->callback = [this](int w, int h)
    {
        scene->OnResize(w, h);
    };
    std::cout << "Initialize Engine.\n";

    auto camera = scene->AddEntity(std::make_shared<Entity>("Main Camera"), scene);
    camera->AddComponent<Camera>(SCREEN_HEIGHT, SCREEN_HEIGHT);
    camera->position = glm::vec3(0.0f, 0.0f, 5.0f);

    // auto cube = Model::Load("assets/models/cube.fbx", scene);
    auto cube = Model::Load("assets/models/octahedron-sharpe.fbx", scene);
    scene->AddEntity(cube, scene);
}

void Engine::Run()
{
    std::cout << "Running...\n";
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 last = now;
    double deltaTime = 0.0;
    while (!platform->ShouldClose())
    {
        platform->PollEvent();

        // Calculate delta time in seconds
        now = SDL_GetPerformanceCounter();
        Uint64 freq = SDL_GetPerformanceFrequency();
        deltaTime = (double)(now - last) / (double)freq;
        last = now;

        // Update
        scene->Update(deltaTime);

        platform->SwapBuffer();
    }
}
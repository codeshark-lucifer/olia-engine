#include <engine/engine.hpp>
#include <engine/config.h>
#include <engine/components/camera.hpp>
#include <iostream>
#include <engine/model.hpp>

#include <engine/components/rotator.hpp>
#include <engine/components/physics/rigidbody.hpp>
#include <engine/components/physics/box.hpp>
#include <engine/components/physics/sphere.hpp>

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

    auto sphere = Model::Load("assets/models/cube.fbx", scene);
    sphere->position = glm::vec3(0.8f, 2.0f, 0.8f);
    // sphere->AddComponent<SphereCollider>();
    sphere->AddComponent<Rigidbody>();
    sphere->AddComponent<BoxCollider>();

    auto sphere1 = Model::Load("assets/models/cube.fbx", scene);
    sphere1->position = glm::vec3(0.0f, -2.0f, 0.0f);
    // auto rig = sphere1->AddComponent<Rigidbody>();
    // rig->isUseGravity = false;
    sphere1->AddComponent<BoxCollider>();
}

void Engine::Run()
{
    std::cout << "Running...\n";
    scene->Begin();
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
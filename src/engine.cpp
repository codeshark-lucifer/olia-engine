#include <engine/engine.hpp>
#include <engine/config.h>
#include <engine/components/camera.hpp>
#include <iostream>
#include <glm/gtx/string_cast.hpp> // For glm::to_string
#include <engine/model.hpp>
#include <engine/input.hpp>

#include <engine/components/rotator.hpp>
#include <engine/components/physics/rigidbody.hpp>
#include <engine/components/physics/box.hpp>
#include <engine/components/physics/sphere.hpp>

#include <engine/components/looker.hpp>
#include <engine/components/player.hpp>

Engine::Engine()
{
    std::cout << "Engine::Engine - Initializing platform and scene." << std::endl;
    Platform::Get().Init(SCREEN_WIDTH, SCREEN_HEIGHT, APPLICATION_TITLE);
    scene = std::make_shared<Scene>(SCREEN_WIDTH, SCREEN_HEIGHT, "SampleScene");
    Platform::Get().callback = [this](int w, int h)
    {
        scene->OnResize(w, h);
    };
    std::cout << "[INFO] Initialize Engine.\n";

    auto camera = scene->AddEntity(std::make_shared<Entity>("Main Camera"), scene);
    camera->AddComponent<Camera>(SCREEN_HEIGHT, SCREEN_HEIGHT);
    camera->AddComponent<Looker>();
    camera->position = glm::vec3(0.0f, 0.0f, 5.0f);
    std::cout << "Engine::Engine - Added Camera entity with ID: " << camera->getID() << std::endl;

    auto sphere = Model::Load("assets/models/sphere-smooth.fbx", scene);
    sphere->position = glm::vec3(0.0f, 0.5f, 0.0f); // Changed position
    sphere->AddComponent<SphereCollider>();
    sphere->AddComponent<Rigidbody>();
    sphere->AddComponent<Player>();
    // sphere->AddComponent<BoxCollider>();

    auto sphere1 = Model::Load("assets/models/cube.fbx", scene);
    sphere1->position = glm::vec3(0.0f, -2.5f, 0.0f); // Changed position
    // sphere1->AddComponent<Rigidbody>(); // Uncommented this line
    sphere1->AddComponent<BoxCollider>();
}

void Engine::Run()
{
    scene->Begin();
    Uint64 now = SDL_GetPerformanceCounter();
    Uint64 last = now;
    double deltaTime = 0.0;
    while (!Platform::Get().ShouldClose())
    {
        Platform::Get().PollEvent();
        InputManager::Get().Update();

        // Calculate delta time in seconds
        now = SDL_GetPerformanceCounter();
        Uint64 freq = SDL_GetPerformanceFrequency();
        deltaTime = (double)(now - last) / (double)freq;
        last = now;

        // Update
        auto &input = InputManager::Get();
        if (input.IsKeyDown(SDLK_ESCAPE))
            input.SetMouseLock(false);

        scene->Update(deltaTime);

        Platform::Get().SwapBuffer();
    }
    std::cout << "Engine::Run - Exiting engine loop." << std::endl;
}
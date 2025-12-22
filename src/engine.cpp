#include <engine/engine.hpp>

#include <engine/render/platform.hpp>
#include <engine/scene.hpp>
#include <engine/assetloader.hpp>
#include <engine/config.h>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <engine/components/rotator.hpp>
#include <engine/components/rigidbody.hpp>

// ImGui includes
#include "imgui.h"
#include "imgui_impl_sdl3.h"
#include "imgui_impl_opengl3.h"

Engine::Engine()
{
    platform = std::make_unique<Platform>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        APPLICATION_NAME);

    scene = std::make_unique<Scene>(
        SCREEN_WIDTH,
        SCREEN_HEIGHT,
        APPLICATION_NAME);

    asset::Model cube("assets/models/octahedron-sharpe.fbx");
    cube.root->AddComponent<Rotator>();
    cube.root->AddComponent<Rigidbody>();
    scene->AddEntity(cube.root);
    
    asset::Model plane("assets/models/plane.fbx");
    plane.root->transform->position = glm::vec3(0.0f, -2.0f, 0.0f);
    // plane.root->AddComponent<BoxCollider>();
    scene->AddEntity(plane.root);

}

Engine::~Engine()
{
}

void Engine::callback(int w, int h)
{
    if (scene)
    {
        scene->OnResize(w, h);
    }
}

void Engine::Run()
{
    glEnable(GL_DEPTH_TEST);
    glDepthFunc(GL_LESS);

    scene->Begin();

    Uint32 lastTime = SDL_GetTicks();
    Uint32 currentTime;
    float deltaTime = 0.0f;

    while (!platform->ShouldClose())
    {
        platform->PollEvent();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        scene->Update(deltaTime);

        platform->SwapBuffers();
    }
}

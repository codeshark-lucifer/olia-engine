#include <engine/engine.hpp>

#include <engine/render/platform.hpp>
#include <engine/scene.hpp>
#include <engine/assetloader.hpp>
#include <engine/config.h>

#include <SDL3/SDL.h>
#include <glad/glad.h>

#include <engine/components/rotator.hpp>
#include <engine/components/physics/box.hpp>

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

    asset::Model cube("assets/models/cube.fbx");
    cube.root->AddComponent<Rotator>();
    cube.root->AddComponent<BoxCollider>();
    scene->AddEntity(cube.root);
    
    asset::Model plane("assets/models/plane.fbx");
    plane.root->transform->position = glm::vec3(0.0f, -2.0f, 0.0f);
    plane.root->AddComponent<BoxCollider>();
    scene->AddEntity(plane.root);

    // ImGui Initialization
    IMGUI_CHECKVERSION();
    ImGui::CreateContext();
    ImGuiIO& io = ImGui::GetIO(); (void)io;
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableKeyboard;       // Enable Keyboard Controls
    io.ConfigFlags |= ImGuiConfigFlags_NavEnableGamepad;        // Enable Gamepad Controls

    ImGui::StyleColorsDark(); // Or ImGui::StyleColorsLight();

    ImGui_ImplSDL3_InitForOpenGL(platform->GetWindow(), platform->GetGLContext());
    ImGui_ImplOpenGL3_Init("#version 330");
}

Engine::~Engine()
{
    // ImGui Shutdown
    ImGui_ImplOpenGL3_Shutdown();
    ImGui_ImplSDL3_Shutdown();
    ImGui::DestroyContext();
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

        // ImGui New Frame
        ImGui_ImplOpenGL3_NewFrame();
        ImGui_ImplSDL3_NewFrame();
        ImGui::NewFrame();

        // ImGui Hierarchy Window
        ImGui::Begin("Hierarchy");
        // Recursive lambda to draw entities
        std::function<void(std::shared_ptr<Entity>)> DrawEntityNode = 
            [&](std::shared_ptr<Entity> entity) {
            ImGuiTreeNodeFlags node_flags = ImGuiTreeNodeFlags_OpenOnArrow | ImGuiTreeNodeFlags_OpenOnDoubleClick;
            if (entity == selectedEntity.lock()) { // Check if this entity is selected
                node_flags |= ImGuiTreeNodeFlags_Selected;
            }
            if (entity->children.empty()) { // If no children, it's a leaf node
                node_flags |= ImGuiTreeNodeFlags_Leaf | ImGuiTreeNodeFlags_NoTreePushOnOpen;
            }

            bool node_open = ImGui::TreeNodeEx((void*)entity.get(), node_flags, "%s", entity->name.c_str());

            if (ImGui::IsItemClicked()) {
                selectedEntity = entity; // Select this entity
            }

            if (node_open && !entity->children.empty()) {
                for (auto& child : entity->children) {
                    DrawEntityNode(child); // Recursively draw children
                }
                ImGui::TreePop();
            } else if (node_open && entity->children.empty()) { // If it's a leaf node but TreeNodeEx returned true
                // Removed redundant ImGui::TreePop() as NoTreePushOnOpen was used
            }
        };

        // Iterate through root entities
        for (auto& entity : scene->GetEntities()) { // Using scene->GetEntities() as root list
            DrawEntityNode(entity);
        }
        ImGui::End();

        // ImGui Inspector Window
        ImGui::Begin("Inspector");
        if (auto selected = selectedEntity.lock()) {
            ImGui::Text("Entity: %s", selected->name.c_str());

            // Transform properties
            if (ImGui::CollapsingHeader("Transform", ImGuiTreeNodeFlags_DefaultOpen)) {
                ImGui::InputFloat3("Position", &selected->transform->position.x);
                ImGui::InputFloat3("Rotation", &selected->transform->rotation.x);
                ImGui::InputFloat3("Scale", &selected->transform->scale.x);
            }

            // Component properties
            for (auto& component : selected->components) {
                // Get component name for header
                const char* componentName = typeid(*component).name(); 
                // This will be mangled, but serves as a placeholder.
                // A better approach would be to have a virtual GetTypeName() in Component.

                if (ImGui::CollapsingHeader(componentName, ImGuiTreeNodeFlags_DefaultOpen)) {
                    component->DrawImGuiControls();
                }
            }
        } else {
            ImGui::Text("No entity selected.");
        }
        ImGui::End();

        // ImGui Content
        ImGui::Begin("Debug Window");
        ImGui::Text("Hello, ImGui!");
        ImGui::End();

        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);

        currentTime = SDL_GetTicks();
        deltaTime = (currentTime - lastTime) / 1000.0f;
        lastTime = currentTime;

        scene->Update(deltaTime);

        // ImGui Rendering
        ImGui::Render();
        ImGui_ImplOpenGL3_RenderDrawData(ImGui::GetDrawData());

        platform->SwapBuffers();
    }
}

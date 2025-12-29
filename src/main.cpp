#include <iostream>
#include <memory>
#include <engine/configure.hpp>
#include <engine/window/platform.hpp>
#include <engine/scene.hpp>
#include <engine/input.hpp>
#include <engine/model.hpp>

#include <engine/components/camera.hpp>
#include <engine/components/looker.hpp>
#include <engine/components/light.hpp>

#include <engine/components/physics/sphere.hpp>
#include <engine/components/physics/box.hpp>

std::shared_ptr<Scene> scene;

void PrintBanner()
{
    std::cout << "=============================================\n";
    std::cout << "        PHYSICS ENGINE - @CODESHARK         \n";
    std::cout << "=============================================\n";
    std::cout << "        Initializing Engine Components       \n";
    std::cout << "---------------------------------------------\n";
    std::cout << "Screen Resolution : " << SCREEN_WIDTH << " x " << SCREEN_HEIGHT << "\n";
    std::cout << "Application Title : " << APPLICATION_TITLE << "\n";
    std::cout << "---------------------------------------------\n";
    std::cout << "Engine Ready. Starting Main Loop...\n";
    std::cout << "=============================================\n\n";
}

int main()
{
    try
    {
        PrintBanner();

        Platform::Get().Init(SCREEN_WIDTH, SCREEN_HEIGHT, APPLICATION_TITLE);
        scene = std::make_shared<Scene>(SCREEN_WIDTH, SCREEN_HEIGHT, "SampleScene");

        auto camera = scene->CreateObject("MainCamera");
        camera->AddComponent<Camera>(SCREEN_WIDTH, SCREEN_HEIGHT);
        camera->transform.position = {0, 0, 5};
        camera->AddComponent<Looker>();

        {
            auto sun = scene->CreateObject("SunLight");
            auto light = sun->AddComponent<Light>();
            light->type = LightType::Directional;
            light->direction = glm::normalize(glm::vec3(-2.0f, -4.0f, -1.0f));
            light->color = glm::vec3(1.0f);
            light->intensity = 0.1f;
        }

        {
            auto point = scene->CreateObject("PointLight");
            auto light = point->AddComponent<Light>();
            light->type = LightType::Point;
            light->color = glm::vec3(1.0f, 0.95f, 0.8f);
            light->intensity = 5.0f;
            light->range = 50.0f;
            point->transform.position = {-4.0f, -2.0f, -4.0f};
        }

        {
            auto spot = scene->CreateObject("SpotLight");
            auto light = spot->AddComponent<Light>();
            light->type = LightType::Spot;
            light->color = glm::vec3(0.8f, 0.8f, 1.0f);
            light->intensity = 10.0f;
            light->range = 5.0f;
            spot->transform.position = {0.0f, 4.0f, 4.0f};
            light->direction = glm::normalize(glm::vec3(0.0f, -1.0f, -1.0f));
        }

        auto model0 = Model::Load("assets/models/octahedron-sharpe.fbx", scene);
        model0->transform.position = {0.0f, 2.0f, 0.0f};
        model0->AddComponent<SphereCollider>();
        auto rb = model0->AddComponent<Rigidbody>();

        auto model1 = Model::Load("assets/models/cube.fbx", scene);
        model1->transform.position = {0.0f, -2.0f, 0.0f};
        model1->AddComponent<BoxCollider>();

        Platform::Get().SetCallback([](int width, int height)
                                    { scene->OnResize(width, height); });

        scene->Begin();

        while (!Platform::Get().ShouldClose())
        {
            Platform::Get().PollEvent();
            InputManager::Get().Update();

            float deltaTime = Platform::Get().GetDeltaTime();

            scene->Update(deltaTime);

            Platform::Get().SwapBuffer();
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "[Exception]: " << e.what() << std::endl;
    }

    std::cout << "\n=============================================\n";
    std::cout << "       Engine Shutdown. Goodbye!             \n";
    std::cout << "=============================================\n";
    return 0;
}

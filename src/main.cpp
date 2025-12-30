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

#include <engine/components/physics/rigidbody3d.hpp>
#include <engine/components/physics/collider/boxcollider3d.hpp>

std::shared_ptr<Scene> scene;

void PrintBanner()
{
    std::cout << "=============================================\n";
    std::cout << "        PHYSICS ENGINE - @CODESHARK           \n";
    std::cout << "=============================================\n\n";
}

int main()
{
    try
    {
        PrintBanner();

        Platform::Get().Initialize(SCREEN_WIDTH, SCREEN_HEIGHT, APPLICATION_TITLE);
        PhysicsSystem::Get().Initialize();
        scene = std::make_shared<Scene>(SCREEN_WIDTH, SCREEN_HEIGHT, "SampleScene");

        auto camera = scene->CreateObject("MainCamera");
        camera->AddComponent<Camera>(SCREEN_WIDTH, SCREEN_HEIGHT);
        camera->AddComponent<Looker>();
        camera->transform.position = {0.0f, 0.0f, 5.0f};

        {
            auto sun = scene->CreateObject("SunLight");
            auto l = sun->AddComponent<Light>();
            l->type = LightType::Directional;
            l->direction = glm::normalize(glm::vec3(-1.0f, -2.0f, -1.0f));
            l->color = glm::vec3(1.0f);
            l->intensity = 0.2f;
        }

        {
            auto point = scene->CreateObject("PointLight");
            auto l = point->AddComponent<Light>();
            l->type = LightType::Point;
            l->intensity = 6.0f;
            l->range = 40.0f;
            point->transform.position = {-3.0f, 3.0f, 2.0f};
        }

        auto model = Model::Load("assets/models/cube.fbx", scene);
        model->AddComponent<BoxCollider3D>();
        model->AddComponent<RigidBody3D>();
        model->transform.position = {0.0f, 1.0f, 0.0f};

        auto model1 = Model::Load("assets/models/cube.fbx", scene);
        model1->AddComponent<BoxCollider3D>();
        model1->transform.position.y = -3.0f;
        // model1->AddComponent<RigidBody3D>();

        Platform::Get().SetCallback([](int w, int h)
                                    { scene->OnResize(w, h); });

        scene->Begin();

        while (!Platform::Get().ShouldClose())
        {
            Platform::Get().PollEvent();
            InputManager::Get().Update();

            float dt = Platform::Get().GetDeltaTime();
            scene->Update(dt);

            Platform::Get().SwapBuffer();
        }
    }
    catch (const std::exception &e)
    {
        std::cout << "[Exception] " << e.what() << "\n";
    }

    return 0;
}

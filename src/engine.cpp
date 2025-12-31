#include <engine/engine.hpp>
#include <iostream>

namespace Engine
{
    void Engine::Initialize(int width, int height, const char *title)
    {
        screenWidth = width;
        screenHeight = height;
        appTitle = title;

        std::cout << "=============================================\n";
        std::cout << "          ENGINE INITIALIZATION             \n";
        std::cout << "=============================================\n\n";

        // Initialize platform/window
        Platform::Get().Initialize(screenWidth, screenHeight, appTitle);

        // Initialize physics
        PhysicsSystem::Get().Initialize();

        // Create main scene
        scene = std::make_shared<Scene>(screenWidth, screenHeight, "MainScene");

        SetupDefaultScene();

        // Window resize callback
        Platform::Get().ListenCallback([this](int w, int h)
                                       { scene->OnResize(w, h); });
    }

    void Engine::SetupDefaultScene()
    {
        // Create main camera
        auto camera = scene->CreateObject("MainCamera");
        camera->AddComponent<Camera>(screenWidth, screenHeight);
        camera->AddComponent<Looker>();
        camera->transform.position = {0.0f, 0.0f, 5.0f};

        // Directional light
        auto sun = scene->CreateObject("SunLight");
        auto l = sun->AddComponent<Light>();
        l->type = LightType::Directional;
        l->direction = glm::normalize(glm::vec3(-1.0f, -2.0f, -1.0f));
        l->color = glm::vec3(1.0f);
        l->intensity = 0.2f;

        // Point light
        auto point = scene->CreateObject("PointLight");
        auto pl = point->AddComponent<Light>();
        pl->type = LightType::Point;
        pl->intensity = 6.0f;
        pl->range = 40.0f;
        point->transform.position = {-3.0f, 3.0f, 2.0f};

        // Example models
        auto model = Model::Load("assets/models/octahedron-sharpe.fbx", scene);
        model->AddComponent<SphereCollider3D>();
        model->AddComponent<RigidBody3D>();
        model->transform.position = {0.0f, 1.0f, 0.0f};

        auto model1 = Model::Load("assets/models/cube.fbx", scene);
        model1->AddComponent<BoxCollider3D>();
        model1->transform.position.y = -3.0f;
        // model1->AddComponent<RigidBody3D>();

        auto model2 = Model::Load("assets/resources/arce.fbx", scene);
        model2->transform.scale = glm::vec3(0.01f);

        auto canvas = scene->CreateObject("Canvas");
        canvas->AddComponent<Canvas>(screenWidth, screenHeight);

        // Example Image
        auto image = scene->CreateObject("Image");
        image->AddComponent<Image>("assets/textures/wall.jpg");
        canvas->AddChild(image);

        // // Example Text
        // auto text = scene->CreateObject("text");
        // auto uiFont = std::make_shared<Font>(
        //     "assets/fonts/roboto.ttf",
        //     32);

        // text->AddComponent<Text>(uiFont, "Hello, World!");
        // canvas->AddChild(text);
    }

    void Engine::Run()
    {
        if (!scene)
            return;

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

    void Engine::Shutdown()
    {
        if (scene)
        {
            scene.reset();
        }

        PhysicsSystem::Get().Clean();

        std::cout << "=============================================\n";
        std::cout << "          ENGINE SHUTDOWN!                     \n";
        std::cout << "=============================================\n\n";
    }
} // namespace Engine

#pragma once
#include <memory>
#include <glm/glm.hpp>

#include "scene.hpp"
#include "window/platform.hpp"
#include "input.hpp"
#include "model.hpp"
#include "systems/physics.hpp"

// Components
#include "components/camera.hpp"
#include "components/looker.hpp"
#include "components/light.hpp"
#include "components/physics/rigidbody3d.hpp"
#include "components/physics/collider/boxcollider3d.hpp"
#include "components/physics/collider/spherecollider3d.hpp"
#include "components/physics/collider/capsulecollider3d.hpp"
#include "components/physics/collider/meshcollider3d.hpp"

namespace Engine
{
    class Engine
    {
    public:
        Engine() = default;
        ~Engine() = default;

        // Initialize the engine with screen size and title
        void Initialize(int width, int height, const char* title);

        // Run the main loop
        void Run();

        // Shutdown engine and cleanup resources
        void Shutdown();

        // Get the active scene
        std::shared_ptr<Scene> GetScene() const { return scene; }

    private:
        std::shared_ptr<Scene> scene;
        int screenWidth = 800;
        int screenHeight = 600;
        const char* appTitle = "Engine App";

        void SetupDefaultScene();
    };
} // namespace Engine

#include "engine/platform/platform.h"
#include <SDL3/SDL.h>
#include <stdexcept>
#include <cassert>
#include "engine/engine.h"
#include <SDL3/SDL_vulkan.h>

using namespace std;

OLIA_API Engine::ECS *ecs = nullptr;
OLIA_API Input *input = nullptr;

namespace Engine
{
    Platform::Platform(PlatformData context)
    {
        this->width = context.width;
        this->height = context.height;
        this->title = context.title;
        this->m_input = new Input();
        this->m_ecs = new ECS();

        ecs = this->m_ecs;
        input = this->m_input;

        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            assert(false && "Failed to initialize SDL.");
        }

        // SDL3 signature: title, width, height, flags (no x, y parameters)
        window = SDL_CreateWindow(
            this->title,
            (int)this->width, (int)this->height,
            SDL_WINDOW_VULKAN | SDL_WINDOW_RESIZABLE);

        this->m_input->Init();
    }

    Platform::~Platform()
    {
        if (this->m_ecs)
            delete this->m_ecs;
        if (m_input)
            delete m_input;

        m_ecs = nullptr;
        m_input = nullptr;

        if (this->window)
            SDL_DestroyWindow(this->window);
        SDL_Quit();
    }

    bool Platform::ShouldClose()
    {
        this->m_input->Update();
        SDL_Event e;
        while (SDL_PollEvent(&e))
        {
            if (e.type == SDL_EVENT_QUIT)
            {
                isRunning = false;
            }
            else if (e.type == SDL_EVENT_WINDOW_RESIZED)
            {
                int newWidth = e.window.data1;
                int newHeight = e.window.data2;

                SDL_Log("Window resized to: %dx%d", newWidth, newHeight);
                width = newWidth;
                height = newHeight;
                // TODO: Update your viewport, projection matrices, or renderer here
                framebufferResize = true;
                break;
            }
            else if (e.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
            {
                // Useful if you are using a Vulkan/OpenGL/Metal graphics context
                // where backing pixel dimensions differ from window logical points.
                int pixelWidth = e.window.data1;
                int pixelHeight = e.window.data2;
                break;
            }
        }
        // Return true if it should STAY open (i.e. isRunning is true)
        return !isRunning;
    }

    SDL_Window *Platform::GetWindow()
    {
        return window;
    }

    void Platform::CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface)
    {
        // SDL3 version of SDL_Vulkan_CreateSurface returns a bool and takes an allocator parameter
        if (!SDL_Vulkan_CreateSurface(window, instance, nullptr, surface))
        {
            throw std::runtime_error(std::string("Failed to create window surface: ") + SDL_GetError());
        }
    }

    VkExtent2D Platform::GetExtent()
    {
        return {static_cast<uint32_t>(width), static_cast<uint32_t>(height)};
    }

} // namespace Engine
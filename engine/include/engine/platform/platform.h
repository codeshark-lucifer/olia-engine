#pragma once
#include <vulkan/vulkan.h>
#include <array>
#include "engine/globals.h"

struct SDL_Window;
namespace Engine
{
#include <cstdint>
    struct PlatformData
    {
        uint32_t width          = 956;
        uint32_t height         = 540;
        const char *title       = "olia - engine";
    };

    class Platform
    {
    public:
        Platform(PlatformData context);
        ~Platform();

        bool ShouldClose();
        SDL_Window *GetWindow();
        void CreateWindowSurface(VkInstance instance, VkSurfaceKHR *surface);
        VkExtent2D GetExtent();
        bool WasResized() { return framebufferResize; }
        void ResetResizedFlag() { framebufferResize = false; }

        std::array<int, 2> GetDim() const
        {
            return {
                static_cast<int>(width),
                static_cast<int>(height)
            };
        }

    private:
        uint32_t width          = 956;
        uint32_t height         = 540;
        const char *title       = "olia - engine";
        bool isRunning          = true;
        bool framebufferResize  = false;
    
        SDL_Window *window      = nullptr;
        Input* m_input          = nullptr;
        ECS* m_ecs              = nullptr;
    };
} // namespace Engine

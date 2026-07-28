#pragma once
#include "./export.h"
#include "./platform/platform.h"
#include "vulkan/swapchain.hpp"
#include "vulkan/device.hpp"
#include "vulkan/shader/pipeline.hpp"

#include <memory>
#include <cstdint>
#include <vector>

struct SDL_Window;

namespace Engine 
{
    struct VKContext
    {
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        EngineDevice *device                        = nullptr;
        std::unique_ptr<EngineSwapChain> swapchain  = nullptr;
        std::unique_ptr<Pipeline> pipeline          = nullptr;
    };

    struct EngineContext
    {
        uint32_t width = 956;
        uint32_t height = 540;
        bool running = true;
        const char *title = "olia - engine";

        Platform *platform = nullptr;
        VKContext vk_context;
    };

    // This must be inside the namespace to match engine.cpp and vulkan.cpp
    extern OLIA_API EngineContext *g_context;

    using SetupFn = void (*)();
    using LoopFn = void (*)();

    extern "C" OLIA_API void Run(SetupFn setup, LoopFn loop);
    void Clean();
    
    void CreatePipelineLayout();
    void CreatePipeline();
    void CreateCommandBuffers();
    void DrawFrame();
    void DrawScene(VkCommandBuffer commandBuffer);
    
    void RecreateSwapChain();
    void RecordCommandBuffer(int imageIndex);
    void FreeCommandBuffer();
} // namespace Engine
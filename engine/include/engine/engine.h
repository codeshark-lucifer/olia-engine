#pragma once
#include "./export.h"
#include "./platform/platform.h"

#include "vulkan/swapchain.hpp"
#include "vulkan/device.hpp"
#include "vulkan/shader/pipeline.hpp"
#include "vulkan/shader/buffer.hpp"
#include "vulkan/shader/descriptor.hpp"
#include "vulkan/shader/resource-manager.hpp"
#include "vulkan/texture.hpp"
#include "vulkan/texture-manager.hpp" // <--- INCLUDE TEXTURE MANAGER

#include <memory>
#include <cstdint>
#include <vector>

struct SDL_Window;

namespace Engine
{
    class TextureManager;

    struct VKContext
    {
        VkPipelineLayout pipelineLayout;
        std::vector<VkCommandBuffer> commandBuffers;
        std::unique_ptr<ResourceManager> resourceManager;
        std::unique_ptr<TextureManager> textureManager; // <--- REPLACED defaultTexture WITH textureManager

        EngineDevice *device                        = nullptr;
        std::unique_ptr<EngineSwapChain> swapchain  = nullptr;
        std::unique_ptr<Pipeline> pipeline          = nullptr;
    };

    struct EngineContext
    {
        uint32_t width = 956;
        uint32_t height = 540;
        bool running = true;
        
        Platform *platform = nullptr;
        VKContext vk_context;
    };

    extern OLIA_API EngineContext *g_context;

    using SetupFn   = void (*)();
    using LoopFn    = void (*)();

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
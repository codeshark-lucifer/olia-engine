#include "engine/engine.h"
#include <glm/gtc/matrix_transform.hpp>
#include <SDL3/SDL.h>
#include <cstdio>
#include <algorithm>

#include "utils/file.h"
#include "engine/platform/platform.h"

#define GLM_FORCE_RADIANS
#define GLM_FORCE_DEPTH_ZERO_TO_ONE
#include <glm/glm.hpp>

#include "components/mesh-renderer.h"

namespace Engine
{
    struct PushConstantData
    {
        alignas(16) glm::mat4 model;
        alignas(16) glm::mat4 proj;
        alignas(16) glm::mat4 view;
    };

    EngineContext *g_context = nullptr;
    void CreateContextInfo(Platform &platform);

    void Run(SetupFn setup, LoopFn loop)
    {
        PlatformData platformData{};

        g_context = new EngineContext();

        try
        {
            g_context->platform = new Platform(platformData);
            Platform *platform = g_context->platform;

            g_context->vk_context.device =
                new EngineDevice(*platform);
            g_context->vk_context.swapchain = std::make_unique<EngineSwapChain>(
                *g_context->vk_context.device,
                platform->GetExtent());

            if (setup)
                setup();
            CreateContextInfo(*platform);

            while (!platform->ShouldClose())
            {
                if (loop)
                    loop();

                DrawFrame();
            }
        }
        catch (...)
        {
            Clean();
            throw;
        }

        Clean();
    }

    void Clean()
    {
        if (!g_context)
            return;
        vkDeviceWaitIdle(g_context->vk_context.device->device());
        vkDestroyPipelineLayout(g_context->vk_context.device->device(), g_context->vk_context.pipelineLayout, nullptr);

        delete g_context->vk_context.device;
        delete static_cast<Platform *>(g_context->platform);

        delete g_context;
        g_context = nullptr;
    }

    void CreateContextInfo(Platform &platform)
    {
        {
            // query all mesh-renderer to setup mesh if it has mesh
            auto entities = ecs->Query<MeshRenderer>();
            for (auto &entity : entities)
            {
                auto &renderer = ecs->Get<MeshRenderer>(entity);
                if (renderer.HasMesh())
                {
                    renderer.Setup(g_context->vk_context.device);
                }
            }
        }

        CreatePipelineLayout();
        RecreateSwapChain();
        CreateCommandBuffers();
    }

    void CreatePipelineLayout()
    {
        VkPushConstantRange pushConstantRange{};
        pushConstantRange.stageFlags = VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT;
        pushConstantRange.offset = 0;
        pushConstantRange.size = sizeof(PushConstantData);

        VkPipelineLayoutCreateInfo pipelineLayoutInfo{};
        pipelineLayoutInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_LAYOUT_CREATE_INFO;
        pipelineLayoutInfo.pNext = nullptr;
        pipelineLayoutInfo.flags = 0;

        // No descriptor sets
        pipelineLayoutInfo.setLayoutCount = 0;
        pipelineLayoutInfo.pSetLayouts = nullptr;

        // No push constants
        pipelineLayoutInfo.pushConstantRangeCount = 1;
        pipelineLayoutInfo.pPushConstantRanges = &pushConstantRange;

        if (vkCreatePipelineLayout(
                g_context->vk_context.device->device(),
                &pipelineLayoutInfo,
                nullptr,
                &g_context->vk_context.pipelineLayout) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create pipeline layout.");
        }
    }

    void CreatePipeline()
    {
        assert(g_context->vk_context.swapchain != nullptr && "Cannot create pipeline before swap chain.");
        assert(g_context->vk_context.pipelineLayout != nullptr && "Cannot create pipeline before layout");

        auto pipelineConfig = Pipeline::DefaultPipelineConfigInfo();
        pipelineConfig.renderPass = g_context->vk_context.swapchain->getRenderPass();
        pipelineConfig.pipelineLayout = g_context->vk_context.pipelineLayout;

        g_context->vk_context.pipeline = std::make_unique<Pipeline>(*g_context->vk_context.device, "main.vert", "main.frag", std::move(pipelineConfig));
    }

    void CreateCommandBuffers()
    {
        g_context->vk_context.commandBuffers.resize(g_context->vk_context.swapchain->imageCount());

        VkCommandBufferAllocateInfo allocInfo{};
        allocInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_ALLOCATE_INFO;
        allocInfo.level = VK_COMMAND_BUFFER_LEVEL_PRIMARY;
        allocInfo.commandPool = g_context->vk_context.device->getCommandPool();
        allocInfo.commandBufferCount = static_cast<uint32_t>(g_context->vk_context.commandBuffers.size());

        if (vkAllocateCommandBuffers(
                g_context->vk_context.device->device(),
                &allocInfo,
                g_context->vk_context.commandBuffers.data()) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to allocate command buffers!");
        }
    }

    void RecordCommandBuffer(int imageIndex)
    {
        VkCommandBufferBeginInfo beginInfo{};
        beginInfo.sType = VK_STRUCTURE_TYPE_COMMAND_BUFFER_BEGIN_INFO;

        if (vkBeginCommandBuffer(g_context->vk_context.commandBuffers[imageIndex], &beginInfo) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to begin recording command buffer!");
        }

        VkRenderPassBeginInfo renderPassInfo{};
        renderPassInfo.sType = VK_STRUCTURE_TYPE_RENDER_PASS_BEGIN_INFO;
        renderPassInfo.renderPass = g_context->vk_context.swapchain->getRenderPass();
        renderPassInfo.framebuffer = g_context->vk_context.swapchain->getFrameBuffer(imageIndex);

        renderPassInfo.renderArea.offset = {0, 0};
        renderPassInfo.renderArea.extent = g_context->vk_context.swapchain->getSwapChainExtent();

        std::array<VkClearValue, 2> clearValues;
        clearValues[0].color = {{0.1f, 0.1f, 0.1f, 1.0f}};
        clearValues[1].depthStencil = {1.0f, 0};

        renderPassInfo.clearValueCount = static_cast<uint32_t>(clearValues.size());
        renderPassInfo.pClearValues = clearValues.data();

        vkCmdBeginRenderPass(
            g_context->vk_context.commandBuffers[imageIndex],
            &renderPassInfo,
            VK_SUBPASS_CONTENTS_INLINE);

        VkViewport viewport{};
        viewport.x = 0.0f;
        viewport.y = 0.0f;
        viewport.width = static_cast<float>(g_context->vk_context.swapchain->getSwapChainExtent().width);
        viewport.height = static_cast<float>(g_context->vk_context.swapchain->getSwapChainExtent().height);
        viewport.minDepth = 0.0f;
        viewport.maxDepth = 1.0f;
        VkRect2D scissor{{0, 0}, g_context->vk_context.swapchain->getSwapChainExtent()};
        vkCmdSetViewport(g_context->vk_context.commandBuffers[imageIndex], 0, 1, &viewport);
        vkCmdSetScissor(g_context->vk_context.commandBuffers[imageIndex], 0, 1, &scissor);

        g_context->vk_context.pipeline->Bind(g_context->vk_context.commandBuffers[imageIndex]);

        DrawScene(g_context->vk_context.commandBuffers[imageIndex]);
        vkCmdEndRenderPass(g_context->vk_context.commandBuffers[imageIndex]);

        if (vkEndCommandBuffer(g_context->vk_context.commandBuffers[imageIndex]) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to record command buffer!");
        }
    }

    void FreeCommandBuffer()
    {
        vkFreeCommandBuffers(
            g_context->vk_context.device->device(),
            g_context->vk_context.device->getCommandPool(),
            static_cast<uint32_t>(g_context->vk_context.commandBuffers.size()),
            g_context->vk_context.commandBuffers.data());

        g_context->vk_context.commandBuffers.clear();
    }

    void RecreateSwapChain()
    {
        auto extent = g_context->platform->GetExtent();
        while (extent.width == 0 || extent.height == 0)
        {
            extent = g_context->platform->GetExtent();
            SDL_Delay(2000);
        }
        vkDeviceWaitIdle(g_context->vk_context.device->device());
        if (g_context->vk_context.swapchain == nullptr)
        {
            g_context->vk_context.swapchain = std::make_unique<EngineSwapChain>(
                *g_context->vk_context.device,
                extent);
        }
        else
        {
            g_context->vk_context.swapchain = std::make_unique<EngineSwapChain>(
                *g_context->vk_context.device,
                extent, std::move(g_context->vk_context.swapchain));
            if (g_context->vk_context.swapchain->imageCount() != g_context->vk_context.commandBuffers.size())
            {
                FreeCommandBuffer();
                CreateCommandBuffers();
            }
        }
        CreatePipeline();
    }

    void DrawFrame()
    {
        uint32_t imageIndex;
        auto result = g_context->vk_context.swapchain->acquireNextImage(&imageIndex);

        if (result == VK_ERROR_OUT_OF_DATE_KHR)
        {
            RecreateSwapChain();
            return;
        }

        if (result != VK_SUCCESS && result != VK_SUBOPTIMAL_KHR)
        {
            throw std::runtime_error("Failed to acquire swap chain image!");
        }

        RecordCommandBuffer(imageIndex);
        result = g_context->vk_context.swapchain->submitCommandBuffers(&g_context->vk_context.commandBuffers[imageIndex], &imageIndex);
        if (result == VK_ERROR_OUT_OF_DATE_KHR || result == VK_SUBOPTIMAL_KHR || g_context->platform->WasResized())
        {
            g_context->platform->ResetResizedFlag();
            RecreateSwapChain();
            return;
        }
        if (result != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to present swap chain image!");
        }
    }

    void DrawScene(VkCommandBuffer commandBuffer)
    {
        Camera camera;
        Transform cameraTransform;
        auto cameras = ecs->Query<Camera>();
        if (!cameras.empty())
        {
            Entity camEntity = cameras[0];
            camera = ecs->Get<Camera>(camEntity);
            cameraTransform = ecs->Get<Transform>(camEntity);
        }
        else
        {
            // fallback camera
            cameraTransform.position = glm::vec3(0.0f, 0.0f, 10.0f);
        }
        camera.aspect = g_context->vk_context.swapchain->extentAspectRatio();

        auto entities = ecs->Query<MeshRenderer>();
        for (auto &entity : entities)
        {
            auto &trans = ecs->Get<Transform>(entity);

            PushConstantData push{};
            push.model = trans.GetMatrix();
            push.proj = camera.GetProjection();
            push.view = camera.GetView(cameraTransform);

            vkCmdPushConstants(
                commandBuffer,
                g_context->vk_context.pipelineLayout,
                VK_SHADER_STAGE_VERTEX_BIT | VK_SHADER_STAGE_FRAGMENT_BIT,
                0, sizeof(PushConstantData), &push);

            // find meshrender and bind & draw
            auto &renderer = ecs->Get<MeshRenderer>(entity);
            renderer.BindBuffers(commandBuffer);
            renderer.Draw(commandBuffer);
        }
    }

}
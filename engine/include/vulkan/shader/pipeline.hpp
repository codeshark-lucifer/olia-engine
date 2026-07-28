#pragma once
#include <vulkan/vulkan.h>
#include <string>
#include "vulkan/device.hpp"

namespace Engine
{
    struct PipelineConfigInfo
    {
        // Default constructor
        PipelineConfigInfo() = default;

        // Delete copies
        PipelineConfigInfo(const PipelineConfigInfo &) = delete;
        PipelineConfigInfo &operator=(const PipelineConfigInfo &) = delete;

        // Allow moves
        PipelineConfigInfo(PipelineConfigInfo &&) noexcept = default;
        PipelineConfigInfo &operator=(PipelineConfigInfo &&) noexcept = default;

        VkPipelineViewportStateCreateInfo viewportInfo;
        VkPipelineInputAssemblyStateCreateInfo inputAssemblyInfo;
        VkPipelineRasterizationStateCreateInfo rasterizationInfo;
        VkPipelineMultisampleStateCreateInfo multisampleInfo;
        VkPipelineColorBlendAttachmentState colorBlendAttachment;
        VkPipelineColorBlendStateCreateInfo colorBlendInfo;
        VkPipelineDepthStencilStateCreateInfo depthStencilInfo;

        std::vector<VkDynamicState> dynamicStateEnables;
        VkPipelineDynamicStateCreateInfo dynamicStateInfo;

        VkPipelineLayout pipelineLayout = nullptr;
        VkRenderPass renderPass = nullptr;
        uint32_t subpass = 0;
    };
    class Pipeline
    {
    public:
        Pipeline(
            EngineDevice &device,
            const char *vertFilePath,
            const char *fragFilePath,
            PipelineConfigInfo config);
        ~Pipeline();

        Pipeline(const Pipeline &) = delete;
        void operator=(const Pipeline &) = delete;
        Pipeline(Pipeline &&) = delete;
        Pipeline &operator=(Pipeline &&) = delete;

        void Bind(VkCommandBuffer commandBuffer);
        static PipelineConfigInfo DefaultPipelineConfigInfo();

    private:
        EngineDevice &device;
        PipelineConfigInfo pipelineConfigInfo;

        VkPipeline graphicsPipeline = VK_NULL_HANDLE;
        VkShaderModule vertShaderModule = VK_NULL_HANDLE;
        VkShaderModule fragShaderModule = VK_NULL_HANDLE;

        void CreateGraphicsPipeline(const char *vertex, const char *fragment);
        void CreateShaderModule(const std::vector<char> &code, VkShaderModule *module);
    };
}
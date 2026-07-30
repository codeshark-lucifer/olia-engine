#include "vulkan/shader/pipeline.hpp"
#include "utils/file.h"
#include <vector>
#include "components/mesh.h"

namespace Engine
{
    Pipeline::Pipeline(
        EngineDevice &device,
        const char *vertFilePath,
        const char *fragFilePath,
        PipelineConfigInfo config)
        : device(device),
          pipelineConfigInfo(std::move(config))
    {
        CreateGraphicsPipeline(vertFilePath, fragFilePath);
    }

    Pipeline::~Pipeline()
    {
        VkDevice vkDevice = device.device();

        if (graphicsPipeline != VK_NULL_HANDLE)
        {
            vkDestroyPipeline(vkDevice, graphicsPipeline, nullptr);
            graphicsPipeline = VK_NULL_HANDLE;
        }

        if (vertShaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(vkDevice, vertShaderModule, nullptr);
            vertShaderModule = VK_NULL_HANDLE;
        }

        if (fragShaderModule != VK_NULL_HANDLE)
        {
            vkDestroyShaderModule(vkDevice, fragShaderModule, nullptr);
            fragShaderModule = VK_NULL_HANDLE;
        }
    }

    void Pipeline::Bind(VkCommandBuffer commandBuffer)
    {
        vkCmdBindPipeline(commandBuffer, VK_PIPELINE_BIND_POINT_GRAPHICS, graphicsPipeline);
    }

    void Pipeline::CreateGraphicsPipeline(const char *vertex_fp, const char *fragment_fp)
    {
        std::filesystem::path vertPath = GetExecutableDir() / "assets" / "shaders" / "compiled" / (std::string(vertex_fp) + ".spv");
        std::filesystem::path fragPath = GetExecutableDir() / "assets" / "shaders" / "compiled" / (std::string(fragment_fp) + ".spv");

        std::vector<char> vertexShaderBytes = ReadFile(vertPath.string().c_str());
        std::vector<char> fragmentShaderBytes = ReadFile(fragPath.string().c_str());

        if (vertexShaderBytes.empty())
        {
            throw std::runtime_error("Vertex shader not found.");
        }

        if (fragmentShaderBytes.empty())
        {
            throw std::runtime_error("Fragment shader not found.");
        }

        CreateShaderModule(vertexShaderBytes, &vertShaderModule);
        CreateShaderModule(fragmentShaderBytes, &fragShaderModule);

        VkPipelineShaderStageCreateInfo shaderStages[2]{};

        // Vertex Shader
        shaderStages[0].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[0].stage = VK_SHADER_STAGE_VERTEX_BIT;
        shaderStages[0].module = vertShaderModule;
        shaderStages[0].pName = "main";
        shaderStages[0].flags = 0;
        shaderStages[0].pNext = nullptr;
        shaderStages[0].pSpecializationInfo = nullptr;

        // Fragment Shader
        shaderStages[1].sType = VK_STRUCTURE_TYPE_PIPELINE_SHADER_STAGE_CREATE_INFO;
        shaderStages[1].stage = VK_SHADER_STAGE_FRAGMENT_BIT;
        shaderStages[1].module = fragShaderModule;
        shaderStages[1].pName = "main";
        shaderStages[1].flags = 0;
        shaderStages[1].pNext = nullptr;
        shaderStages[1].pSpecializationInfo = nullptr;

        auto bindingDescriptions = Mesh::GetBindingDescriptions();
        auto attributeDescritpions = Mesh::GetAttributeDescriptions();

        VkPipelineVertexInputStateCreateInfo vertexInputInfo{};
        vertexInputInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VERTEX_INPUT_STATE_CREATE_INFO;
        vertexInputInfo.pNext = nullptr;
        vertexInputInfo.flags = 0;
        vertexInputInfo.vertexBindingDescriptionCount = static_cast<uint32_t>(bindingDescriptions.size());
        vertexInputInfo.pVertexBindingDescriptions = bindingDescriptions.data();
        vertexInputInfo.vertexAttributeDescriptionCount = static_cast<uint32_t>(attributeDescritpions.size());
        vertexInputInfo.pVertexAttributeDescriptions = attributeDescritpions.data();

        VkGraphicsPipelineCreateInfo pipelineInfo{};
        pipelineInfo.sType = VK_STRUCTURE_TYPE_GRAPHICS_PIPELINE_CREATE_INFO;
        pipelineInfo.pNext = nullptr;
        pipelineInfo.flags = 0;
        pipelineInfo.stageCount = 2;
        pipelineInfo.pStages = shaderStages;
        pipelineInfo.pVertexInputState = &vertexInputInfo;
        pipelineInfo.pInputAssemblyState = &pipelineConfigInfo.inputAssemblyInfo;
        pipelineInfo.pViewportState = &pipelineConfigInfo.viewportInfo;
        pipelineInfo.pRasterizationState = &pipelineConfigInfo.rasterizationInfo;
        pipelineInfo.pMultisampleState = &pipelineConfigInfo.multisampleInfo;
        pipelineInfo.pDepthStencilState = &pipelineConfigInfo.depthStencilInfo;
        pipelineInfo.pColorBlendState = &pipelineConfigInfo.colorBlendInfo;
        pipelineInfo.pDynamicState = &pipelineConfigInfo.dynamicStateInfo;

        pipelineInfo.layout = pipelineConfigInfo.pipelineLayout;
        pipelineInfo.renderPass = pipelineConfigInfo.renderPass;
        pipelineInfo.subpass = pipelineConfigInfo.subpass;

        pipelineInfo.basePipelineHandle = VK_NULL_HANDLE;
        pipelineInfo.basePipelineIndex = -1;

        if (vkCreateGraphicsPipelines(
                device.device(),
                VK_NULL_HANDLE,
                1,
                &pipelineInfo,
                nullptr,
                &graphicsPipeline) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create graphics pipeline.");
        }
    }

    void Pipeline::CreateShaderModule(const std::vector<char> &code, VkShaderModule *module)
    {
        if (code.empty() || (code.size() % 4) != 0)
        {
            throw std::runtime_error("Invalid SPIR-V shader.");
        }
        VkShaderModuleCreateInfo createInfo{};
        createInfo.sType = VK_STRUCTURE_TYPE_SHADER_MODULE_CREATE_INFO;
        createInfo.codeSize = code.size();
        createInfo.pCode = reinterpret_cast<const uint32_t *>(code.data());

        if (vkCreateShaderModule(device.device(), &createInfo, nullptr, module) != VK_SUCCESS)
        {
            throw std::runtime_error("Failed to create shader module.");
        }
    }

    PipelineConfigInfo Pipeline::DefaultPipelineConfigInfo()
    {
        PipelineConfigInfo configInfo{};
        configInfo.inputAssemblyInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_INPUT_ASSEMBLY_STATE_CREATE_INFO;
        configInfo.inputAssemblyInfo.topology = VK_PRIMITIVE_TOPOLOGY_TRIANGLE_LIST;
        configInfo.inputAssemblyInfo.primitiveRestartEnable = VK_FALSE;
        configInfo.inputAssemblyInfo.pNext = nullptr;
        configInfo.inputAssemblyInfo.flags = 0;

        configInfo.viewportInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_VIEWPORT_STATE_CREATE_INFO;
        configInfo.viewportInfo.pNext = nullptr;
        configInfo.viewportInfo.flags = 0;
        configInfo.viewportInfo.viewportCount = 1;
        configInfo.viewportInfo.pViewports = nullptr;
        configInfo.viewportInfo.scissorCount = 1;
        configInfo.viewportInfo.pScissors = nullptr;

        configInfo.rasterizationInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_RASTERIZATION_STATE_CREATE_INFO;
        configInfo.rasterizationInfo.depthClampEnable = VK_FALSE;
        configInfo.rasterizationInfo.rasterizerDiscardEnable = VK_FALSE;
        configInfo.rasterizationInfo.polygonMode = VK_POLYGON_MODE_FILL;
        configInfo.rasterizationInfo.lineWidth = 1.0f;
        configInfo.rasterizationInfo.cullMode = VK_CULL_MODE_BACK_BIT;
        configInfo.rasterizationInfo.frontFace = VK_FRONT_FACE_COUNTER_CLOCKWISE;
        configInfo.rasterizationInfo.depthBiasEnable = VK_FALSE;
        configInfo.rasterizationInfo.depthBiasConstantFactor = 0.0f; // Optional
        configInfo.rasterizationInfo.depthBiasClamp = 0.0f;          // Optional
        configInfo.rasterizationInfo.depthBiasSlopeFactor = 0.0f;    // Optional
        configInfo.rasterizationInfo.pNext = nullptr;
        configInfo.rasterizationInfo.flags = 0;

        configInfo.multisampleInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_MULTISAMPLE_STATE_CREATE_INFO;
        configInfo.multisampleInfo.sampleShadingEnable = VK_FALSE;
        configInfo.multisampleInfo.rasterizationSamples = VK_SAMPLE_COUNT_1_BIT;
        configInfo.multisampleInfo.minSampleShading = 1.0f;          // Optional
        configInfo.multisampleInfo.pSampleMask = nullptr;            // Optional
        configInfo.multisampleInfo.alphaToCoverageEnable = VK_FALSE; // Optional
        configInfo.multisampleInfo.alphaToOneEnable = VK_FALSE;      // Optional
        configInfo.multisampleInfo.pNext = nullptr;
        configInfo.multisampleInfo.flags = 0;

        configInfo.colorBlendAttachment.colorWriteMask =
            VK_COLOR_COMPONENT_R_BIT | VK_COLOR_COMPONENT_G_BIT | VK_COLOR_COMPONENT_B_BIT |
            VK_COLOR_COMPONENT_A_BIT;
        configInfo.colorBlendAttachment.blendEnable = VK_TRUE;
        configInfo.colorBlendAttachment.srcColorBlendFactor = VK_BLEND_FACTOR_SRC_ALPHA;
        configInfo.colorBlendAttachment.dstColorBlendFactor = VK_BLEND_FACTOR_ONE_MINUS_SRC_ALPHA;
        configInfo.colorBlendAttachment.colorBlendOp = VK_BLEND_OP_ADD;
        configInfo.colorBlendAttachment.srcAlphaBlendFactor = VK_BLEND_FACTOR_ONE;
        configInfo.colorBlendAttachment.dstAlphaBlendFactor = VK_BLEND_FACTOR_ZERO;
        configInfo.colorBlendAttachment.alphaBlendOp = VK_BLEND_OP_ADD;

        configInfo.colorBlendInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_COLOR_BLEND_STATE_CREATE_INFO;
        configInfo.colorBlendInfo.logicOpEnable = VK_FALSE;
        configInfo.colorBlendInfo.logicOp = VK_LOGIC_OP_COPY; // Optional
        configInfo.colorBlendInfo.attachmentCount = 1;
        configInfo.colorBlendInfo.pAttachments = &configInfo.colorBlendAttachment;
        configInfo.colorBlendInfo.blendConstants[0] = 0.0f; // Optional
        configInfo.colorBlendInfo.blendConstants[1] = 0.0f; // Optional
        configInfo.colorBlendInfo.blendConstants[2] = 0.0f; // Optional
        configInfo.colorBlendInfo.blendConstants[3] = 0.0f; // Optional
        configInfo.colorBlendInfo.pNext = nullptr;
        configInfo.colorBlendInfo.flags = 0;

        configInfo.depthStencilInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DEPTH_STENCIL_STATE_CREATE_INFO;
        configInfo.depthStencilInfo.depthTestEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthWriteEnable = VK_TRUE;
        configInfo.depthStencilInfo.depthCompareOp = VK_COMPARE_OP_LESS;
        configInfo.depthStencilInfo.depthBoundsTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.minDepthBounds = 0.0f; // Optional
        configInfo.depthStencilInfo.maxDepthBounds = 1.0f; // Optional
        configInfo.depthStencilInfo.stencilTestEnable = VK_FALSE;
        configInfo.depthStencilInfo.front = {}; // Optional
        configInfo.depthStencilInfo.back = {};  // Optional
        configInfo.depthStencilInfo.pNext = nullptr;
        configInfo.depthStencilInfo.flags = 0;

        configInfo.dynamicStateEnables = {VK_DYNAMIC_STATE_VIEWPORT, VK_DYNAMIC_STATE_SCISSOR};
        configInfo.dynamicStateInfo.sType = VK_STRUCTURE_TYPE_PIPELINE_DYNAMIC_STATE_CREATE_INFO;
        configInfo.dynamicStateInfo.pDynamicStates = configInfo.dynamicStateEnables.data();
        configInfo.dynamicStateInfo.dynamicStateCount = static_cast<uint32_t>(configInfo.dynamicStateEnables.size());
        configInfo.dynamicStateInfo.flags = 0;

        return configInfo;
    }
} // namespace Engine

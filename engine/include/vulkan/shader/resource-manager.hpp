#pragma once

#include "vulkan/device.hpp"
#include "vulkan/shader/buffer.hpp"
#include "vulkan/shader/descriptor.hpp"
#include <unordered_map>
#include <memory>
#include <string>

namespace Engine
{

    enum class ResourceType
    {
        Uniform,
        Storage,
        // future: CombinedImageSampler, etc.
    };

    class ResourceManager
    {
    public:
        ResourceManager(EngineDevice &device, uint32_t maxFramesInFlight);
        ~ResourceManager();

        // Register a buffer resource.
        // - name: unique identifier
        // - type: uniform or storage
        // - size: total buffer size in bytes
        // - binding: descriptor set binding index
        // - stageFlags: shader stages that use it
        void RegisterBuffer(const std::string &name,
                            ResourceType type,
                            VkDeviceSize size,
                            uint32_t binding,
                            VkShaderStageFlags stageFlags);

        // Update buffer data. Copies `data` (size bytes) into the buffer at `offset`.
        void UpdateBuffer(const std::string &name,
                          const void *data,
                          VkDeviceSize size,
                          VkDeviceSize offset = 0);

        // Bind all registered resources for the current frame.
        // Must be called inside a render pass after pipeline bind.
        void Bind(VkCommandBuffer cmdBuf, VkPipelineLayout pipelineLayout, uint32_t frameIndex);

        // Build descriptor sets (if not already built). Called automatically on first Bind,
        // but you can call it earlier (e.g., before creating pipeline layout).
        void BuildDescriptorSets();

        // Check if descriptor sets have been built.
        bool IsBuilt() const { return built; }

        // Get the descriptor set layout (valid only after BuildDescriptorSets()).
        VkDescriptorSetLayout GetDescriptorSetLayout() const;

        // Clean up Vulkan objects (call before destroying device).
        void Shutdown();

    private:
        struct Resource
        {
            std::unique_ptr<EngineBuffer> buffer;
            uint32_t binding;
            VkDescriptorBufferInfo descriptorInfo;
        };

        EngineDevice &device;
        uint32_t maxFrames;

        std::unordered_map<std::string, Resource> resources;

        // Descriptor set layout builder (collects bindings)
        EngineDescriptorSetLayout::Builder layoutBuilder;
        std::unique_ptr<EngineDescriptorSetLayout> descriptorSetLayout;
        std::unique_ptr<EngineDescriptorPool> descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;

        bool built = false;

        void BuildDescriptorSetsInternal();
    };

} // namespace Engine
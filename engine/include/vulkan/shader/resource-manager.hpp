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
        CombinedImageSampler
    };

    class ResourceManager
    {
    public:
        ResourceManager(EngineDevice &device, uint32_t maxFramesInFlight);
        ~ResourceManager();

        void RegisterBuffer(const std::string &name,
                            ResourceType type,
                            VkDeviceSize size,
                            uint32_t binding,
                            VkShaderStageFlags stageFlags);

        void RegisterTexture(const std::string &name,
                             VkImageView imageView,
                             VkSampler sampler,
                             uint32_t binding,
                             VkShaderStageFlags stageFlags);

        void UpdateBuffer(const std::string &name,
                          const void *data,
                          VkDeviceSize size,
                          VkDeviceSize offset = 0);
        void UpdateTexture(const std::string &name, VkImageView imageView, VkSampler sampler);

        void Bind(VkCommandBuffer cmdBuf, VkPipelineLayout pipelineLayout, uint32_t frameIndex);
        void BuildDescriptorSets();
        bool IsBuilt() const { return built; }
        VkDescriptorSetLayout GetDescriptorSetLayout() const;
        void Shutdown();

    private:
        struct Resource
        {
            ResourceType type;
            std::unique_ptr<EngineBuffer> buffer;
            VkDescriptorImageInfo imageInfo;
            uint32_t binding;
            VkDescriptorBufferInfo descriptorInfo;
        };

        EngineDevice &device;
        uint32_t maxFrames;

        std::unordered_map<std::string, Resource> resources;

        EngineDescriptorSetLayout::Builder layoutBuilder;
        std::unique_ptr<EngineDescriptorSetLayout> descriptorSetLayout;
        std::unique_ptr<EngineDescriptorPool> descriptorPool;
        std::vector<VkDescriptorSet> descriptorSets;

        bool built = false;

        void BuildDescriptorSetsInternal();
    };

} // namespace Engine
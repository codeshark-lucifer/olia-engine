#pragma once

#include "vulkan/device.hpp"

// std
#include <memory>
#include <unordered_map>
#include <vector>

namespace Engine
{

    class EngineDescriptorSetLayout
    {
    public:
        class Builder
        {
        public:
            Builder(EngineDevice &device) : device{device} {}

            Builder &addBinding(
                uint32_t binding,
                VkDescriptorType descriptorType,
                VkShaderStageFlags stageFlags,
                uint32_t count = 1);
            std::unique_ptr<EngineDescriptorSetLayout> build() const;

        private:
            EngineDevice &device;
            std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings{};
        };

        EngineDescriptorSetLayout(
            EngineDevice &EngineDevice, std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings);
        ~EngineDescriptorSetLayout();
        EngineDescriptorSetLayout(const EngineDescriptorSetLayout &) = delete;
        EngineDescriptorSetLayout &operator=(const EngineDescriptorSetLayout &) = delete;

        VkDescriptorSetLayout getDescriptorSetLayout() const { return descriptorSetLayout; }

    private:
        EngineDevice &device;
        VkDescriptorSetLayout descriptorSetLayout;
        std::unordered_map<uint32_t, VkDescriptorSetLayoutBinding> bindings;

        friend class EngineDescriptorWriter;
    };

    class EngineDescriptorPool
    {
    public:
        class Builder
        {
        public:
            Builder(EngineDevice &device) : device{device} {}

            Builder &addPoolSize(VkDescriptorType descriptorType, uint32_t count);
            Builder &setPoolFlags(VkDescriptorPoolCreateFlags flags);
            Builder &setMaxSets(uint32_t count);
            std::unique_ptr<EngineDescriptorPool> build() const;

        private:
            EngineDevice &device;
            std::vector<VkDescriptorPoolSize> poolSizes{};
            uint32_t maxSets = 1000;
            VkDescriptorPoolCreateFlags poolFlags = 0;
        };

        EngineDescriptorPool(
            EngineDevice &device,
            uint32_t maxSets,
            VkDescriptorPoolCreateFlags poolFlags,
            const std::vector<VkDescriptorPoolSize> &poolSizes);
        ~EngineDescriptorPool();
        EngineDescriptorPool(const EngineDescriptorPool &) = delete;
        EngineDescriptorPool &operator=(const EngineDescriptorPool &) = delete;
        VkDescriptorPool getDescriptorPool() const { return descriptorPool; }
        bool allocateDescriptor(
            const VkDescriptorSetLayout descriptorSetLayout, VkDescriptorSet &descriptor) const;

        void freeDescriptors(std::vector<VkDescriptorSet> &descriptors) const;

        void resetPool();

    private:
        EngineDevice &device;
        VkDescriptorPool descriptorPool;

        friend class EngineDescriptorWriter;
    };

    class EngineDescriptorWriter
    {
    public:
        EngineDescriptorWriter(EngineDescriptorSetLayout &setLayout, EngineDescriptorPool &pool);

        EngineDescriptorWriter &writeBuffer(uint32_t binding, VkDescriptorBufferInfo *bufferInfo);
        EngineDescriptorWriter &writeImage(uint32_t binding, VkDescriptorImageInfo *imageInfo);

        bool build(VkDescriptorSet &set);
        void overwrite(VkDescriptorSet &set);

    private:
        EngineDescriptorSetLayout &setLayout;
        EngineDescriptorPool &pool;
        std::vector<VkWriteDescriptorSet> writes;
    };

} // namespace Engine
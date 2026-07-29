#include "vulkan/shader/resource-manager.hpp"
#include <stdexcept>

namespace Engine
{

    ResourceManager::ResourceManager(EngineDevice &device, uint32_t maxFramesInFlight)
        : device(device), maxFrames(maxFramesInFlight), layoutBuilder(device)
    {
    }

    ResourceManager::~ResourceManager()
    {
        Shutdown();
    }

    void ResourceManager::RegisterBuffer(const std::string &name,
                                         ResourceType type,
                                         VkDeviceSize size,
                                         uint32_t binding,
                                         VkShaderStageFlags stageFlags)
    {
        if (resources.find(name) != resources.end())
        {
            throw std::runtime_error("Resource already registered: " + name);
        }

        // Determine usage flags and descriptor type.
        VkBufferUsageFlags usage = 0;
        VkDescriptorType descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        if (type == ResourceType::Uniform)
        {
            usage = VK_BUFFER_USAGE_UNIFORM_BUFFER_BIT;
            descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
        }
        else if (type == ResourceType::Storage)
        {
            usage = VK_BUFFER_USAGE_STORAGE_BUFFER_BIT;
            descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
        }
        else
        {
            throw std::runtime_error("Unsupported resource type");
        }

        // Create the buffer (persistently mapped for fast updates).
        auto buffer = std::make_unique<EngineBuffer>(
            device,
            size, // instance size = total size (single instance)
            1,    // instance count = 1
            usage,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT,
            1);
        buffer->map(); // keep mapped

        // Store resource.
        Resource res;
        res.buffer = std::move(buffer);
        res.binding = binding;
        res.descriptorInfo = res.buffer->descriptorInfo();
        resources[name] = std::move(res);

        // Add binding to layout builder.
        layoutBuilder.addBinding(binding, descType, stageFlags);
    }

    void ResourceManager::BuildDescriptorSetsInternal()
    {
        if (built)
            return;

        // Build layout from all registered bindings.
        descriptorSetLayout = layoutBuilder.build();

        // Create descriptor pool with enough descriptors for all resources per frame.
        EngineDescriptorPool::Builder poolBuilder(device);
        for (const auto &[name, res] : resources)
        {
            VkDescriptorType descType = (res.buffer->getUsageFlags() & VK_BUFFER_USAGE_STORAGE_BUFFER_BIT)
                                            ? VK_DESCRIPTOR_TYPE_STORAGE_BUFFER
                                            : VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;
            poolBuilder.addPoolSize(descType, maxFrames);
        }
        poolBuilder.setMaxSets(maxFrames);
        descriptorPool = poolBuilder.build();

        // Allocate one descriptor set per frame, each containing all resources.
        descriptorSets.resize(maxFrames);
        for (uint32_t i = 0; i < maxFrames; ++i)
        {
            EngineDescriptorWriter writer(*descriptorSetLayout, *descriptorPool);
            for (const auto &[name, res] : resources)
            {
                VkDescriptorBufferInfo info = res.buffer->descriptorInfo();
                writer.writeBuffer(res.binding, &info);
            }
            if (!writer.build(descriptorSets[i]))
            {
                throw std::runtime_error("Failed to allocate descriptor set");
            }
        }

        built = true;
    }

    void ResourceManager::BuildDescriptorSets()
    {
        BuildDescriptorSetsInternal();
    }

    VkDescriptorSetLayout ResourceManager::GetDescriptorSetLayout() const
    {
        if (!descriptorSetLayout)
        {
            throw std::runtime_error("Descriptor set layout not built yet. Call BuildDescriptorSets() first.");
        }
        return descriptorSetLayout->getDescriptorSetLayout();
    }

    void ResourceManager::UpdateBuffer(const std::string &name,
                                       const void *data,
                                       VkDeviceSize size,
                                       VkDeviceSize offset)
    {
        auto it = resources.find(name);
        if (it == resources.end())
        {
            throw std::runtime_error("Resource not found: " + name);
        }
        auto &res = it->second;
        res.buffer->writeToBuffer(const_cast<void *>(data), size, offset);
        // Since we used HOST_COHERENT, flush is optional but safe.
        res.buffer->flush(size, offset);
    }

    void ResourceManager::Bind(VkCommandBuffer cmdBuf,
                               VkPipelineLayout pipelineLayout,
                               uint32_t frameIndex)
    {
        if (!built)
            BuildDescriptorSetsInternal();
        if (frameIndex >= descriptorSets.size())
        {
            throw std::runtime_error("frameIndex out of range");
        }
        vkCmdBindDescriptorSets(cmdBuf,
                                VK_PIPELINE_BIND_POINT_GRAPHICS,
                                pipelineLayout,
                                0, // first set
                                1,
                                &descriptorSets[frameIndex],
                                0,
                                nullptr);
    }

    void ResourceManager::Shutdown()
    {
        // Unmap all buffers.
        for (auto &[name, res] : resources)
        {
            if (res.buffer)
            {
                res.buffer->unmap();
            }
        }
        resources.clear();
        descriptorSets.clear();
        descriptorPool.reset();
        descriptorSetLayout.reset();
        built = false;
    }

} // namespace Engine
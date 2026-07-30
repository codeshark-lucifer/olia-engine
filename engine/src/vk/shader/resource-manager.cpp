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

        auto buffer = std::make_unique<EngineBuffer>(
            device, size, 1, usage,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT | VK_MEMORY_PROPERTY_HOST_COHERENT_BIT, 1);
        buffer->map();

        Resource res;
        res.type = type;
        res.buffer = std::move(buffer);
        res.binding = binding;
        resources[name] = std::move(res);

        layoutBuilder.addBinding(binding, descType, stageFlags);
    }

    void ResourceManager::RegisterTexture(const std::string &name,
                                          VkImageView imageView,
                                          VkSampler sampler,
                                          uint32_t binding,
                                          VkShaderStageFlags stageFlags)
    {
        if (resources.find(name) != resources.end())
        {
            throw std::runtime_error("Resource already registered: " + name);
        }

        Resource res;
        res.type = ResourceType::CombinedImageSampler;
        res.binding = binding;
        res.imageInfo.imageLayout = VK_IMAGE_LAYOUT_SHADER_READ_ONLY_OPTIMAL;
        res.imageInfo.imageView = imageView;
        res.imageInfo.sampler = sampler;

        resources[name] = std::move(res);

        layoutBuilder.addBinding(binding, VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER, stageFlags);
    }

    void ResourceManager::BuildDescriptorSetsInternal()
    {
        if (built)
            return;

        descriptorSetLayout = layoutBuilder.build();

        EngineDescriptorPool::Builder poolBuilder(device);
        for (const auto &[name, res] : resources)
        {
            VkDescriptorType descType;
            if (res.type == ResourceType::Storage)
                descType = VK_DESCRIPTOR_TYPE_STORAGE_BUFFER;
            else if (res.type == ResourceType::CombinedImageSampler)
                descType = VK_DESCRIPTOR_TYPE_COMBINED_IMAGE_SAMPLER;
            else
                descType = VK_DESCRIPTOR_TYPE_UNIFORM_BUFFER;

            poolBuilder.addPoolSize(descType, maxFrames);
        }
        poolBuilder.setMaxSets(maxFrames);
        descriptorPool = poolBuilder.build();

        descriptorSets.resize(maxFrames);
        for (uint32_t i = 0; i < maxFrames; ++i)
        {
            EngineDescriptorWriter writer(*descriptorSetLayout, *descriptorPool);

            std::vector<VkDescriptorBufferInfo> bufferInfos;
            std::vector<VkDescriptorImageInfo> imageInfos;
            bufferInfos.reserve(resources.size());
            imageInfos.reserve(resources.size());

            for (const auto &[name, res] : resources)
            {
                if (res.type == ResourceType::CombinedImageSampler)
                {
                    imageInfos.push_back(res.imageInfo);
                    writer.writeImage(res.binding, &imageInfos.back());
                }
                else
                {
                    bufferInfos.push_back(res.buffer->descriptorInfo());
                    writer.writeBuffer(res.binding, &bufferInfos.back());
                }
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
                                0,
                                1,
                                &descriptorSets[frameIndex],
                                0,
                                nullptr);
    }

    void ResourceManager::Shutdown()
    {
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
void ResourceManager::UpdateTexture(const std::string &name, VkImageView imageView, VkSampler sampler)
{
    auto it = resources.find(name);
    if (it == resources.end())
    {
        throw std::runtime_error("Resource not found: " + name);
    }

    auto &res = it->second;
    res.imageInfo.imageView = imageView;
    res.imageInfo.sampler = sampler;

    if (built)
    {
        for (uint32_t i = 0; i < maxFrames; ++i)
        {
            EngineDescriptorWriter writer(*descriptorSetLayout, *descriptorPool);
            writer.writeImage(res.binding, &res.imageInfo);
            writer.overwrite(descriptorSets[i]);
        }
    }
}
} // namespace Engine
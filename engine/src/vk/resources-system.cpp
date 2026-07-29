#include "vulkan/resources-system.hpp"

namespace Engine
{
    template <typename T>
    void ResourceSystem::Register(
        uint32_t binding,
        uint32_t count,
        VkBufferUsageFlags usage,
        VkDescriptorType descriptorType)
    {
        GPUResource resource;

        resource.binding = binding;
        resource.count = count;
        resource.stride = sizeof(T);
        resource.usage = usage;
        resource.descriptorType = descriptorType;

        resource.buffer = std::make_unique<EngineBuffer>(
            device,
            sizeof(T),
            count,
            usage,
            VK_MEMORY_PROPERTY_HOST_VISIBLE_BIT |
                VK_MEMORY_PROPERTY_HOST_COHERENT_BIT);

        resource.buffer->map();

        resources.emplace(typeid(T), std::move(resource));
    }

    template <typename T>
    GPUResource &ResourceSystem::Get()
    {
        return resources.at(typeid(T));
    }

    template <typename T>
    void ResourceSystem::Upload(const T &value)
    {
        auto &resource = Get<T>();

        resource.buffer->writeToBuffer(
            &value,
            sizeof(T));

        resource.buffer->flush();
    }

    template <typename T>
    void ResourceSystem::UploadArray(
        const std::vector<T> &values)
    {
        auto &resource = Get<T>();

        resource.buffer->writeToBuffer(
            values.data(),
            values.size() * sizeof(T));

        resource.buffer->flush();
    }
} // namespace Engine

#pragma once

#include <memory>
#include <unordered_map>
#include <typeindex>

#include "shader/buffer.hpp"

namespace Engine
{
    struct GPUResource
    {
        std::unique_ptr<EngineBuffer> buffer;

        VkDescriptorType descriptorType{};
        VkBufferUsageFlags usage{};

        uint32_t count = 1;
        uint32_t binding = 0;
        size_t stride = 0;
    };

    class EngineDevice;

    class ResourceSystem
    {
    public:
        explicit ResourceSystem(EngineDevice& device);

        template<typename T>
        void Register(
            uint32_t binding,
            uint32_t count,
            VkBufferUsageFlags usage,
            VkDescriptorType descriptorType);

        template<typename T>
        GPUResource& Get();

        template<typename T>
        void Upload(const T& value);

        template<typename T>
        void UploadArray(const std::vector<T>& values);

    private:
        EngineDevice& device;

        std::unordered_map<
            std::type_index,
            GPUResource
        > resources;
    };
}
#pragma once

#include <vulkan/vulkan.h>
#include "vk_mem_alloc.h"

namespace Engine 
{
    class MemoryAllocator 
    {
    public:
        MemoryAllocator() = default;
        ~MemoryAllocator() = default;

        // Non-copyable
        MemoryAllocator(const MemoryAllocator&) = delete;
        MemoryAllocator& operator=(const MemoryAllocator&) = delete;

        void Init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t vulkanApiVersion = VK_API_VERSION_1_2);
        void Cleanup();

        [[nodiscard]] VmaAllocator Get() const { return m_Allocator; }

        // Helper wrappers for buffer and image creation
        VkResult CreateBuffer(
            const VkBufferCreateInfo* pBufferCreateInfo,
            const VmaAllocationCreateInfo* pAllocationCreateInfo,
            VkBuffer* pBuffer,
            VmaAllocation* pAllocation,
            VmaAllocationInfo* pAllocationInfo = nullptr
        );

        void DestroyBuffer(VkBuffer buffer, VmaAllocation allocation);

    private:
        VmaAllocator m_Allocator = VK_NULL_HANDLE;
    };
} // namespace Engine
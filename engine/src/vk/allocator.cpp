#define VMA_IMPLEMENTATION
#include "vulkan/allocator.h"
#include <stdexcept>

namespace Engine 
{
    void MemoryAllocator::Init(VkInstance instance, VkPhysicalDevice physicalDevice, VkDevice device, uint32_t vulkanApiVersion)
    {
        VmaAllocatorCreateInfo allocatorInfo = {};
        allocatorInfo.flags = VMA_ALLOCATOR_CREATE_EXT_MEMORY_BUDGET_BIT;
        allocatorInfo.physicalDevice = physicalDevice;
        allocatorInfo.device = device;
        allocatorInfo.instance = instance;
        allocatorInfo.vulkanApiVersion = vulkanApiVersion;

        VkResult result = vmaCreateAllocator(&allocatorInfo, &m_Allocator);
        if (result != VK_SUCCESS) 
        {
            throw std::runtime_error("Failed to create VMA allocator!");
        }
    }

    void MemoryAllocator::Cleanup()
    {
        if (m_Allocator != VK_NULL_HANDLE) 
        {
            vmaDestroyAllocator(m_Allocator);
            m_Allocator = VK_NULL_HANDLE;
        }
    }

    VkResult MemoryAllocator::CreateBuffer(
        const VkBufferCreateInfo* pBufferCreateInfo,
        const VmaAllocationCreateInfo* pAllocationCreateInfo,
        VkBuffer* pBuffer,
        VmaAllocation* pAllocation,
        VmaAllocationInfo* pAllocationInfo)
    {
        return vmaCreateBuffer(m_Allocator, pBufferCreateInfo, pAllocationCreateInfo, pBuffer, pAllocation, pAllocationInfo);
    }

    void MemoryAllocator::DestroyBuffer(VkBuffer buffer, VmaAllocation allocation)
    {
        if (buffer != VK_NULL_HANDLE && allocation != VK_NULL_HANDLE) 
        {
            vmaDestroyBuffer(m_Allocator, buffer, allocation);
        }
    }
} // namespace Engine
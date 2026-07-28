#pragma once
#include "./mesh.h"
#include <memory>
#include "vulkan/device.hpp"

class MeshRenderer
{
public:
    MeshRenderer() = default;

    // MeshRenderer(const MeshRenderer &) = delete;
    // MeshRenderer &operator=(const MeshRenderer &) = delete;

    ~MeshRenderer()
    {
        vkDestroyBuffer(device->device(), vertexBuffer, nullptr);
        vkFreeMemory(device->device(), vertexBufferMemory, nullptr);

        // Added index buffer cleanup
        vkDestroyBuffer(device->device(), indexBuffer, nullptr);
        vkFreeMemory(device->device(), indexBufferMemory, nullptr);
    }

    void SetMesh(std::unique_ptr<Mesh> meshPtr)
    {
        mesh = std::move(meshPtr);
    }

    void Setup(Engine::EngineDevice *devicePtr)
    {
        device = devicePtr;

        CreateVertexBuffers(mesh->vertices);
        CreateIndicesBuffers(mesh->indices);
    }

    void BindBuffers(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);
    bool HasMesh() { return mesh != nullptr; }
    std::unique_ptr<Mesh> GetMesh() { return std::move(mesh); }

private:
    void CreateVertexBuffers(const std::vector<Engine::Vertex> &vertices);
    void CreateIndicesBuffers(const std::vector<uint16_t> &indices);

    std::unique_ptr<Mesh> mesh = nullptr;
    Engine::EngineDevice *device = nullptr;

    VkBuffer vertexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory vertexBufferMemory = VK_NULL_HANDLE;

    // Added index buffer handles
    VkBuffer indexBuffer = VK_NULL_HANDLE;
    VkDeviceMemory indexBufferMemory = VK_NULL_HANDLE;

    uint32_t vertexCount = 0;
    uint32_t indicesCount = 0;
};
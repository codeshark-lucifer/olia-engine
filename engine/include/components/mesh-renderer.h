#pragma once

#include "./mesh.h"
#include <memory>
#include "vulkan/device.hpp"
#include "vulkan/shader/buffer.hpp" 
#include "engine/export.h"

class OLIA_API MeshRenderer
{
public:
    MeshRenderer() = default;

    // No need for custom destructor – unique_ptrs will clean up automatically
    ~MeshRenderer() = default;

    void SetMesh(std::unique_ptr<Mesh> meshPtr) { mesh = std::move(meshPtr); }
    void Setup(Engine::EngineDevice* devicePtr);

    void BindBuffers(VkCommandBuffer commandBuffer);
    void Draw(VkCommandBuffer commandBuffer);

    bool HasMesh() const { return mesh != nullptr; }
    std::unique_ptr<Mesh> GetMesh() { return std::move(mesh); }

private:
    void CreateVertexBuffers(const std::vector<Engine::Vertex>& vertices);
    void CreateIndicesBuffers(const std::vector<uint16_t>& indices);

    std::unique_ptr<Mesh> mesh = nullptr;
    Engine::EngineDevice* device = nullptr;

    // Use EngineBuffer wrappers instead of raw Vulkan handles
    std::unique_ptr<Engine::EngineBuffer> vertexBuffer;
    std::unique_ptr<Engine::EngineBuffer> indexBuffer;

    uint32_t vertexCount = 0;
    uint32_t indicesCount = 0;
};
#pragma once

#include "vulkan/texture.hpp"
#include "engine/export.h"
#include <unordered_map>
#include <memory>
#include <string>

namespace Engine
{
    class OLIA_API TextureManager
    {
    public:
        TextureManager(EngineDevice &device);
        ~TextureManager() = default;

        TextureManager(const TextureManager &) = delete;
        TextureManager &operator=(const TextureManager &) = delete;

        // Loads and caches texture from file path. Returns fallback default texture if loading fails/empty.
        std::shared_ptr<Texture> LoadTexture(const std::string &filePath);

        // Returns 1x1 solid white fallback texture
        std::shared_ptr<Texture> GetDefaultTexture() const { return defaultTexture; }

        void Clear();

    private:
        EngineDevice &device;
        std::shared_ptr<Texture> defaultTexture;
        std::unordered_map<std::string, std::shared_ptr<Texture>> textureCache;
    };
} // namespace Engine
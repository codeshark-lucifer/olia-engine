#pragma once

#include "vulkan/device.hpp"
#include "engine/export.h"
#include <string>

namespace Engine
{
    class OLIA_API Texture
    {
    public:
        Texture(EngineDevice &device, const std::string &filePath);
        Texture(EngineDevice &device); // Default 1x1 White fallback texture
        ~Texture();

        Texture(const Texture &) = delete;
        Texture &operator=(const Texture &) = delete;

        VkImageView getImageView() const { return imageView; }
        VkSampler getSampler() const { return sampler; }

    private:
        void createTextureImage(const std::string &filePath);
        void createDefaultTexture();
        void createImageView();
        void createTextureSampler();
        void transitionImageLayout(VkImage image, VkFormat format, VkImageLayout oldLayout, VkImageLayout newLayout);

        EngineDevice &device;

        uint32_t width = 0;
        uint32_t height = 0;

        VkImage image = VK_NULL_HANDLE;
        VkDeviceMemory imageMemory = VK_NULL_HANDLE;
        VkImageView imageView = VK_NULL_HANDLE;
        VkSampler sampler = VK_NULL_HANDLE;
    };
} // namespace Engine
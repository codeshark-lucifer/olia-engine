#include "vulkan/texture-manager.hpp"
#include "utils/file.h"
#include <filesystem>
#include <iostream>

namespace Engine
{
    TextureManager::TextureManager(EngineDevice &device) : device(device)
    {
        // Create 1x1 solid white fallback texture
        defaultTexture = std::make_shared<Texture>(device);
    }

    std::shared_ptr<Texture> TextureManager::LoadTexture(const std::string &filePath)
    {
        // 1. If path is empty, return default 1x1 white texture
        if (filePath.empty())
        {
            return defaultTexture;
        }

        // 2. Return from cache if already loaded
        auto it = textureCache.find(filePath);
        if (it != textureCache.end())
        {
            return it->second;
        }

        std::string resolvedPath = "";
        std::filesystem::path targetFilename = std::filesystem::path(filePath).filename();

        // 3. Try original path directly (e.g. absolute or direct relative path)
        if (std::filesystem::exists(filePath) && std::filesystem::is_regular_file(filePath))
        {
            resolvedPath = filePath;
        }
        // 4. Try relative to executable directory
        else if (std::filesystem::exists(GetExecutableDir() / filePath) && std::filesystem::is_regular_file(GetExecutableDir() / filePath))
        {
            resolvedPath = (GetExecutableDir() / filePath).string();
        }
        // 5. Try relative to executable's assets folder
        else if (std::filesystem::exists(GetExecutableDir() / "assets" / filePath) && std::filesystem::is_regular_file(GetExecutableDir() / "assets" / filePath))
        {
            resolvedPath = (GetExecutableDir() / "assets" / filePath).string();
        }
        else
        {
            // 6. Search recursively inside ./assets/ for matching filename
            std::filesystem::path assetsDir = GetExecutableDir() / "assets";
            if (std::filesystem::exists(assetsDir) && std::filesystem::is_directory(assetsDir))
            {
                for (const auto &entry : std::filesystem::recursive_directory_iterator(assetsDir, std::filesystem::directory_options::skip_permission_denied))
                {
                    if (entry.is_regular_file() && entry.path().filename() == targetFilename)
                    {
                        resolvedPath = entry.path().string();
                        break;
                    }
                }
            }
        }

        // 7. If no valid file was found, fallback to default white texture
        if (resolvedPath.empty() || !std::filesystem::exists(resolvedPath))
        {
            std::cerr << "Texture file not found: '" << filePath << "'. Falling back to default texture.\n";
            textureCache[filePath] = defaultTexture;
            return defaultTexture;
        }

        // 8. Attempt to load the resolved texture
        try
        {
            auto texture = std::make_shared<Texture>(device, resolvedPath);
            textureCache[filePath] = texture;
            std::cout << "Texture loaded successfully: " << resolvedPath << "\n";
            return texture;
        }
        catch (const std::exception &e)
        {
            std::cerr << "Failed to load texture '" << resolvedPath << "': " << e.what() << ". Using fallback default texture.\n";
            textureCache[filePath] = defaultTexture;
            return defaultTexture;
        }
    }

    void TextureManager::Clear()
    {
        textureCache.clear();
    }
} // namespace Engine
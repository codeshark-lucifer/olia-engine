#include <olia/utils/filesystem.h>
#include <glad/gl.h>
#include <stb/stb_image.h>
#include <iostream>
#include <filesystem>

#ifdef OLIA_PLATFORM_WINDOWS
#include <windows.h>
#endif

namespace Olia
{
    std::string Filesystem::ResolvePath(const std::string& relativePath)
    {
        // 1. Try directly (relative to CWD)
        if (std::filesystem::exists(relativePath))
            return relativePath;

        // 2. Try relative to the executable directory
#ifdef OLIA_PLATFORM_WINDOWS
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        std::string::size_type pos = std::string(buffer).find_last_of("\\/");
        std::string exeDir = std::string(buffer).substr(0, pos);
        
        std::string resolved = exeDir + "/" + relativePath;
        if (std::filesystem::exists(resolved))
            return resolved;

        // 3. Try checking parent directories of executable directory
        // e.g. if running from build/app/application.exe, look in build/assets or root assets
        std::string parentResolved = exeDir + "/../" + relativePath;
        if (std::filesystem::exists(parentResolved))
            return parentResolved;

        std::string grandparentResolved = exeDir + "/../../" + relativePath;
        if (std::filesystem::exists(grandparentResolved))
            return grandparentResolved;
#endif

        return relativePath; // Fallback
    }

    Texture Filesystem::LoadTexture(const std::string& path)
    {
        Texture texture{ 0, 0, 0 };

        std::string resolvedPath = ResolvePath(path);

        // Flip textures vertically on load (standard for OpenGL)
        stbi_set_flip_vertically_on_load(false);

        int width, height, nrChannels;
        unsigned char* data = stbi_load(resolvedPath.c_str(), &width, &height, &nrChannels, 0);

        if (!data)
        {
            std::cerr << "Failed to load texture at path: " << path << " (resolved as: " << resolvedPath << ")" << std::endl;
            return texture;
        }

        GLenum internalFormat = GL_RGB;
        GLenum dataFormat = GL_RGB;

        if (nrChannels == 1)
        {
            internalFormat = GL_RED;
            dataFormat = GL_RED;
        }
        else if (nrChannels == 3)
        {
            internalFormat = GL_RGB;
            dataFormat = GL_RGB;
        }
        else if (nrChannels == 4)
        {
            internalFormat = GL_RGBA;
            dataFormat = GL_RGBA;
        }

        glGenTextures(1, &texture.id);
        glBindTexture(GL_TEXTURE_2D, texture.id);

        // Setup parameters
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_REPEAT);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_NEAREST);
        glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_NEAREST);

        glTexImage2D(GL_TEXTURE_2D, 0, internalFormat, width, height, 0, dataFormat, GL_UNSIGNED_BYTE, data);
        glGenerateMipmap(GL_TEXTURE_2D);

        stbi_image_free(data);

        texture.width = width;
        texture.height = height;

        return texture;
    }
}

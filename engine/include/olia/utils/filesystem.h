#pragma once

#include <string>
#include <core/built-in.h>

namespace Olia
{
    class Filesystem
    {
    public:
        static Texture LoadTexture(const std::string& path);
        static std::string ResolvePath(const std::string& relativePath);
    };
}

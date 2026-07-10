#pragma once

#include <string>
#include <core/built-in.h>

namespace Olia
{
    struct DecodedImageData
    {
        unsigned char* data = nullptr;
        int width = 0;
        int height = 0;
        int channels = 0;
    };

    class Filesystem
    {
    public:
        static Texture LoadTexture(const std::string& path);
        static std::string ResolvePath(const std::string& relativePath);

        // Multithreaded Image Decoding & Uploading
        static DecodedImageData DecodeImage(const std::string& path);
        static Texture UploadTexture(const DecodedImageData& img);
        static void FreeImageData(DecodedImageData& img);
    };
}

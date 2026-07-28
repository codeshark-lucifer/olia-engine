#pragma once

#include <string>
#include <filesystem>
#include <vector>
#include <fstream>

#ifdef _WIN32
#include <windows.h>
#elif __APPLE__
#include <mach-o/dyld.h>
#else
#include <unistd.h>
#endif

namespace Engine
{
    inline std::filesystem::path GetExecutablePath()
    {
#ifdef _WIN32
        char buffer[MAX_PATH];
        GetModuleFileNameA(NULL, buffer, MAX_PATH);
        return std::filesystem::path(buffer);

#elif __APPLE__
        char buffer[1024];
        uint32_t size = sizeof(buffer);
        if (_NSGetExecutablePath(buffer, &size) == 0)
            return std::filesystem::path(buffer);
        return "";

#else // Linux
        char buffer[1024];
        ssize_t count = readlink("/proc/self/exe", buffer, sizeof(buffer));
        if (count != -1)
            return std::filesystem::path(std::string(buffer, count));
        return "";
#endif
    }

    // Returns the folder where the EXE lives
    inline std::filesystem::path GetExecutableDir()
    {
        return GetExecutablePath().parent_path();
    }

    inline std::vector<char> ReadFile(const char *filepath)
    {
        // Open at the end (std::ios::ate) to determine file size immediately
        std::ifstream file(filepath, std::ios::binary | std::ios::ate);

        if (!file.is_open())
        {
            // Log error or handle appropriately
            fprintf(stderr, "Failed to open file for reading: %s\n", filepath);
            return {};
        }

        // Get file size and seek back to the beginning
        std::streamsize fileSize = file.tellg();
        file.seekg(0, std::ios::beg);

        std::vector<char> buffer(fileSize);

        // Read data into the vector's underlying array
        if (!file.read(buffer.data(), fileSize))
        {
            fprintf(stderr, "Failed to read data from file: %s\n", filepath);
            return {};
        }

        return buffer;
    }

    // Pass data by const reference to avoid a massive copy of the vector
    inline bool WriteFile(const char *filepath, const std::vector<char> &data)
    {
        // Open for writing in binary mode, truncate existing content
        std::ofstream file(filepath, std::ios::binary | std::ios::trunc);

        if (!file.is_open())
        {
            fprintf(stderr, "Failed to open file for writing: %s\n", filepath);
            return false;
        }

        file.write(data.data(), data.size());

        // Returns true if the stream is still in a "good" state after the write
        return file.good();
    }
}
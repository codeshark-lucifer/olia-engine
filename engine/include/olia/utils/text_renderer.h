#pragma once

#include <string>
#include <unordered_map>
#include <glm/glm.hpp>
#include <glad/gl.h>

#include <ft2build.h>
#include FT_FREETYPE_H

#include <renderer/batch.h>

namespace Olia
{
    struct Character {
        GLuint TextureID;     // ID handle of the glyph texture
        glm::ivec2 Size;      // Size of glyph
        glm::ivec2 Bearing;   // Offset from baseline to left/top of glyph
        unsigned int Advance; // Horizontal offset to advance to next glyph
    };

    class TextRenderer
    {
    public:
        TextRenderer();
        ~TextRenderer();

        bool LoadFont(const std::string& fontPath, unsigned int fontSize);
        
        // Renders text using HarfBuzz and FreeType
        void RenderText(const std::string& text, float x, float y, float scale, const glm::vec4& color);

        float GetTextWidth(const std::string& text, float scale);

    private:
        FT_Library m_FTLibrary = nullptr;
        FT_Face m_FTFace = nullptr;
        
        std::unordered_map<uint32_t, Character> m_Characters; // cache of glyph index to Character
        Batch m_Batch;
    };
}

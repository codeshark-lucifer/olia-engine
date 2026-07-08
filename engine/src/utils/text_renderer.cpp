#include <olia/utils/text_renderer.h>
#include <olia/utils/filesystem.h>
#include <olia/olia.h>
#include <iostream>

#include <hb.h>
#include <hb-ft.h>

namespace Olia
{
    TextRenderer::TextRenderer()
    {
        if (FT_Init_FreeType(&m_FTLibrary))
        {
            std::cerr << "ERROR::FREETYPE: Could not init FreeType Library" << std::endl;
        }
        m_Batch.Init();
    }

    TextRenderer::~TextRenderer()
    {
        for (auto& pair : m_Characters)
        {
            glDeleteTextures(1, &pair.second.TextureID);
        }
        if (m_FTFace)
        {
            FT_Done_Face(m_FTFace);
        }
        if (m_FTLibrary)
        {
            FT_Done_FreeType(m_FTLibrary);
        }
    }

    bool TextRenderer::LoadFont(const std::string& fontPath, unsigned int fontSize)
    {
        std::string resolvedPath = Filesystem::ResolvePath(fontPath);
        if (m_FTFace)
        {
            FT_Done_Face(m_FTFace);
            m_FTFace = nullptr;
        }

        if (FT_New_Face(m_FTLibrary, resolvedPath.c_str(), 0, &m_FTFace))
        {
            std::cerr << "ERROR::FREETYPE: Failed to load font at: " << resolvedPath << std::endl;
            return false;
        }

        // Set size to load glyphs as
        FT_Set_Pixel_Sizes(m_FTFace, 0, fontSize);
        return true;
    }

    void TextRenderer::RenderText(const std::string& text, float x, float y, float scale, const glm::vec4& color)
    {
        if (!m_FTFace)
        {
            std::cerr << "ERROR::TEXT_RENDERER: Font not loaded!" << std::endl;
            return;
        }

        // 1. Create HarfBuzz buffer and populate it
        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);

        // Detect if text contains Khmer characters (U+1780 to U+17FF in UTF-8 starts with 0xE1 0x9E or 0xE1 0x9F)
        bool has_khmer = false;
        for (size_t i = 0; i < text.length(); ++i)
        {
            if (static_cast<unsigned char>(text[i]) == 0xE1 && 
                i + 1 < text.length() && 
                (static_cast<unsigned char>(text[i+1]) == 0x9E || static_cast<unsigned char>(text[i+1]) == 0x9F))
            {
                has_khmer = true;
                break;
            }
        }

        if (has_khmer)
        {
            hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
            hb_buffer_set_script(buf, HB_SCRIPT_KHMER);
            hb_buffer_set_language(buf, hb_language_from_string("km", -1));
        }
        else
        {
            hb_buffer_guess_segment_properties(buf);
        }

        // 2. Create HarfBuzz font from FreeType face
        hb_font_t* hb_font = hb_ft_font_create(m_FTFace, NULL);

        // 3. Shape the text!
        hb_shape(hb_font, buf, NULL, 0);

        // 4. Retrieve glyph information and positions
        unsigned int glyph_count;
        hb_glyph_info_t* glyph_info = hb_buffer_get_glyph_infos(buf, &glyph_count);
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

        // 5. Draw the glyphs using our Batch
        m_Batch.Begin();

        for (unsigned int i = 0; i < glyph_count; ++i)
        {
            uint32_t glyph_index = glyph_info[i].codepoint;
            float x_offset = glyph_pos[i].x_offset / 64.0f;
            float y_offset = glyph_pos[i].y_offset / 64.0f;
            float x_advance = glyph_pos[i].x_advance / 64.0f;
            float y_advance = glyph_pos[i].y_advance / 64.0f;

            // Load character glyph if not in cache
            if (m_Characters.find(glyph_index) == m_Characters.end())
            {
                if (FT_Load_Glyph(m_FTFace, glyph_index, FT_LOAD_RENDER))
                {
                    std::cerr << "ERROR::FREETYPE: Failed to load Glyph: " << glyph_index << std::endl;
                    continue;
                }

                // Generate texture
                GLuint textureID;
                glGenTextures(1, &textureID);
                glBindTexture(GL_TEXTURE_2D, textureID);

                // Set swizzle mask so GL_RED replicates into Alpha (RGB becomes 1.0, A becomes Red value)
                GLint swizzleMask[] = { GL_ONE, GL_ONE, GL_ONE, GL_RED };
                glTexParameteriv(GL_TEXTURE_2D, GL_TEXTURE_SWIZZLE_RGBA, swizzleMask);

                // Disable byte-alignment restriction (since glyphs are 1-byte per pixel)
                glPixelStorei(GL_UNPACK_ALIGNMENT, 1);

                glTexImage2D(
                    GL_TEXTURE_2D,
                    0,
                    GL_RED,
                    m_FTFace->glyph->bitmap.width,
                    m_FTFace->glyph->bitmap.rows,
                    0,
                    GL_RED,
                    GL_UNSIGNED_BYTE,
                    m_FTFace->glyph->bitmap.buffer
                );

                // Reset unpack alignment
                glPixelStorei(GL_UNPACK_ALIGNMENT, 4);

                // Set texture options
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
                glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);

                // Store character in cache
                Character character = {
                    textureID,
                    glm::ivec2(m_FTFace->glyph->bitmap.width, m_FTFace->glyph->bitmap.rows),
                    glm::ivec2(m_FTFace->glyph->bitmap_left, m_FTFace->glyph->bitmap_top),
                    static_cast<unsigned int>(m_FTFace->glyph->advance.x)
                };
                m_Characters[glyph_index] = character;
            }

            Character ch = m_Characters[glyph_index];

            if (!m_Batch.HasTextureSpace(ch.TextureID))
            {
                m_Batch.End();
                m_Batch.Flush(*context.shader);
                m_Batch.Begin();
            }

            float xpos = x + (ch.Bearing.x + x_offset) * scale;
            float ypos = y - (ch.Bearing.y + y_offset) * scale;

            float w = ch.Size.x * scale;
            float h = ch.Size.y * scale;

            // Add quad to batch if size > 0 (e.g. skip space character)
            if (w > 0 && h > 0)
            {
                m_Batch.DrawQuad({ xpos, ypos, 0.0f }, { w, h }, color, ch.TextureID);
            }

            // Move cursor for next glyph
            x += x_advance * scale;
            y -= y_advance * scale;
        }

        m_Batch.End();
        m_Batch.Flush(*context.shader);

        // Clean up HarfBuzz
        hb_buffer_destroy(buf);
        hb_font_destroy(hb_font);
    }

    float TextRenderer::GetTextWidth(const std::string& text, float scale)
    {
        if (!m_FTFace) return 0.0f;

        hb_buffer_t* buf = hb_buffer_create();
        hb_buffer_add_utf8(buf, text.c_str(), -1, 0, -1);

        // Detect if text contains Khmer characters
        bool has_khmer = false;
        for (size_t i = 0; i < text.length(); ++i)
        {
            if (static_cast<unsigned char>(text[i]) == 0xE1 && 
                i + 1 < text.length() && 
                (static_cast<unsigned char>(text[i+1]) == 0x9E || static_cast<unsigned char>(text[i+1]) == 0x9F))
            {
                has_khmer = true;
                break;
            }
        }

        if (has_khmer)
        {
            hb_buffer_set_direction(buf, HB_DIRECTION_LTR);
            hb_buffer_set_script(buf, HB_SCRIPT_KHMER);
            hb_buffer_set_language(buf, hb_language_from_string("km", -1));
        }
        else
        {
            hb_buffer_guess_segment_properties(buf);
        }

        hb_font_t* hb_font = hb_ft_font_create(m_FTFace, NULL);
        hb_shape(hb_font, buf, NULL, 0);

        unsigned int glyph_count;
        hb_glyph_position_t* glyph_pos = hb_buffer_get_glyph_positions(buf, &glyph_count);

        float totalWidth = 0.0f;
        for (unsigned int i = 0; i < glyph_count; ++i)
        {
            totalWidth += (glyph_pos[i].x_advance / 64.0f) * scale;
        }

        hb_buffer_destroy(buf);
        hb_font_destroy(hb_font);

        return totalWidth;
    }
}

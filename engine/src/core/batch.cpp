#include <renderer/batch.h>

#include <olia/utils/shader.h>

namespace Olia
{

Batch::~Batch()
{
    if (m_EBO) glDeleteBuffers(1, &m_EBO);
    if (m_VBO) glDeleteBuffers(1, &m_VBO);
    if (m_VAO) glDeleteVertexArrays(1, &m_VAO);
}

void Batch::Init()
{
    glGenVertexArrays(1, &m_VAO);
    glGenBuffers(1, &m_VBO);
    glGenBuffers(1, &m_EBO);

    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);

    glEnableVertexAttribArray(0);
    glVertexAttribPointer(
        0,
        3,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, position));

    glEnableVertexAttribArray(1);
    glVertexAttribPointer(
        1,
        4,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, color));

    glEnableVertexAttribArray(2);
    glVertexAttribPointer(
        2,
        2,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, texCoords));

    glEnableVertexAttribArray(3);
    glVertexAttribPointer(
        3,
        1,
        GL_FLOAT,
        GL_FALSE,
        sizeof(Vertex),
        (void*)offsetof(Vertex, texIndex));

    glBindVertexArray(0);
}

void Batch::Begin()
{
    m_Vertices.clear();
    m_Indices.clear();
    m_TextureSlots.clear();
}

void Batch::DrawQuad(
    const glm::vec3& position,
    const glm::vec2& size,
    const glm::vec4& color,
    uint32_t textureID,
    float rotation)
{
    float texIndex = 0.0f;

    if (textureID != 0)
    {
        int slot = -1;
        for (size_t i = 0; i < m_TextureSlots.size(); ++i)
        {
            if (m_TextureSlots[i] == textureID)
            {
                slot = static_cast<int>(i);
                break;
            }
        }

        if (slot == -1)
        {
            if (m_TextureSlots.size() < 8)
            {
                m_TextureSlots.push_back(textureID);
                slot = static_cast<int>(m_TextureSlots.size() - 1);
            }
            else
            {
                // slots full, fallback to slot 0
                slot = 0;
            }
        }

        texIndex = static_cast<float>(slot + 1);
    }

    uint32_t start = static_cast<uint32_t>(m_Vertices.size());

    if (glm::abs(rotation) < 0.0001f)
    {
        m_Vertices.push_back({
            position,
            color,
            { 0.0f, 0.0f },
            texIndex
        });

        m_Vertices.push_back({
            {position.x + size.x, position.y, position.z},
            color,
            { 1.0f, 0.0f },
            texIndex
        });

        m_Vertices.push_back({
            {position.x + size.x, position.y + size.y, position.z},
            color,
            { 1.0f, 1.0f },
            texIndex
        });

        m_Vertices.push_back({
            {position.x, position.y + size.y, position.z},
            color,
            { 0.0f, 1.0f },
            texIndex
        });
    }
    else
    {
        // Calculate center of the quad
        glm::vec2 center = glm::vec2(position.x + size.x * 0.5f, position.y + size.y * 0.5f);
        float cosRot = glm::cos(rotation);
        float sinRot = glm::sin(rotation);

        auto rotatePoint = [&](float x, float y) -> glm::vec3 {
            float rx = (x - center.x) * cosRot - (y - center.y) * sinRot + center.x;
            float ry = (x - center.x) * sinRot + (y - center.y) * cosRot + center.y;
            return glm::vec3(rx, ry, position.z);
        };

        m_Vertices.push_back({
            rotatePoint(position.x, position.y),
            color,
            { 0.0f, 0.0f },
            texIndex
        });

        m_Vertices.push_back({
            rotatePoint(position.x + size.x, position.y),
            color,
            { 1.0f, 0.0f },
            texIndex
        });

        m_Vertices.push_back({
            rotatePoint(position.x + size.x, position.y + size.y),
            color,
            { 1.0f, 1.0f },
            texIndex
        });

        m_Vertices.push_back({
            rotatePoint(position.x, position.y + size.y),
            color,
            { 0.0f, 1.0f },
            texIndex
        });
    }

    m_Indices.push_back(start + 0);
    m_Indices.push_back(start + 1);
    m_Indices.push_back(start + 2);

    m_Indices.push_back(start + 2);
    m_Indices.push_back(start + 3);
    m_Indices.push_back(start + 0);
}

void Batch::End()
{
    glBindVertexArray(m_VAO);

    glBindBuffer(GL_ARRAY_BUFFER, m_VBO);
    glBufferData(
        GL_ARRAY_BUFFER,
        m_Vertices.size() * sizeof(Vertex),
        m_Vertices.data(),
        GL_DYNAMIC_DRAW);

    glBindBuffer(GL_ELEMENT_ARRAY_BUFFER, m_EBO);
    glBufferData(
        GL_ELEMENT_ARRAY_BUFFER,
        m_Indices.size() * sizeof(uint32_t),
        m_Indices.data(),
        GL_DYNAMIC_DRAW);

    glBindVertexArray(0);
}

void Batch::Flush(Shader& shader)
{
    if (m_Indices.empty())
        return;

    shader.Bind();

    // Bind texture slots
    for (size_t i = 0; i < m_TextureSlots.size(); ++i)
    {
        glActiveTexture(GL_TEXTURE0 + static_cast<GLenum>(i));
        glBindTexture(GL_TEXTURE_2D, m_TextureSlots[i]);
    }

    glBindVertexArray(m_VAO);

    glDrawElements(
        GL_TRIANGLES,
        static_cast<GLsizei>(m_Indices.size()),
        GL_UNSIGNED_INT,
        nullptr);

    glBindVertexArray(0);
}

bool Batch::HasTextureSpace(uint32_t textureID) const
{
    if (textureID == 0) return true;
    for (uint32_t id : m_TextureSlots)
    {
        if (id == textureID) return true;
    }
    return m_TextureSlots.size() < 8;
}

}
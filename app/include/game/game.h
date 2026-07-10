#pragma once
#include <unordered_map>
#include <core.h>

struct Platform
{
    glm::vec2 position;
    glm::vec2 size;
    glm::vec4 color;
};

struct Player
{
    Olia::Entity id;

    int frameWidth = 32;

    std::unordered_map<std::string, Animation> animations;

    Animation* current = nullptr;

    // Physics state
    glm::vec2 velocity{0.0f, 0.0f};
    bool isGrounded = false;
    bool facingRight = true;
    int doubleJumpsLeft = 1;

    // Colliders (inset relative to the 64x64 visual size)
    glm::vec2 colliderSize{26.0f, 54.0f};
    glm::vec2 colliderOffset{19.0f, 10.0f};
};

static void LoadAnimation(
    Player& p,
    const std::string& name,
    const std::string& path,
    int frames,
    float duration = 0.05f)
{
    Animation animation;

    animation.texture = Olia::Filesystem::LoadTexture(path);
    animation.frames = frames;
    animation.frameDuration = duration;

    p.animations.emplace(name, std::move(animation));
}

static void PlayAnimation(Player& p, const std::string& name)
{
    auto it = p.animations.find(name);

    if (it == p.animations.end())
        return;

    if (p.current != &it->second)
    {
        p.current = &it->second;
        frametime = 0.0f;
    }
}

static void UpdateAnimation(Player& p)
{
    if (!p.current)
        return;

    Animation& animation = *p.current;

    int frame =
        static_cast<int>(frametime / animation.frameDuration) %
        animation.frames;

    // Calculate sub-pixel padding to prevent texture bleeding
    // Shifting coordinates inwards by roughly half a pixel or 1 pixel prevents adjacent frame bleeding
    float texelWidth = 1.0f / (float)animation.texture.width;
    float texelHeight = 1.0f / (float)animation.texture.height;

    // Shift boundaries slightly inward to skip the edge pixel boundaries
    float u0 = (float(frame * p.frameWidth) / animation.texture.width) + (0.5f * texelWidth);
    float u1 = (float((frame + 1) * p.frameWidth) / animation.texture.width) - (0.5f * texelWidth);
    
    // Shift the top (0.0f) down by 1 pixel, and bottom up slightly to stay within bounds
    float v0 = 0.0f + (1.0f * texelHeight); 
    float v1 = 1.0f - (0.5f * texelHeight);

    auto& sprite =
        Olia::context.ecs->Get<Olia::SpriteRenderer>(p.id);

    sprite.texture = &animation.texture;

    if (p.facingRight)
    {
        sprite.texCoords[0] = { u0, v0 };
        sprite.texCoords[1] = { u1, v0 };
        sprite.texCoords[2] = { u1, v1 };
        sprite.texCoords[3] = { u0, v1 };
    }
    else
    {
        sprite.texCoords[0] = { u1, v0 };
        sprite.texCoords[1] = { u0, v0 };
        sprite.texCoords[2] = { u0, v1 };
        sprite.texCoords[3] = { u1, v1 };
    }
}
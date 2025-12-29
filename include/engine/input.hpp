#pragma once
#include <engine/singleton.hpp>
#include <engine/window/platform.hpp>
#include <SDL3/SDL.h>
#include <SDL3/SDL_mouse.h>
#include <unordered_map>

class InputManager : public Singleton<InputManager>
{
    friend class Singleton<InputManager>; // REQUIRED

private:
    InputManager() = default;

    std::unordered_map<SDL_Keycode, bool> currentKeys;
    std::unordered_map<SDL_Keycode, bool> previousKeys;

    uint32_t mouseButtons = 0;
    uint32_t prevMouseButtons = 0;

    float mouseX = 0, mouseY = 0;
    float mouseDX = 0, mouseDY = 0;

public:
    void Update();

    bool IsKeyDown(SDL_Keycode key) const;
    bool IsKeyPressed(SDL_Keycode key) const;
    bool IsKeyReleased(SDL_Keycode key) const;

    bool IsMouseDown(uint32_t button) const;
    bool IsMousePressed(uint32_t button) const;
    bool IsMouseReleased(uint32_t button) const;

    float GetMousePosX() const { return mouseX; }
    float GetMousePosY() const { return mouseY; }
    float GetMouseDX() const { return mouseDX; }
    float GetMouseDY() const { return mouseDY; }

    void SetMouseLock(bool state) {
        SDL_SetWindowRelativeMouseMode(Platform::Get().GetWindow(), state);
    }
};

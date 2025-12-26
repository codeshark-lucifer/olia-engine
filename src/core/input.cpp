#include <engine/input.hpp>

void InputManager::Update()
{
    previousKeys = currentKeys;
    prevMouseButtons = mouseButtons;

    float x, y;
    mouseButtons = SDL_GetRelativeMouseState(&x, &y);
    mouseDX = x;
    mouseDY = y;

    mouseButtons = SDL_GetMouseState(&x, &y);
    mouseX = x;
    mouseY = y;

    int numKeys;
    const bool *state = SDL_GetKeyboardState(&numKeys);
    for (int i = 0; i < numKeys; ++i)
    {
        SDL_Keycode key = SDL_GetKeyFromScancode((SDL_Scancode)i, SDL_KMOD_NONE, false);
        if (key != SDLK_UNKNOWN)
        {
            currentKeys[key] = (state[i] != 0);
        }
    }
}

bool InputManager::IsKeyDown(SDL_Keycode key) const
{
    auto it = currentKeys.find(key);
    return it != currentKeys.end() && it->second;
}

bool InputManager::IsKeyPressed(SDL_Keycode key) const
{
    bool isDown = IsKeyDown(key);
    auto it = previousKeys.find(key);
    bool wasDown = (it != previousKeys.end() && it->second);
    return isDown && !wasDown;
}

bool InputManager::IsKeyReleased(SDL_Keycode key) const
{
    bool isDown = IsKeyDown(key);
    auto it = previousKeys.find(key);
    bool wasDown = (it != previousKeys.end() && it->second);
    return !isDown && wasDown;
}

bool InputManager::IsMouseDown(uint32_t button) const
{
    return (mouseButtons & SDL_BUTTON_MASK(button)) != 0;
}

bool InputManager::IsMousePressed(uint32_t button) const
{
    bool isDown = IsMouseDown(button);
    bool wasDown = (prevMouseButtons & SDL_BUTTON_MASK(button)) != 0;
    return isDown && !wasDown;
}

bool InputManager::IsMouseReleased(uint32_t button) const
{
    bool isDown = IsMouseDown(button);
    bool wasDown = (prevMouseButtons & SDL_BUTTON_MASK(button)) != 0;
    return !isDown && wasDown;
}
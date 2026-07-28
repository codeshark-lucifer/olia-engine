#pragma once

#include <SDL3/SDL.h>
#include "engine/export.h"
#include <cstdio>
#include <cstring>

enum class KeyCode : int
{
    Unknown = 0,

    // Letters (Physical positions)
    A = SDL_SCANCODE_A,
    B = SDL_SCANCODE_B,
    C = SDL_SCANCODE_C,
    D = SDL_SCANCODE_D,
    E = SDL_SCANCODE_E,
    F = SDL_SCANCODE_F,
    G = SDL_SCANCODE_G,
    H = SDL_SCANCODE_H,
    I = SDL_SCANCODE_I,
    J = SDL_SCANCODE_J,
    K = SDL_SCANCODE_K,
    L = SDL_SCANCODE_L,
    M = SDL_SCANCODE_M,
    N = SDL_SCANCODE_N,
    O = SDL_SCANCODE_O,
    P = SDL_SCANCODE_P,
    Q = SDL_SCANCODE_Q,
    R = SDL_SCANCODE_R,
    S = SDL_SCANCODE_S,
    T = SDL_SCANCODE_T,
    U = SDL_SCANCODE_U,
    V = SDL_SCANCODE_V,
    W = SDL_SCANCODE_W,
    X = SDL_SCANCODE_X,
    Y = SDL_SCANCODE_Y,
    Z = SDL_SCANCODE_Z,

    // Numbers
    Alpha1 = SDL_SCANCODE_1,
    Alpha2 = SDL_SCANCODE_2,
    Alpha3 = SDL_SCANCODE_3,
    Alpha4 = SDL_SCANCODE_4,
    Alpha5 = SDL_SCANCODE_5,
    Alpha6 = SDL_SCANCODE_6,
    Alpha7 = SDL_SCANCODE_7,
    Alpha8 = SDL_SCANCODE_8,
    Alpha9 = SDL_SCANCODE_9,
    Alpha0 = SDL_SCANCODE_0,

    // Functions
    F1 = SDL_SCANCODE_F1,
    F2 = SDL_SCANCODE_F2,
    F3 = SDL_SCANCODE_F3,
    F4 = SDL_SCANCODE_F4,
    F5 = SDL_SCANCODE_F5,
    F6 = SDL_SCANCODE_F6,
    F7 = SDL_SCANCODE_F7,
    F8 = SDL_SCANCODE_F8,
    F9 = SDL_SCANCODE_F9,
    F10 = SDL_SCANCODE_F10,
    F11 = SDL_SCANCODE_F11,
    F12 = SDL_SCANCODE_F12,

    // Controls
    Escape = SDL_SCANCODE_ESCAPE,
    Enter = SDL_SCANCODE_RETURN,
    Tab = SDL_SCANCODE_TAB,
    Space = SDL_SCANCODE_SPACE,
    Backspace = SDL_SCANCODE_BACKSPACE,
    LShift = SDL_SCANCODE_LSHIFT,
    RShift = SDL_SCANCODE_RSHIFT,
    LControl = SDL_SCANCODE_LCTRL,
    RControl = SDL_SCANCODE_RCTRL,
    LAlt = SDL_SCANCODE_LALT,
    RAlt = SDL_SCANCODE_RALT,

    // Navigation
    Up = SDL_SCANCODE_UP,
    Down = SDL_SCANCODE_DOWN,
    Left = SDL_SCANCODE_LEFT,
    Right = SDL_SCANCODE_RIGHT,
    Insert = SDL_SCANCODE_INSERT,
    Delete = SDL_SCANCODE_DELETE,
    Home = SDL_SCANCODE_HOME,
    End = SDL_SCANCODE_END,
    PageUp = SDL_SCANCODE_PAGEUP,
    PageDown = SDL_SCANCODE_PAGEDOWN,

    // Mobile / Console Specific Mappings (via SDL Scancodes)
    // Often mapped to the 'Back' button on Android or 'B' on some controllers
    AppBack = SDL_SCANCODE_AC_BACK,
    AppHome = SDL_SCANCODE_AC_HOME,
    AppMenu = SDL_SCANCODE_MENU,

    // Numpad
    Num1 = SDL_SCANCODE_KP_1,
    Num2 = SDL_SCANCODE_KP_2,
    Num3 = SDL_SCANCODE_KP_3,
    Num4 = SDL_SCANCODE_KP_4,
    Num5 = SDL_SCANCODE_KP_5,
    Num6 = SDL_SCANCODE_KP_6,
    Num7 = SDL_SCANCODE_KP_7,
    Num8 = SDL_SCANCODE_KP_8,
    Num9 = SDL_SCANCODE_KP_9,
    Num0 = SDL_SCANCODE_KP_0,
    NumPlus = SDL_SCANCODE_KP_PLUS,
    NumMinus = SDL_SCANCODE_KP_MINUS,
    NumEnter = SDL_SCANCODE_KP_ENTER
};

enum class MouseButton : uint8_t
{
    Left = SDL_BUTTON_LEFT,
    Middle = SDL_BUTTON_MIDDLE,
    Right = SDL_BUTTON_RIGHT,
    X1 = SDL_BUTTON_X1,
    X2 = SDL_BUTTON_X2
};

constexpr double FIXED_DELTA_TIME = 1.0 / 60.0;

struct Time
{
    uint64_t lastCounter = 0;
    uint64_t frequency = 0;

    double deltaTime = 0.0;
    double accumulator = 0.0;
};

class OLIA_API Input
{
private:
    Time time;
    uint64_t lastFrame = 0;

    // Keyboard State
    const bool *keyboard = nullptr;
    uint8_t prevKeyboard[SDL_SCANCODE_COUNT];

    // Mouse State
    float mouseX = 0, mouseY = 0;
    uint32_t mouseState = 0;
    uint32_t prevMouseState = 0;

public:
    void Init()
    {
        keyboard = SDL_GetKeyboardState(nullptr);
        std::memset(prevKeyboard, 0, SDL_SCANCODE_COUNT);

        time.frequency = SDL_GetPerformanceFrequency();
        time.lastCounter = SDL_GetPerformanceCounter();
    }

    void Update()
    {
        // Capture previous state
        std::memcpy(prevKeyboard, keyboard, SDL_SCANCODE_COUNT);
        prevMouseState = mouseState;

        SDL_PumpEvents();

        // Capture new state
        mouseState = SDL_GetMouseState(&mouseX, &mouseY);

        lastFrame = time.lastCounter;
        time.lastCounter = SDL_GetPerformanceCounter();
        time.deltaTime = (double)(time.lastCounter - lastFrame) / (double)time.frequency;

        if (time.deltaTime > 0.1)
            time.deltaTime = 0.1;
        time.accumulator += time.deltaTime;
    }
    // --- KEYBOARD QUERIES ---
    bool GetKey(KeyCode key) const
    {
        return keyboard[static_cast<int>(key)];
    }

    bool GetKeyDown(KeyCode key) const
    {
        int idx = static_cast<int>(key);
        return keyboard[idx] && !prevKeyboard[idx];
    }

    bool GetKeyUp(KeyCode key) const
    {
        int idx = static_cast<int>(key);
        return !keyboard[idx] && prevKeyboard[idx];
    }

    // --- MOUSE QUERIES ---
    void GetMousePosition(float *x, float *y) const
    {
        *x = mouseX;
        *y = mouseY;
    }

    bool GetMouseButton(MouseButton button) const
    {
        return mouseState & SDL_BUTTON_MASK(static_cast<int>(button));
    }

    bool GetMouseButtonDown(MouseButton button) const
    {
        uint32_t mask = SDL_BUTTON_MASK(static_cast<int>(button));
        return (mouseState & mask) && !(prevMouseState & mask);
    }

    bool GetMouseButtonUp(MouseButton button) const
    {
        uint32_t mask = SDL_BUTTON_MASK(static_cast<int>(button));
        return !(mouseState & mask) && (prevMouseState & mask);
    }

    const Time &GetTime() const { return time; }
};

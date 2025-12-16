#pragma once
#include <SDL3/SDL.h>
#include <glm/glm.hpp>
#include <unordered_map>
#include <vector>
#include <string>
#include <cstring>

// ======================================================
// Keyboard Enum
// ======================================================
enum class Keyboard
{
    A,
    B,
    C,
    D,
    E,
    F,
    G,
    H,
    I,
    J,
    K,
    L,
    M,
    N,
    O,
    P,
    Q,
    R,
    S,
    T,
    U,
    V,
    W,
    X,
    Y,
    Z,

    Up,
    Down,
    Left,
    Right,
    Space,
    Enter,
    Escape,
    LShift,
    LCtrl,
    LAlt
};

// ======================================================
// Mouse Enum
// ======================================================
enum class MouseButton
{
    Left,
    Right,
    Middle
};

// ======================================================
// Key Maps
// ======================================================
static inline const std::unordered_map<Keyboard, SDL_Scancode> KeyMap =
    {
        {Keyboard::W, SDL_SCANCODE_W},
        {Keyboard::A, SDL_SCANCODE_A},
        {Keyboard::S, SDL_SCANCODE_S},
        {Keyboard::D, SDL_SCANCODE_D},

        {Keyboard::Up, SDL_SCANCODE_UP},
        {Keyboard::Down, SDL_SCANCODE_DOWN},
        {Keyboard::Left, SDL_SCANCODE_LEFT},
        {Keyboard::Right, SDL_SCANCODE_RIGHT},

        {Keyboard::Space, SDL_SCANCODE_SPACE},
        {Keyboard::Enter, SDL_SCANCODE_RETURN},
        {Keyboard::Escape, SDL_SCANCODE_ESCAPE},

        {Keyboard::LShift, SDL_SCANCODE_LSHIFT},
        {Keyboard::LCtrl, SDL_SCANCODE_LCTRL},
        {Keyboard::LAlt, SDL_SCANCODE_LALT}};

static inline const std::unordered_map<MouseButton, Uint32> MouseMap =
    {
        {MouseButton::Left, SDL_BUTTON_LEFT},
        {MouseButton::Right, SDL_BUTTON_RIGHT},
        {MouseButton::Middle, SDL_BUTTON_MIDDLE}};

// ======================================================
// Binding Types
// ======================================================
struct ButtonBinding
{
    Keyboard key;
};

struct MouseButtonBinding
{
    MouseButton button;
};

struct AxisBinding
{
    Keyboard negative;
    Keyboard positive;
};

enum class MouseAxis
{
    X,
    Y
};

struct MouseAxisBinding
{
    MouseAxis axis;
    float sensitivity = 1.0f;
};

// ======================================================
// Input Action
// ======================================================
struct InputAction
{
    std::string name;

    std::vector<ButtonBinding> keyboardButtons;
    std::vector<MouseButtonBinding> mouseButtons;
    std::vector<AxisBinding> keyboardAxes;
    std::vector<MouseAxisBinding> mouseAxes;

    bool current = false;
    bool previous = false;
};

// ======================================================
// Input System (Unity-style)
// ======================================================
class Input
{
public:
    // ---------- Setup ----------
    static void Initialize()
    {
        keyboardState = SDL_GetKeyboardState(nullptr);

        // Default actions (Unity-style)
        CreateAction("Jump").keyboardButtons =
            {
                {Keyboard::Space}};

        CreateAction("Fire").mouseButtons =
            {
                {MouseButton::Left}};

        CreateAction("Move").keyboardAxes =
            {
                {Keyboard::A, Keyboard::D},
                {Keyboard::Left, Keyboard::Right}};

        CreateAction("LookX").mouseAxes =
            {
                {MouseAxis::X, 0.1f}};

        CreateAction("LookY").mouseAxes =
            {
                {MouseAxis::Y, 0.1f}};
    }

    // ---------- Update ----------
    static void Update()
    {
        SDL_PumpEvents();

        keyboardState = SDL_GetKeyboardState(nullptr);

        // Mouse
        previousMousePos = mousePos;
        mouseButtonsState = SDL_GetMouseState(&mousePos.x, &mousePos.y);
        mouseDelta = mousePos - previousMousePos;

        for (auto &[_, action] : actions)
        {
            action.previous = action.current;
            action.current = false;

            // Keyboard buttons
            for (auto &b : action.keyboardButtons)
            {
                if (IsKeyDown(b.key))
                {
                    action.current = true;
                    break;
                }
            }

            // Mouse buttons
            for (auto &b : action.mouseButtons)
            {
                if (IsMouseDown(b.button))
                {
                    action.current = true;
                    break;
                }
            }
        }
    }

    // ---------- Query ----------
    static bool GetAction(const std::string &name)
    {
        return actions[name].current;
    }

    static bool GetActionDown(const std::string &name)
    {
        auto &a = actions[name];
        return a.current && !a.previous;
    }

    static bool GetActionUp(const std::string &name)
    {
        auto &a = actions[name];
        return !a.current && a.previous;
    }

    static float GetAxis(const std::string &name)
    {
        float value = 0.0f;
        auto &a = actions[name];

        // Keyboard axes
        for (auto &axis : a.keyboardAxes)
        {
            value += (IsKeyDown(axis.positive) - IsKeyDown(axis.negative));
        }

        // Mouse axes
        for (auto &axis : a.mouseAxes)
        {
            if (axis.axis == MouseAxis::X)
                value += mouseDelta.x * axis.sensitivity;
            else if (axis.axis == MouseAxis::Y)
                value += mouseDelta.y * axis.sensitivity;
        }

        return value;
    }

    // ---------- Mouse Direct ----------
    static glm::ivec2 GetMousePosition()
    {
        return mousePos;
    }

    static glm::ivec2 GetMouseDelta()
    {
        return mouseDelta;
    }

private:
    // Keyboard
    static inline const bool *keyboardState = nullptr;

    // Mouse
    static inline Uint32 mouseButtonsState = 0;
    static inline glm::vec2 mousePos{0.0f};
    static inline glm::vec2 previousMousePos{0.0f};
    static inline glm::vec2 mouseDelta{0.0f};

    // Actions
    static inline std::unordered_map<std::string, InputAction> actions;

private:
    static bool IsKeyDown(Keyboard key)
    {
        return keyboardState && keyboardState[KeyMap.at(key)];
    }

    static bool IsMouseDown(MouseButton button)
    {
        return mouseButtonsState & MouseMap.at(button);
    }

    static InputAction &CreateAction(const std::string &name)
    {
        actions[name] = InputAction{};
        actions[name].name = name;
        return actions[name];
    }
};

#include <core/input.h>
#include <cstring>
#include <olia/olia.h>

namespace Olia
{
    GLFWwindow* InputManager::s_Window = nullptr;

    bool InputManager::s_CurrentKeys[GLFW_KEY_LAST + 1]{};
    bool InputManager::s_PreviousKeys[GLFW_KEY_LAST + 1]{};

    bool InputManager::s_CurrentMouse[GLFW_MOUSE_BUTTON_LAST + 1]{};
    bool InputManager::s_PreviousMouse[GLFW_MOUSE_BUTTON_LAST + 1]{};

    void InputManager::Initialize(GLFWwindow* window)
    {
        s_Window = window;
    }

    void InputManager::Update()
    {
        std::memcpy(s_PreviousKeys, s_CurrentKeys, sizeof(s_CurrentKeys));
        std::memcpy(s_PreviousMouse, s_CurrentMouse, sizeof(s_CurrentMouse));

        for (int i = 0; i <= GLFW_KEY_LAST; i++)
            s_CurrentKeys[i] = glfwGetKey(s_Window, i) == GLFW_PRESS;

        for (int i = 0; i <= GLFW_MOUSE_BUTTON_LAST; i++)
            s_CurrentMouse[i] = glfwGetMouseButton(s_Window, i) == GLFW_PRESS;
    }

    bool InputManager::GetKey(int key)
    {
        return s_CurrentKeys[key];
    }

    bool InputManager::GetKeyDown(int key)
    {
        return s_CurrentKeys[key] && !s_PreviousKeys[key];
    }

    bool InputManager::GetKeyUp(int key)
    {
        return !s_CurrentKeys[key] && s_PreviousKeys[key];
    }

    bool InputManager::GetMouseButton(int button)
    {
        return s_CurrentMouse[button];
    }

    bool InputManager::GetMouseButtonDown(int button)
    {
        return s_CurrentMouse[button] && !s_PreviousMouse[button];
    }

    bool InputManager::GetMouseButtonUp(int button)
    {
        return !s_CurrentMouse[button] && s_PreviousMouse[button];
    }

    void InputManager::GetMousePosition(double& x, double& y)
    {
        double physicalX, physicalY;
        glfwGetCursorPos(s_Window, &physicalX, &physicalY);

        int winWidth, winHeight;
        glfwGetWindowSize(s_Window, &winWidth, &winHeight);

        if (winWidth > 0 && winHeight > 0)
        {
            x = physicalX * (static_cast<double>(context.virtualWidth) / winWidth);
            y = physicalY * (static_cast<double>(context.virtualHeight) / winHeight);
        }
        else
        {
            x = physicalX;
            y = physicalY;
        }
    }
}
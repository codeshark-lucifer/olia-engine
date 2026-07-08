#pragma once
#define GLFW_INCLUDE_NONE
#include <GLFW/glfw3.h>

namespace Olia
{
    class InputManager
    {
    public:
        static void Initialize(GLFWwindow* window);

        static bool GetKey(int key);
        static bool GetKeyDown(int key);
        static bool GetKeyUp(int key);

        static bool GetMouseButton(int button);
        static bool GetMouseButtonDown(int button);
        static bool GetMouseButtonUp(int button);

        static void GetMousePosition(double& x, double& y);

        static void Update();

    private:
        static GLFWwindow* s_Window;

        static bool s_CurrentKeys[GLFW_KEY_LAST + 1];
        static bool s_PreviousKeys[GLFW_KEY_LAST + 1];

        static bool s_CurrentMouse[GLFW_MOUSE_BUTTON_LAST + 1];
        static bool s_PreviousMouse[GLFW_MOUSE_BUTTON_LAST + 1];
    };
}
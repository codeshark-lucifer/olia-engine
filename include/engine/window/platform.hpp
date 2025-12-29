#pragma once
#include <glad/glad.h>
#include <SDL3/SDL.h>
#include <stdexcept>
#include <functional>
#include <engine/singleton.hpp>

class Platform : public Singleton<Platform>
{
    friend class Singleton<Platform>; // REQUIRED

public:
Platform()=default;
    void Init(int width, int height, const char *title)
    {
        if (!SDL_Init(SDL_INIT_VIDEO))
        {
            throw std::runtime_error("Failed to initialize SDL3.");
        }

        // Set OpenGL version (example: 3.3 Core Profile)
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
        SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);

        // Enable double buffering
        SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
        window = SDL_CreateWindow(
            title,
            width,
            height,
            SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);
        if (!window)
        {
            throw std::runtime_error("Failed to create Window.");
            SDL_Quit();
        }
        gl_context = SDL_GL_CreateContext(window);
        if (!gl_context)
        {
            throw std::runtime_error("Failed to create GL_Context.");
            SDL_DestroyWindow(window);
            SDL_Quit();
        }
        if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
        {
            SDL_GL_DestroyContext(gl_context);
            SDL_DestroyWindow(window);
            SDL_Quit();
            throw std::runtime_error("Failed to initialize glad OpenGL.");
        }
        running = true;
        lastTime = SDL_GetPerformanceCounter();
    }

    ~Platform()
    {
        SDL_GL_DestroyContext(gl_context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    void PollEvent()
    {
        SDL_Event event;
        while (SDL_PollEvent(&event))
        {
            switch (event.type)
            {
            case SDL_EVENT_QUIT:
                running = false;
                break;

            case SDL_EVENT_WINDOW_RESIZED:
                if (callback)
                    callback(event.window.data1, event.window.data2); // width, height
                break;
            }
        }

        Uint64 currentTime = SDL_GetPerformanceCounter();
        Uint64 freq = SDL_GetPerformanceFrequency();
        deltaTime = static_cast<float>(currentTime - lastTime) / static_cast<float>(freq);
        lastTime = currentTime;
    }

    void SwapBuffer()
    {
        SDL_GL_SwapWindow(window);
    }

    bool ShouldClose()
    {
        return !running;
    }

    void SetCallback(std::function<void(int, int)> func)
    {
        callback = func;
    }
    float GetDeltaTime() const
    {
        return deltaTime;
    }

    SDL_Window *GetWindow() { return window; }

private:
    bool running = false;

    Uint64 lastTime = 0;
    float deltaTime = 0.0f;
    SDL_Window *window;
    SDL_GLContext gl_context;
    std::function<void(int, int)> callback;
};
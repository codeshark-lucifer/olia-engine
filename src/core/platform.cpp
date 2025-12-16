#include "engine/platform.hpp"
#include <stdexcept>

Platform::Platform()
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error("Failed initialize SDL3.");
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK,
                        SDL_GL_CONTEXT_PROFILE_CORE);

    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24); // 24-bit depth buffer
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);

    window = SDL_CreateWindow(
        APPLICATION_NAME,
        SCREEN_WIDTH, SCREEN_HEIGHT,
        SDL_WINDOW_RESIZABLE | SDL_WINDOW_OPENGL);

    if (!window)
    {
        throw std::runtime_error("Failed to create window.");
        SDL_Quit();
    }

    context = SDL_GL_CreateContext(window);

    if (!context)
    {
        throw std::runtime_error("SDL GL context failed: " + (std::string)SDL_GetError());
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        throw std::runtime_error("Failed initialize GLAD.");
        SDL_GL_DestroyContext(context);
        SDL_DestroyWindow(window);
        SDL_Quit();
    }

    SDL_GL_SetSwapInterval(1);
    glViewport(0, 0, SCREEN_WIDTH, SCREEN_HEIGHT);
    running = true;
}

Platform::~Platform()
{
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Platform::ShouldClose()
{
    return running == false;
}

void Platform::PollEvent()
{
    SDL_Event e;
    while (SDL_PollEvent(&e))
    {
        if (e.type == SDL_EVENT_QUIT)
            running = false;
        if (e.type == SDL_EVENT_WINDOW_RESIZED)
        {
            int width = e.window.data1;
            int height = e.window.data2;
            glViewport(0, 0, width, height);

            if (callback)
                callback(width, height);
        }
    }
}

void Platform::SwapBuffers()
{
    SDL_GL_SwapWindow(window);
}

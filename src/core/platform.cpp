#include <engine/window/platform.hpp>
#include <glad/glad.h> // Added for gladLoadGLLoader
#include <stdexcept>

Platform::Platform(const int &width, const int &height, const std::string &title)
{
    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        throw std::runtime_error("Failed to Initialize SDL3.");
    }
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MAJOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_MINOR_VERSION, 3);
    SDL_GL_SetAttribute(SDL_GL_CONTEXT_PROFILE_MASK, SDL_GL_CONTEXT_PROFILE_CORE);
    SDL_GL_SetAttribute(SDL_GL_DOUBLEBUFFER, 1);
    SDL_GL_SetAttribute(SDL_GL_DEPTH_SIZE, 24);

    this->window = SDL_CreateWindow(
        title.c_str(),
        width, height,
        SDL_WINDOW_OPENGL | SDL_WINDOW_RESIZABLE);

    if (!this->window)
    {
        SDL_Quit();
        throw std::runtime_error("Failed to create window.");
    }

    context = SDL_GL_CreateContext(this->window);
    if (!context)
    {
        SDL_DestroyWindow(window);
        SDL_Quit();
        throw std::runtime_error("Failed to create context SDL3.");
    }

    SDL_GL_MakeCurrent(this->window, context);
    SDL_GL_SetSwapInterval(1);

    if (!gladLoadGLLoader((GLADloadproc)SDL_GL_GetProcAddress))
    {
        throw std::runtime_error("Failed to initialize GLAD.");
    }
    
    glViewport(0, 0, width, height); // Set initial viewport
    
    running = true;
}

Platform::~Platform()
{
    SDL_GL_DestroyContext(context);
    SDL_DestroyWindow(window);
    SDL_Quit();
}

bool Platform::ShouldClose()
{
    return !running;
}

void Platform::PollEvent()
{
    SDL_Event event;
    while (SDL_PollEvent(&event))
    {
        if (event.type == SDL_EVENT_QUIT)
        {
            running = false;
        }
        else if (event.type == SDL_EVENT_WINDOW_PIXEL_SIZE_CHANGED)
        {
            int w = event.window.data1;
            int h = event.window.data2;
            if (callback)
                callback(w, h);
        }
    }
}

void Platform::SwapBuffer()
{
    SDL_GL_SwapWindow(window);
}

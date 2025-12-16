#pragma once
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <string>
#include <functional>
#include "config.h"

class Platform
{
public:
    Platform();
    virtual ~Platform();

    bool ShouldClose();
    void PollEvent();
    void SwapBuffers();

    std::function<void(int, int)> callback;

private:
    bool running = false;
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;
};
#pragma once
#include <SDL3/SDL.h>
#include <glad/glad.h>
#include <functional>
#include <string>

class Platform
{
public:
    Platform(const int &w, const int &h, const std::string &t);
    ~Platform();

    bool ShouldClose();
    void PollEvent();
    void SwapBuffers();

    std::function<void(int, int)> callback;

    SDL_Window *GetWindow() const { return window; }
    SDL_GLContext GetGLContext() const { return context; }
    void ProcessImGuiEvent(SDL_Event* event);

private:
    int width = 0, height = 0;
    const char *title = "engine";
    bool running = false;
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;
};
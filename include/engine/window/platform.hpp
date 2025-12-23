#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <functional>

class Platform
{
public:
    Platform(const int &width, const int &height,const std::string &title);
    ~Platform();

    bool ShouldClose();
    void PollEvent();
    void SwapBuffer();

    std::function<void(int, int)> callback;

private:
    bool running{false};
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;
};
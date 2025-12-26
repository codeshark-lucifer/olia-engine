#pragma once
#include <SDL3/SDL.h>
#include <string>
#include <functional>
#include <engine/singleton.hpp>

class Platform : public Singleton<Platform>
{
    friend class Singleton<Platform>;

public:
    void Init(int width, int height, const std::string &title);

    bool ShouldClose();
    void PollEvent();
    void SwapBuffer();

    std::function<void(int, int)> callback;
    SDL_Window *GetWindow() { return window; }

private:
    Platform() = default;
    ~Platform();

private:
    bool running{false};
    SDL_Window *window = nullptr;
    SDL_GLContext context = nullptr;
};
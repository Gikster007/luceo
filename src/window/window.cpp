#include "window.hpp"

#include <SDL3/SDL.h>

void Window::init() const
{
    SDL_SetAppMetadata("luceo", "0.1", "com.luceo.project");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        SDL_Log("Couldn't initialize SDL: %s", SDL_GetError());
        return;
    }

    if (!SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_VULKAN))
    {
        SDL_Log("Couldn't create window: %s", SDL_GetError());
        return;
    }
}

void Window::update()
{
    SDL_Event event{};

    while (SDL_PollEvent(&event))
    {
        switch (event.type)
        {
        case SDL_EVENT_QUIT:
            is_running = false;
            break;
        default:
            break;
        }
    }
}

Window::~Window()
{
    if (window && SDL_WasInit(SDL_INIT_VIDEO))
    {
        SDL_DestroyWindow(window);
    }
}
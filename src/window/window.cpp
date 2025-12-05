#include "window.hpp"

#include <SDL3/SDL.h>

#include <imgui_impl_sdl3.h>

void Window::init()
{
    SDL_SetAppMetadata("luceo", "0.1", "com.luceo.project");

    if (!SDL_Init(SDL_INIT_VIDEO))
    {
        printf("Couldn't initialize SDL: %s \n", SDL_GetError());
        return;
    }

    window = SDL_CreateWindow(title.data(), width, height, SDL_WINDOW_VULKAN);

    if (!window)
    {
        printf("Couldn't create window: %s \n", SDL_GetError());
        return;
    }
}

void Window::update()
{
    uint64_t now = SDL_GetTicks();
    if (now > last)
    {
        dt = ((float)now - last) / 1000.0f;
        last = now;
    }

    SDL_Event event{};

    while (SDL_PollEvent(&event))
    {
        ImGui_ImplSDL3_ProcessEvent(&event);
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

HWND Window::get_window_handle() const
{
    const SDL_PropertiesID props = SDL_GetWindowProperties(window);

    return (HWND)SDL_GetPointerProperty(props, SDL_PROP_WINDOW_WIN32_HWND_POINTER, nullptr);
}

Window::~Window()
{
    if (window && SDL_WasInit(SDL_INIT_VIDEO))
    {
        SDL_DestroyWindow(window);
    }
}
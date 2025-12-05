#pragma once

#include <string_view>

#define WIN32_LEAN_AND_MEAN
#include <Windows.h>
#undef min
#undef max

class SDL_Window;

class Window
{
  public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void init();
    void update();

    HWND get_window_handle() const;

  public:
    SDL_Window* window = nullptr;

    int width = 1280;
    int height = 720;
    std::string_view title{};
    bool is_running = true;

    float dt = 0.0f;
    float last = 0.0f;
};
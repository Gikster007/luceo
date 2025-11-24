#pragma once

#include <string_view>

class SDL_Window;

class Window
{
  public:
    Window() = default;
    ~Window();

    Window(const Window&) = delete;
    Window& operator=(const Window&) = delete;

    void init() const;
    void update();

  public:
    SDL_Window* window = nullptr;

    int width = 1280;
    int height = 720;
    std::string_view title{};
    bool is_running = true;
};
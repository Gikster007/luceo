#pragma once

#include <graphite/imgui.hh>
#include <graphite/resources/handle.hh>

class GPUAdapter;
class RenderGraph;
class Window;

class Renderer
{
  public:
    Renderer(Window& window);
    ~Renderer();

    Renderer(const Renderer&) = delete;
    Renderer& operator=(const Renderer&) = delete;

    void init();
    void update(float dt);
    void end();

    void set_imgui(ImGUI* imgui, ImGUIFunctions functions);

  private:
    Window& window;
    GPUAdapter& gpu;
    RenderGraph& render_graph;

    RenderTarget render_target{};
    Buffer vertex_buffer{};

    ImGUI* imgui = nullptr;
};
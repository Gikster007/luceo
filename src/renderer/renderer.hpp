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

  private:
    Window& window;
    GPUAdapter& gpu;
    RenderGraph& render_graph;

    RenderTarget render_target{};

    Texture test_tex{};
    Image test_img{};

    ImGUI imgui{};
};
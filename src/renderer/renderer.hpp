#pragma once

#include <graphite/imgui.hh>
#include <graphite/resources/handle.hh>

class GPUAdapter;
class RenderGraph;
class Window;

struct Data
{
    uint32_t flag;
};

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
    // Applies a Fast Fourier Transform to the Input and stores it in the Output.
    // For an Inverse FFT, set is_inverse to true
    void fft(Image input, Image output, bool is_inverse);

  private:
    Window& window;
    GPUAdapter& gpu;
    RenderGraph& render_graph;

    RenderTarget render_target{};

    Texture test_tex{};
    Image test_img{};

    // Used for packing channels
    Texture freq_rg_tex{};
    Image freq_rg_img{};
    Texture freq_b_tex{};
    Image freq_b_img{};

    Texture temp_tex{};
    Image temp_img{};

    Texture final_tex{};
    Image final_img{};

    Sampler linear_sampler{};

    ImGUI imgui{};
};
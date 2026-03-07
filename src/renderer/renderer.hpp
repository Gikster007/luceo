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

enum class FFTOption
{
    INVERSE,
    FORWARD
};

struct ComplexRGB
{
    Texture rg_tex{};
    Image rg_img{};

    Texture b_tex{};
    Image b_img{};
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
    /* Applies a Fast Fourier Transform to the Input and stores it in the Output. */
    void fft(Image image, Image temp, FFTOption option);

    /* Splits the Input Image Into 2 Textures (one holds RG and the other holds B) */
    void prepare_for_fft(Image input, ComplexRGB output);

  private:
    Window& window;
    GPUAdapter& gpu;
    RenderGraph& render_graph;

    RenderTarget render_target{};

    /* The Image We Apply the Kernel to (original source, so most likely stored as RGBA16Float or other) */
    /* Loaded by User */
    Texture input_tex{};
    Image input_img{};

    /* The Kernel Image That we Generate Based on User Inputs */
    Texture kernel_tex{};
    Image kernel_img{};

    /* The Input Image Transformed to Complex Format (FFT Ready) */
    ComplexRGB image;
    /* The Kernel Image Transformed to Complex Format (FFT Ready) */
    ComplexRGB kernel;

    /* Used for Ping-Pong When Performing FFT */
    Texture temp_tex{};
    Image temp_img{};

    /* Used in the Final Full-Screen Triangle Pass to Output to the Swapchain */
    Texture final_tex{};
    Image final_img{};

    Sampler linear_sampler{};

    ImGUI imgui{};
};
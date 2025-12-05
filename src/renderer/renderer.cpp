#include "renderer.hpp"

#include <graphite/vram_bank.hh>
#include <graphite/gpu_adapter.hh>
#include <graphite/render_graph.hh>
#include <graphite/nodes/raster_node.hh>
#include <graphite/nodes/compute_node.hh>

#include <glm/glm.hpp>

#define STB_IMAGE_IMPLEMENTATION
#include <stb_image.h>

#include <imgui.h>
#include <imgui_impl_sdl3.h>
#include <imgui_impl_vulkan.h>

#include "window/window.hpp"

Renderer::Renderer(Window& window)
    : window(window), gpu(*new GPUAdapter()), render_graph(*new RenderGraph())
{
}

Renderer::~Renderer()
{
    delete &render_graph;
    delete &gpu;
}

void Renderer::init()
{
    /* Initialize the GPU adapter */
    if (const Result r = gpu.init(true); r.is_err())
    {
        printf("failed to initialize gpu adapter.\nreason: %s \n", r.unwrap_err().c_str());
        return;
    }

    /* Initialize the Render Graph */
    render_graph.set_shader_path("assets/shaders/bin");
    render_graph.set_staging_limit(10000000u /* 10mb */);
    render_graph.set_max_graphs_in_flight(2u); /* Double buffering */
    if (const Result r = render_graph.init(gpu); r.is_err())
    {
        printf("failed to initialize render graph.\nreason: %s \n", r.unwrap_err().c_str());
        return;
    }

    VRAMBank& bank = gpu.get_vram_bank();

    /* Initialize the Render Target */
    const TargetDesc target{window.get_window_handle()};
    if (const Result r = bank.create_render_target(target); r.is_err())
    {
        printf("failed to initialize render target.\nreason: %s \n", r.unwrap_err().c_str());
        return;
    }
    else
        render_target = r.unwrap();

    /* Initialize ImGui */
    ImGui::CreateContext();
    ImGui_ImplSDL3_InitForVulkan(window.window);
    if (const Result r = imgui.init(gpu, render_target, IMGUI_FUNCTIONS); r.is_err())
    {
        printf("failed to initialize imgui.\nreason: %s \n", r.unwrap_err().c_str());
        return;
    }

    /* Load a test png */
    int tex_width = -1;
    int tex_height = -1;
    int channels = -1;
    unsigned char* data = nullptr;

    data = stbi_load("assets/test.jpg", &tex_width, &tex_height, &channels, 4);
    if (!data)
    {
        printf("failed to load image.\n");
        return;
    }

    /* Initialise a debug texture */
    if (const Result r =
            bank.create_texture(TextureUsage::Sampled | TextureUsage::TransferDst,
                                TextureFormat::RGBA8Unorm, {(u32)tex_width, (u32)tex_height, 0});
        r.is_err())
    {
        printf("failed to initialize debug texture.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        test_tex = r.unwrap();

    if (const Result r = bank.upload_texture(test_tex, data, tex_width * tex_height * channels);
        r.is_err())
    {
        printf("failed to upload the debug texture.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    free(data);

    if (const Result r = bank.create_image(test_tex); r.is_err())
    {
        printf("failed to initialize debug image.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        test_img = r.unwrap();
}

void Renderer::update(float dt)
{
    imgui.new_frame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    bool show_demo = true;
    ImGui::ShowDemoWindow(&show_demo);

    ImGui::Render();

    render_graph.new_graph().unwrap();

    // Render Passes
    // clang-format off
    {
        render_graph.add_compute_pass("Image Blit", "blit.cs")
                    .read(test_img)
                    .write(render_target)
                    .group_size(8, 8)
                    .work_size(window.width, window.height);
    }
    // clang-format on

    /* Add the immediate mode GUI to the render graph */
    render_graph.add_imgui(imgui, render_target);

    /* Compile the render graph */
    if (const Result r = render_graph.end_graph(); r.is_err())
        printf("failed to compile render graph.\nreason: %s \n", r.unwrap_err().c_str());
    /* Dispatch the render graph */
    if (const Result r = render_graph.dispatch(); r.is_err())
        printf("failed to dispatch render graph.\nreason: %s \n", r.unwrap_err().c_str());
}

void Renderer::end()
{
    VRAMBank& bank = gpu.get_vram_bank();
    bank.destroy(test_tex);
    bank.destroy(test_img);
    bank.destroy(render_target);

    /* Cleanup the VRAM bank & GPU adapter */
    render_graph.deinit().expect("failed to destroy render graph.");
    bank.deinit().expect("failed to destroy vram bank.");
    gpu.deinit().expect("failed to destroy gpu adapter.");
}

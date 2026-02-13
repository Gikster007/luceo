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
    gpu.set_logger();

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
    if (const Result r = imgui.init(gpu, render_target); r.is_err())
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
            bank.create_texture("Debug Texture", TextureUsage::Sampled | TextureUsage::TransferDst,
                                TextureFormat::RGBA8Unorm, {(u32)tex_width, (u32)tex_height, 0});
        r.is_err())
    {
        printf("failed to initialize debug texture.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        test_tex = r.unwrap();
    if (const Result r = bank.upload_texture(test_tex, data, tex_width * tex_height * 4);
        r.is_err())
    {
        printf("failed to upload the debug texture.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    free(data);

    /* Initialise a debug image, from the debug texture */
    if (const Result r = bank.create_image("Debug Image", test_tex); r.is_err())
    {
        printf("failed to initialize debug image.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        test_img = r.unwrap();

    /* Initialise a RG texture */
    if (const Result r = bank.create_texture("Frequency RG Texture",
                                             TextureUsage::Sampled | TextureUsage::Storage |
                                                 TextureUsage::TransferDst,
                                             TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0});
        r.is_err())
    {
        printf("failed to initialize rg texture.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        freq_rg_tex = r.unwrap();
    /* Initialise a RG image, from the RG texture */
    if (const Result r = bank.create_image("Frequency RG Image", freq_rg_tex); r.is_err())
    {
        printf("failed to initialize rg image.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        freq_rg_img = r.unwrap();

    /* Initialise a B texture */
    if (const Result r = bank.create_texture("Frequency RG Texture",
                                             TextureUsage::Sampled | TextureUsage::Storage |
                                                 TextureUsage::TransferDst,
                                             TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0});
        r.is_err())
    {
        printf("failed to initialize b texture.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        freq_b_tex = r.unwrap();
    /* Initialise a B image, from the B texture */
    if (const Result r = bank.create_image("Frequency RG Image", freq_b_tex); r.is_err())
    {
        printf("failed to initialize b image.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        freq_b_img = r.unwrap();

    /* Initialise the Temp Texture */
    if (const Result r = bank.create_texture("Temp Texture",
                                             TextureUsage::Sampled | TextureUsage::Storage |
                                                 TextureUsage::TransferDst,
                                             TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0});
        r.is_err())
    {
        printf("failed to initialize temp texture.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        temp_tex = r.unwrap();
    /* Initialise the temp image, from the temp texture */
    if (const Result r = bank.create_image("Temp Image", temp_tex); r.is_err())
    {
        printf("failed to initialize temp image.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        temp_img = r.unwrap();

    /* Initialise the Final Texture */
    if (const Result r = bank.create_texture("Final Texture",
                                             TextureUsage::Sampled | TextureUsage::Storage |
                                                 TextureUsage::TransferDst,
                                             TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0});
        r.is_err())
    {
        printf("failed to initialize final texture.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        final_tex = r.unwrap();
    /* Initialise the final image, from the final texture */
    if (const Result r = bank.create_image("Final Image", final_tex); r.is_err())
    {
        printf("failed to initialize final image.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        final_img = r.unwrap();

    /* Initialise a sampler */
    if (const Result r = bank.create_sampler("Linear Sampler"); r.is_err())
    {
        printf("failed to initialize linear sampler.\nreason: %s\n", r.unwrap_err().c_str());
        return;
    }
    else
        linear_sampler = r.unwrap();
}

static bool flag = true;

void Renderer::update(float dt)
{
    imgui.new_frame();
    ImGui_ImplSDL3_NewFrame();
    ImGui::NewFrame();

    bool show_metrics = true;
    ImGui::ShowMetricsWindow(&show_metrics);

    ImGui::Render();

    render_graph.new_graph().unwrap();

    // Render Passes
    // clang-format off
    {
        if (flag) 
        {
            render_graph.add_compute_pass("Prepare FFT Input", "prepare_fft.cs")
                        .read(test_img)
                        .write(freq_rg_img)
                        .write(freq_b_img)
                        .group_size(16, 16)
                        .work_size(512, 512);

            fft(freq_rg_img, temp_img, false);
            fft(freq_b_img, temp_img, false);
            flag = false;

            fft(freq_rg_img, temp_img, true);
            fft(freq_b_img, temp_img, true);

            render_graph.add_compute_pass("Recombine RGB", "recombine_rgb.cs")
                        .read(freq_rg_img)
                        .read(freq_b_img)
                        .write(final_img)
                        .group_size(16, 16)
                        .work_size(512, 512);
        }

        RasterNode& fs_pass = render_graph.add_raster_pass("Full Screen Triangle Pass", "fs_triangle.vx", "fs_triangle.px")
                                                .topology(Topology::TriangleList)
                                                .read(final_img, ShaderStages::Pixel)
                                                .read(linear_sampler, ShaderStages::Pixel)
                                                .attach(render_target)
                                                .raster_extent(window.width, window.height);
        fs_pass.draw(NULL_BUFFER, 3);
                    
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

void Renderer::fft(Image image, Image temp, bool is_inverse)
{
    const std::string_view pass_name = is_inverse ? "Inverse FFT" : "Forward FFT";

    VRAMBank& bank = gpu.get_vram_bank();

    Data data{};
    data.flag = uint32_t(is_inverse); // 1 is for Inverse FFT and 0 for Forward FFT

    // clang-format off
    render_graph.add_compute_pass(pass_name, "vertical_fft.cs")
                .read(image)
                .write(temp)
                .push_constants(&data, 0, sizeof(Data))
                .group_size(1, 1)
                .work_size(1, 512);

    render_graph.add_compute_pass(pass_name, "horizontal_fft.cs")
                .read(temp)
                .write(image)
                .push_constants(&data, 0, sizeof(Data))
                .group_size(1, 1)
                .work_size(1, 512);
    // clang-format on
}

void Renderer::end()
{
    VRAMBank& bank = gpu.get_vram_bank();
    bank.destroy(test_tex);
    bank.destroy(test_img);

    bank.destroy(freq_rg_tex);
    bank.destroy(freq_rg_img);
    bank.destroy(freq_b_tex);
    bank.destroy(freq_b_img);

    bank.destroy(temp_tex);
    bank.destroy(temp_img);

    bank.destroy(final_tex);
    bank.destroy(final_img);

    bank.destroy(linear_sampler);

    bank.destroy(render_target);

    imgui.deinit();

    /* Cleanup the VRAM bank & GPU adapter */
    render_graph.deinit().expect("failed to destroy render graph.");
    bank.deinit().expect("failed to destroy vram bank.");
    gpu.deinit().expect("failed to destroy gpu adapter.");
}

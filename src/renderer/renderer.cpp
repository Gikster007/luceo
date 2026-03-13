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
    gpu.set_max_textures(32u);
    gpu.set_max_images(32u);
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
    float* data = nullptr;

    data = stbi_loadf("assets/milan512.hdr", &tex_width, &tex_height, &channels, 4);
    if (!data)
    {
        printf("failed to load image.\n");
        return;
    }

    /* Initialise the Input Texture */
    {
        input_tex =
            bank.create_texture("Input Texture", TextureUsage::Sampled | TextureUsage::TransferDst,
                                TextureFormat::RGBA32Sfloat, {(u32)tex_width, (u32)tex_height, 0})
                .expect("failed to initialize input texture.");
        bank.upload_texture(input_tex, data, tex_width * tex_height * 4 * sizeof(float))
            .expect("failed to upload the input texture.");
        free(data);

        /* Initialise the Input Image */
        input_img =
            bank.create_image("Input Image", input_tex).expect("failed to initialize input image.");
    }

    /* Initialise the Kernel Texture */
    {
        aperture_tex =
            bank.create_texture("APerture Texture",
                                TextureUsage::Sampled | TextureUsage::Storage |
                                    TextureUsage::TransferDst,
                                TextureFormat::RGBA32Sfloat, {(u32)tex_width, (u32)tex_height, 0})
                .expect("failed to initialize aperture texture.");
        
        /* Initialise the Input Image */
        aperture_img = bank.create_image("APerture Image", aperture_tex)
                         .expect("failed to initialize aperture image.");
    }

    /* Initialise the Complex Image Texture (Complex Version of the Input Texture) */
    {
        /* RG Texture */
        image.rg_tex = bank.create_texture("Input RG Texture (Complex)",
                                           TextureUsage::Sampled | TextureUsage::Storage |
                                               TextureUsage::TransferDst,
                                           TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0})
                           .expect("failed to initialize input rg texture.");
        /* RG Image */
        image.rg_img = bank.create_image("Input RG Image (Complex)", image.rg_tex)
                           .expect("failed to initialize input rg image.");

        /* B Texture */
        image.b_tex = bank.create_texture("Input B Texture (Complex)",
                                          TextureUsage::Sampled | TextureUsage::Storage |
                                              TextureUsage::TransferDst,
                                          TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0})
                          .expect("failed to initialize input b texture.");
        /* B Image */
        image.b_img = bank.create_image("Input B Image (Complex)", image.b_tex)
                          .expect("failed to initialize input b image.");
    }

    /* Initialise the Complex Aperture Texture (Complex Version of the Aperture Texture) */
    {
        /* RG Texture */
        aperture.rg_tex = bank.create_texture("Aperture RG Texture (Complex)",
                                            TextureUsage::Sampled | TextureUsage::Storage |
                                                TextureUsage::TransferDst,
                                            TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0})
                            .expect("failed to initialize aperture rg texture.");
        /* RG Image */
        aperture.rg_img = bank.create_image("Aperture RG Image (Complex)", aperture.rg_tex)
                            .expect("failed to initialize aperture rg image.");

        /* B Texture */
        aperture.b_tex = bank.create_texture("Aperture B Texture (Complex)",
                                           TextureUsage::Sampled | TextureUsage::Storage |
                                               TextureUsage::TransferDst,
                                           TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0})
                           .expect("failed to initialize aperture b texture.");
        /* B Image */
        aperture.b_img = bank.create_image("Aperture B Image (Complex)", aperture.b_tex)
                           .expect("failed to initialize aperture b image.");
    }

    /* Initialise the Complex PSF Texture (Normalised Complex-Space Aperture) */
    {
        /* RG Texture */
        psf.rg_tex = bank.create_texture("Kernel RG Texture (Complex)",
                                         TextureUsage::Sampled | TextureUsage::Storage |
                                             TextureUsage::TransferDst,
                                         TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0})
                         .expect("failed to initialize psf rg texture.");
        /* RG Image */
        psf.rg_img = bank.create_image("Kernel RG Image (Complex)", psf.rg_tex)
                         .expect("failed to initialize psf rg image.");

        /* B Texture */
        psf.b_tex = bank.create_texture("Kernel B Texture (Complex)",
                                        TextureUsage::Sampled | TextureUsage::Storage |
                                            TextureUsage::TransferDst,
                                        TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0})
                        .expect("failed to initialize psf b texture.");
        /* B Image */
        psf.b_img = bank.create_image("Kernel B Image (Complex)", psf.b_tex)
                        .expect("failed to initialize psf b image.");
    }

    /* Initialise the Temp Texture */
    {
        temp_tex = bank.create_texture("Temp Texture",
                                       TextureUsage::Sampled | TextureUsage::Storage |
                                           TextureUsage::TransferDst,
                                       TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0})
                       .expect("failed to initialize temp texture.");

        /* Initialise the temp image, from the temp texture */
        temp_img =
            bank.create_image("Temp Image", temp_tex).expect("failed to initialise temp image");
    }

    /* Initialise the Final Texture */
    {
        final_tex = bank.create_texture("Final Texture",
                                        TextureUsage::Sampled | TextureUsage::Storage |
                                            TextureUsage::TransferDst,
                                        TextureFormat::RGBA32Sfloat, {(u32)512, (u32)512, 0})
                        .expect("failed to initialize final texture.");

        /* Initialise the final image, from the final texture */
        final_img =
            bank.create_image("Final Image", final_tex).expect("failed to initialize final image.");
    }

    /* Initialise a Linear Sampler */
    {
        linear_sampler =
            bank.create_sampler("Linear Sampler").expect("failed to initialize linear sampler.");
    }
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
            /* Generate Kernel */
            //render_graph.add_compute_pass("Generate Gaussian Kernel", "gen_gauss_kernel.cs")
            //            .write(kernel_img)
            //            .group_size(16, 16)
            //            .work_size(512, 512);
            render_graph.add_compute_pass("Generate Aperture Mask", "aperture_mask.cs")
                        .write(aperture_img)
                        .group_size(16, 16)
                        .work_size(512, 512);
            
            /* Prepare Complex Data for FFT */
            prepare_for_fft(input_img, image);
            prepare_for_fft(aperture_img, aperture);

            /* Bring Aperture Image to Freq Domain */
            fft(aperture.rg_img, temp_img, FFTOption::FORWARD);
            fft(aperture.b_img,  temp_img, FFTOption::FORWARD);

            /* Compute PSF */
            render_graph.add_compute_pass("Compute PSF RG", "compute_psf.cs")
            .read(aperture.rg_img)
            .write(psf.rg_img)
            .group_size(16, 16)
            .work_size(512, 512);

            render_graph.add_compute_pass("Compute PSF B", "compute_psf.cs")
            .read(aperture.b_img)
            .write(psf.b_img)
            .group_size(16, 16)
            .work_size(512, 512);

            /* Bring PSF Image to Freq Domain */
            fft(psf.rg_img, temp_img, FFTOption::FORWARD);
            fft(psf.b_img, temp_img, FFTOption::FORWARD);

            /* Bring Input Image to Freq Domain */
            fft(image.rg_img, temp_img, FFTOption::FORWARD);
            fft(image.b_img, temp_img, FFTOption::FORWARD);

            /* Multiply (in Freq Domain) */
            render_graph.add_compute_pass("Freq Multiply RG", "freq_multiply.cs")
                        .write(image.rg_img)
                        .read(psf.rg_img)
                        .group_size(16, 16)
                        .work_size(512, 512);
            render_graph.add_compute_pass("Freq Multiply B", "freq_multiply.cs")
                        .write(image.b_img)
                        .read(psf.b_img)
                        .group_size(16, 16)
                        .work_size(512, 512);

            /* Bring Input Image back to Spatial/Time Domain */
            fft(image.rg_img, temp_img, FFTOption::INVERSE);
            fft(image.b_img, temp_img, FFTOption::INVERSE);

            /* Combine RG and B Textures to the Final RGBA Texture */
            render_graph.add_compute_pass("Recombine RGB", "recombine_rgb.cs")
                        .read(image.rg_img)
                        .read(image.b_img)
                        .write(final_img)
                        .group_size(16, 16)
                        .work_size(512, 512);
            
            //flag = false;
        }

        /* Output Final Image to the Screen */
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

void Renderer::fft(Image image, Image temp, FFTOption option)
{
    const bool inverse = option == FFTOption::INVERSE ? true : false;
    const std::string_view pass_name = inverse ? "Inverse FFT" : "Forward FFT";

    VRAMBank& bank = gpu.get_vram_bank();

    Data data{};
    data.flag = (uint32_t)inverse; // 1 is for Inverse FFT and 0 for Forward FFT

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

void Renderer::prepare_for_fft(Image input, ComplexRGB output)
{
    render_graph.add_compute_pass("Prepare FFT Input", "prepare_fft.cs")
        .read(input)
        .write(output.rg_img)
        .write(output.b_img)
        .group_size(16, 16)
        .work_size(512, 512);
}

void Renderer::end()
{
    VRAMBank& bank = gpu.get_vram_bank();
    bank.destroy(input_tex);
    bank.destroy(input_img);
    bank.destroy(aperture_tex);
    bank.destroy(aperture_img);

    bank.destroy(image.rg_tex);
    bank.destroy(image.rg_img);
    bank.destroy(image.b_tex);
    bank.destroy(image.b_img);
    bank.destroy(aperture.rg_tex);
    bank.destroy(aperture.rg_img);
    bank.destroy(aperture.b_tex);
    bank.destroy(aperture.b_img);
    bank.destroy(psf.rg_tex);
    bank.destroy(psf.rg_img);
    bank.destroy(psf.b_tex);
    bank.destroy(psf.b_img);

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

#include "renderer.hpp"

#include <graphite/vram_bank.hh>
#include <graphite/gpu_adapter.hh>
#include <graphite/render_graph.hh>
#include <graphite/nodes/raster_node.hh>

#include <glm/glm.hpp>

#include "window/window.hpp"

struct Vertex
{
    glm::vec3 pos;
};

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

    /* Initialise a vertex buffer */
    if (const Result r = bank.create_buffer(BufferUsage::Vertex | BufferUsage::TransferDst, 1,
                                            sizeof(Vertex) * 3);
        r.is_err())
    {
        printf("failed to initialise constant buffer.\nreason: %s \n", r.unwrap_err().c_str());
        return;
    }
    else
        vertex_buffer = r.unwrap();

    std::vector<Vertex> vertices{};
    vertices.push_back({glm::vec3(-0.5f, 0.5f, 0.0f)});
    vertices.push_back({glm::vec3(0.0f, -0.5f, 0.0f)});
    vertices.push_back({glm::vec3(0.5f, 0.5f, 0.0f)});
    bank.upload_buffer(vertex_buffer, vertices.data(), 0, sizeof(Vertex) * 3);
}

void Renderer::update(float dt)
{
    render_graph.new_graph().unwrap();

    // Render Passes
    // clang-format off
    {
        // Test Triangle Pass
        RasterNode& graphics_pass = render_graph.add_raster_pass("graphics pass", "test.vx", "test.px")
                                    .topology(Topology::TriangleList)
                                    .attribute(AttrFormat::XYZ32_SFloat)  // Position
                                    .attach(render_target)
                                    .raster_extent(window.width, window.height);
        graphics_pass.draw(vertex_buffer, 3);
    }
    // clang-format on

    /* Add the immediate mode GUI to the render graph */
    if (imgui != nullptr)
        render_graph.add_imgui(*imgui, render_target);

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
    bank.destroy(vertex_buffer);
    bank.destroy(render_target);

    /* Cleanup the VRAM bank & GPU adapter */
    render_graph.deinit().expect("failed to destroy render graph.");
    bank.deinit().expect("failed to destroy vram bank.");
    gpu.deinit().expect("failed to destroy gpu adapter.");
}

void Renderer::set_imgui(ImGUI* imgui, ImGUIFunctions functions)
{
    this->imgui = imgui;
    /* Initialize the immediate mode GUI */
    if (const Result r = imgui->init(gpu, render_target, functions); r.is_err())
    {
        printf("failed to initialize imgui.\nreason: %s \n", r.unwrap_err().c_str());
        return;
    }
}

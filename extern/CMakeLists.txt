# SDL3 - https://github.com/libsdl-org/SDL
set(SDL_SHARED OFF)
set(SDL_STATIC ON)
set(SDL_TEST_LIBRARY OFF)
set(SDL_TESTS OFF)
set(SDL_DISABLE_INSTALL ON)
set(SDL_DISABLE_INSTALL_DOCS ON)
set(SDL_INSTALL_TESTS OFF)
add_subdirectory(SDL3)
#target_include_directories(luceo PRIVATE "./SDL3")

# graphite - https://github.com/mxcop/graphite
add_subdirectory(graphite)

# GLM - https://github.com/g-truc/glm
add_subdirectory(glm)

# STB Image - https://github.com/nothings/stb/blob/master/stb_image.h
target_include_directories(luceo PRIVATE "./stb/")

# ImGUI - immediate mode gui
    # https://github.com/ocornut/imgui
    target_sources(luceo
        PRIVATE "./imgui/imgui_demo.cpp"
        PRIVATE "./imgui/imgui_draw.cpp"
        PRIVATE "./imgui/imgui_tables.cpp"
        PRIVATE "./imgui/imgui_widgets.cpp"
        PRIVATE "./imgui/imgui.cpp"
        PRIVATE "./imgui/backends/imgui_impl_sdl3.cpp"
        PRIVATE "./imgui/backends/imgui_impl_vulkan.cpp"
        PRIVATE "./imgui/backends/imgui_impl_win32.cpp"
    )
    target_include_directories(luceo
        PRIVATE "./imgui"
        PRIVATE "./imgui/backends"
    )

    target_compile_definitions(luceo PRIVATE IMGUI_IMPL_VULKAN_USE_VOLK)


# Link Libraries
target_link_libraries(luceo PRIVATE SDL3::SDL3-static graphite glm::glm)
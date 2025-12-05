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

# Link Libraries
target_link_libraries(luceo PRIVATE SDL3::SDL3-static graphite)
set(SDL_SHARED OFF CACHE BOOL "" FORCE)
set(SDL_STATIC ON CACHE BOOL "" FORCE)

set(SDL_TESTS OFF CACHE BOOL "" FORCE)
set(SDL_TEST_LIBRARY OFF CACHE BOOL "" FORCE)
set(SDL_EXAMPLES OFF CACHE BOOL "" FORCE)

set(FT_DISABLE_ZLIB ON CACHE BOOL "" FORCE)
set(FT_DISABLE_BZIP2 ON CACHE BOOL "" FORCE)
set(FT_DISABLE_PNG ON CACHE BOOL "" FORCE)
set(FT_DISABLE_HARFBUZZ ON CACHE BOOL "" FORCE) # We link HB separately

add_subdirectory(freetype)
add_subdirectory(harfbuzz)

add_subdirectory(SDL)
add_subdirectory(vulkan)
add_subdirectory(glm)
add_subdirectory(stb)

add_library(vendor INTERFACE)
target_link_libraries(vendor
    INTERFACE
    SDL3::SDL3-static
    Vulkan::Vulkan
    glm
    stb
    freetype
    harfbuzz
)

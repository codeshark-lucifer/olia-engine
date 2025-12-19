cmake_minimum_required(VERSION 3.31.5)
project(olia LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

add_executable(olia
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/utils/glad.c
    ${SRC_DIR}/utils/stb_image.cpp
    ${SRC_DIR}/utils/engine.cpp

    ${SRC_DIR}/core/platform.cpp
    ${SRC_DIR}/core/shader.cpp
    ${SRC_DIR}/core/scene.cpp
    ${SRC_DIR}/core/transform.cpp

    ${SRC_DIR}/core/components/meshrenderer.cpp

)

target_include_directories(olia PRIVATE
    include
)

target_link_directories(olia PRIVATE
    lib/SDL3/x64
    lib
)

target_link_libraries(olia
    SDL3
    OpenGL::GL
    assimp
    ImGui # Link ImGui library
)

# ImGui
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third-party/imgui")

file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    # "${IMGUI_DIR}/imgui_demo.cpp" # Optional: remove if not needed for production
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_library(ImGui STATIC ${IMGUI_SOURCES}) # Create static library for ImGui

target_include_directories(ImGui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    include # Add project's main include directory for SDL3 headers
)

target_link_libraries(ImGui PUBLIC SDL3) # Link SDL3 to ImGui static library

add_custom_command(TARGET olia POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/lib/SDL3/x64/SDL3.dll"
        $<TARGET_FILE_DIR:olia>
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/lib/libassimp-6.dll"
        $<TARGET_FILE_DIR:olia>
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        $<TARGET_FILE_DIR:olia>/assets
)
cmake_minimum_required(VERSION 3.31.5)
project(OliaEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(OpenGL REQUIRED)

# -------------------------------
# Directories
# -------------------------------
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
set(INCLUDE_DIR "${CMAKE_SOURCE_DIR}/include")
set(IMGUI_DIR "${CMAKE_SOURCE_DIR}/third-party/imgui")
set(BIN_DIR "${CMAKE_BINARY_DIR}/bin")  # All outputs go here

# -------------------------------
# Collect engine sources automatically
# -------------------------------
file(GLOB_RECURSE ENGINE_SOURCES
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.c"
)

# -------------------------------
# Create engine static library
# -------------------------------
add_library(OliaEngine STATIC ${ENGINE_SOURCES})
target_include_directories(OliaEngine PUBLIC ${INCLUDE_DIR})

# Set output folder for library
set_target_properties(OliaEngine PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${BIN_DIR}"
)

# -------------------------------
# ImGui static library
# -------------------------------
file(GLOB IMGUI_SOURCES
    "${IMGUI_DIR}/imgui.cpp"
    "${IMGUI_DIR}/imgui_draw.cpp"
    "${IMGUI_DIR}/imgui_widgets.cpp"
    "${IMGUI_DIR}/imgui_tables.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_sdl3.cpp"
    "${IMGUI_DIR}/backends/imgui_impl_opengl3.cpp"
)

add_library(ImGui STATIC ${IMGUI_SOURCES})
target_include_directories(ImGui PUBLIC
    ${IMGUI_DIR}
    ${IMGUI_DIR}/backends
    ${INCLUDE_DIR}  # For SDL3 headers
)
target_link_libraries(ImGui PUBLIC SDL3)

# Output folder for ImGui
set_target_properties(ImGui PROPERTIES
    ARCHIVE_OUTPUT_DIRECTORY "${BIN_DIR}"
)

# -------------------------------
# Create executable
# -------------------------------
add_executable(engine "${SRC_DIR}/main.cpp")

# Include directories
target_include_directories(engine PRIVATE ${INCLUDE_DIR})

# Link engine, ImGui, SDL3, OpenGL, Assimp
target_link_directories(engine PRIVATE
    "${CMAKE_SOURCE_DIR}/lib/SDL3/x64"
    "${CMAKE_SOURCE_DIR}/lib"
)

target_link_libraries(engine
    PRIVATE OliaEngine
    ImGui
    SDL3
    OpenGL::GL
    assimp
)

# Output folder for executable
set_target_properties(engine PROPERTIES
    RUNTIME_OUTPUT_DIRECTORY "${BIN_DIR}"
)

# -------------------------------
# Post-build copy DLLs and assets
# -------------------------------
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/lib/SDL3/x64/SDL3.dll"
        "${BIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        "${CMAKE_SOURCE_DIR}/lib/libassimp-6.dll"
        "${BIN_DIR}"
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_SOURCE_DIR}/assets"
        "${BIN_DIR}/assets"
)

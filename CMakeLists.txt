cmake_minimum_required(VERSION 3.31.5)
project(olia LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)

find_package(OpenGL REQUIRED)
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")

add_executable(olia
    ${SRC_DIR}/main.cpp
    ${SRC_DIR}/utils/glad.c
    ${SRC_DIR}/utils/stb_image.cpp

    ${SRC_DIR}/core/platform.cpp
    ${SRC_DIR}/core/engine.cpp
    ${SRC_DIR}/core/scene.cpp
    ${SRC_DIR}/core/shader.cpp
    
    ${SRC_DIR}/core/entity.cpp
    ${SRC_DIR}/core/transform.cpp
    ${SRC_DIR}/core/meshrenderer.cpp
    ${SRC_DIR}/core/components/rotator.cpp
    ${SRC_DIR}/core/updatesystem.cpp
    ${SRC_DIR}/core/rendersystem.cpp
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
)

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
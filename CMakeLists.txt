cmake_minimum_required(VERSION 3.10)
project(PhysicsEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directory for binaries
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)

# Source files
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SRC_FILES
    "${SRC_DIR}/*.cpp"
    "${SRC_DIR}/*.c"
)

add_executable(engine ${SRC_FILES})

# Include directories
target_include_directories(engine PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/lib/SDL3/include
    ${CMAKE_SOURCE_DIR}/lib       # for Assimp headers
)

# Link directories
target_link_directories(engine PRIVATE
    ${CMAKE_SOURCE_DIR}/lib/SDL3/x64
    ${CMAKE_SOURCE_DIR}/lib       # For libassimp.dll.a
)

# Find OpenGL
find_package(OpenGL REQUIRED)
if(NOT OPENGL_FOUND)
    message(FATAL_ERROR "OpenGL not found!")
endif()
target_include_directories(engine PUBLIC ${OPENGL_INCLUDE_DIR})
target_link_libraries(engine
    ${OPENGL_gl_LIBRARY}   # Use the variable, not OpenGL::GL
    SDL3
    assimp
)

# Copy DLLs after build
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
        ${CMAKE_SOURCE_DIR}/lib/SDL3/x64/SDL3.dll
        ${CMAKE_SOURCE_DIR}/lib/libassimp-6.dll
        $<TARGET_FILE_DIR:engine>
)

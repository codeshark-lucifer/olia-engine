cmake_minimum_required(VERSION 3.10)
project(OliaEngine LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

# Output directories
set(CMAKE_RUNTIME_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/bin)   # for executables
set(CMAKE_ARCHIVE_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # for static libraries
set(CMAKE_LIBRARY_OUTPUT_DIRECTORY ${CMAKE_BINARY_DIR}/lib)   # for shared libs if needed

# Source files
set(SRC_DIR "${CMAKE_SOURCE_DIR}/src")
file(GLOB_RECURSE SRC_FILES "${SRC_DIR}/*.cpp" "${SRC_DIR}/*.c")

# -----------------------------
# 1️⃣ Build static library
# -----------------------------
add_library(lengine STATIC ${SRC_FILES})

# Include directories for library
target_include_directories(lengine PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/Bullet3
)

# Link libraries for static library (use full paths for MinGW)
target_link_libraries(lengine PUBLIC
    "${CMAKE_SOURCE_DIR}/lib/SDL3/x64/libSDL3.dll.a"   # generate if missing
    "${CMAKE_SOURCE_DIR}/lib/assimp/libassimp.dll.a"   # generate if missing
    "${CMAKE_SOURCE_DIR}/lib/Bullet3/libBulletDynamics.a"
    "${CMAKE_SOURCE_DIR}/lib/Bullet3/libBulletCollision.a"
    "${CMAKE_SOURCE_DIR}/lib/Bullet3/libBulletSoftBody.a"
    "${CMAKE_SOURCE_DIR}/lib/Bullet3/libLinearMath.a"
    ${OPENGL_gl_LIBRARY}
)

# -----------------------------
# 2️⃣ Build executable
# -----------------------------
add_executable(engine "${SRC_DIR}/main.cpp")  # only main.cpp

# Include directories for executable
target_include_directories(engine PUBLIC
    ${CMAKE_SOURCE_DIR}/include
    ${CMAKE_SOURCE_DIR}/include/Bullet3
)

# Link executable to the static library
target_link_libraries(engine PRIVATE lengine)

# Copy required DLLs after build
add_custom_command(TARGET engine POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_if_different
    "${CMAKE_SOURCE_DIR}/lib/SDL3/x64/SDL3.dll"
    "${CMAKE_SOURCE_DIR}/lib/assimp/libassimp-6.dll"
    $<TARGET_FILE_DIR:engine>
)

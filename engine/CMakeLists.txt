# Collect all source files
file(GLOB_RECURSE ENGINE_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

file(GLOB_RECURSE ENGINE_HEADERS CONFIGURE_DEPENDS
    include/*.h
    include/*.hpp
)

# Create the engine library
add_library(OliaEngine STATIC
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)

# Public include folder
target_include_directories(OliaEngine
PUBLIC
    ${CMAKE_CURRENT_SOURCE_DIR}/include
    ${CMAKE_CURRENT_SOURCE_DIR}/../vendor/bullet3/src
)

# Link third-party libraries
target_link_libraries(OliaEngine
PUBLIC
    glad
    glfw
    glm
    stb
    freetype
    harfbuzz
    BulletDynamics
    BulletCollision
    LinearMath
)

# Windows
if(WIN32)
    target_compile_definitions(OliaEngine PUBLIC OLIA_PLATFORM_WINDOWS)
endif()

# C++20
target_compile_features(OliaEngine PUBLIC cxx_std_20)

# Organize in Visual Studio
source_group(TREE ${CMAKE_CURRENT_SOURCE_DIR} FILES
    ${ENGINE_SOURCES}
    ${ENGINE_HEADERS}
)
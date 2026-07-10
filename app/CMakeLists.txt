file(GLOB_RECURSE APP_SOURCES CONFIGURE_DEPENDS
    src/*.cpp
)

add_executable(application
    ${APP_SOURCES}
)

target_include_directories(application
PRIVATE
    ${CMAKE_CURRENT_SOURCE_DIR}/include
)

target_link_libraries(application
PRIVATE
    OliaEngine
    winmm
)

# 1. Static Linking for Portability (removes reliance on external MinGW/MSYS2 DLLs)
if(WIN32)
    target_link_options(application PRIVATE -static -static-libgcc -static-libstdc++)
endif()

# 2. Post-Build: Automatically copy the assets directory to the executable output folder
add_custom_command(TARGET application POST_BUILD
    COMMAND ${CMAKE_COMMAND} -E copy_directory
        "${CMAKE_CURRENT_SOURCE_DIR}/../assets"
        "$<TARGET_FILE_DIR:application>/assets"
    COMMENT "Copying assets to target output directory..."
)

# 3. Packaging/Installation rules (for creating a standalone demo folder)
install(TARGETS application DESTINATION .)
install(DIRECTORY "${CMAKE_CURRENT_SOURCE_DIR}/../assets" DESTINATION .)



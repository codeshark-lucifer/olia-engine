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
)


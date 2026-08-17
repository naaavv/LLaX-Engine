file(GLOB_RECURSE ENGINE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image/*.h"
)

add_library(LLaXEngine STATIC ${ENGINE_SOURCES})

target_include_directories(LLaXEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor>
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/vendor/stb_image>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(LLaXEngine PUBLIC
    glfw
    glad
    glm::glm
    imgui_lib
    spdlog::spdlog
)

if (WIN32)
    target_link_libraries(LLaXEngine PUBLIC opengl32)
endif()
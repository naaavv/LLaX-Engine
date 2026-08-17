file(GLOB_RECURSE ENGINE_SOURCES 
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.cpp"
    "${CMAKE_CURRENT_SOURCE_DIR}/src/*.h"
)

add_library(LLaXEngine STATIC ${ENGINE_SOURCES})

target_include_directories(LLaXEngine PUBLIC
    $<BUILD_INTERFACE:${CMAKE_CURRENT_SOURCE_DIR}/src>
    $<INSTALL_INTERFACE:include>
)

target_link_libraries(LLaXEngine PUBLIC
    glfw
    glad
    glm::glm
    imgui_lib
    EnTT::EnTT
    assimp::assimp
    spdlog::spdlog
)

if (WIN32)
    target_link_libraries(LLaXEngine PUBLIC opengl32)
endif()
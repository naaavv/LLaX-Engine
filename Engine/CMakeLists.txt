set(ENGINE_SOURCES
    # Core
    src/Core/Base.h
    src/Core/Assert.h
    src/Core/Log.h
    src/Core/Log.cpp
    src/Core/Timestep.h
    src/Core/KeyCodes.h
    src/Core/MouseButtonCodes.h
    src/Core/Input.h
    src/Core/Input.cpp
    src/Core/Window.h
    src/Core/Window.cpp
    src/Core/Layer.h
    src/Core/Layer.cpp
    src/Core/LayerStack.h
    src/Core/LayerStack.cpp
    src/Core/Application.h
    src/Core/Application.cpp

    # Events
    src/Events/Event.h
    src/Events/ApplicationEvent.h
    src/Events/KeyEvent.h
    src/Events/MouseEvent.h

    # ImGui
    src/ImGui/ImGuiLayer.h
    src/ImGui/ImGuiLayer.cpp

    # Renderer
    src/Renderer/Renderer.h
    src/Renderer/Renderer.cpp

    # Master Header
    src/LLaX.h

    # Vendor
    vendor/stb_image/stb_image.h
    vendor/stb_image/stb_image.cpp
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
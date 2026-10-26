set(ENGINE_SOURCES
    # Core
    src/Core/Base.h
    src/Core/Assert.h
    src/Core/Log.h
    src/Core/Log.cpp
    src/Core/LogSink.h
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

    # Scene & ECS
    src/Scene/Components.h
    src/Scene/Entity.h
    src/Scene/Entity.cpp
    src/Scene/ScriptableEntity.h
    src/Scene/Scene.h
    src/Scene/Scene.cpp
    src/Scene/SceneSerializer.h
    src/Scene/SceneSerializer.cpp
    src/Scene/ParticleSystem.h
    src/Scene/ParticleSystem.cpp

    # Audio
    src/Audio/AudioSystem.h
    src/Audio/AudioSystem.cpp

    # Utilities & Platform
    src/Utils/PlatformUtils.h
    src/Platform/Windows/WindowsPlatformUtils.cpp

    # Platform OpenGL
    src/Platform/OpenGL/OpenGLContext.cpp
    src/Platform/OpenGL/OpenGLContext.h
    src/Platform/OpenGL/OpenGLBuffer.cpp
    src/Platform/OpenGL/OpenGLBuffer.h
    src/Platform/OpenGL/OpenGLVertexArray.cpp
    src/Platform/OpenGL/OpenGLVertexArray.h
    src/Platform/OpenGL/OpenGLShader.cpp
    src/Platform/OpenGL/OpenGLShader.h
    src/Platform/OpenGL/OpenGLTexture2D.cpp
    src/Platform/OpenGL/OpenGLTexture2D.h
    src/Platform/OpenGL/OpenGLFramebuffer.cpp
    src/Platform/OpenGL/OpenGLFramebuffer.h

    # Renderer
    src/Renderer/Renderer.h
    src/Renderer/Renderer.cpp
    src/Renderer/Renderer2D.h
    src/Renderer/Renderer2D.cpp
    src/Renderer/GraphicsContext.h
    src/Renderer/Buffer.h
    src/Renderer/Buffer.cpp
    src/Renderer/VertexArray.h
    src/Renderer/VertexArray.cpp
    src/Renderer/Shader.h
    src/Renderer/Shader.cpp
    src/Renderer/Texture.h
    src/Renderer/Texture.cpp
    src/Renderer/SubTexture2D.h
    src/Renderer/SubTexture2D.cpp
    src/Renderer/OrthographicCamera.h
    src/Renderer/OrthographicCamera.cpp
    src/Renderer/OrthographicCameraController.h
    src/Renderer/OrthographicCameraController.cpp
    src/Renderer/Framebuffer.h
    src/Renderer/Framebuffer.cpp


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
    EnTT
    yaml-cpp
    box2d
)

if (WIN32)
    target_link_libraries(LLaXEngine PUBLIC opengl32 comdlg32)
endif()
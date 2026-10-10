#pragma once

#include "Renderer/GraphicsContext.h"
#include <GLFW/glfw3.h>

namespace LLaX
{
    // OpenGL concrete implementation of GraphicsContext.
    // Responsible for:
    //   1. Making the GLFW window's GL context current (glfwMakeContextCurrent)
    //   2. Loading all OpenGL function pointers via GLAD
    //   3. Swapping the front/back buffers every frame (glfwSwapBuffers)
    class OpenGLContext : public GraphicsContext
    {
    public:
        explicit OpenGLContext(GLFWwindow* window);

        void Init()        override;
        void SwapBuffers() override;

    private:
        GLFWwindow* m_Window = nullptr;
    };
}

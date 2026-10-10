#include "OpenGLContext.h"
#include "Core/Log.h"
#include "Core/Assert.h"

// glad.h MUST come before glfw3.h.
// GLFW would otherwise include the legacy <GL/gl.h> first,
// which conflicts with glad's own OpenGL declarations.
#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace LLaX
{
    OpenGLContext::OpenGLContext(GLFWwindow* window)
        : m_Window(window)
    {
        LLAX_CORE_ASSERT(m_Window, "OpenGLContext: window handle is null!");
    }

    void OpenGLContext::Init()
    {
        // Step 1: Make this window's OpenGL context active on the current thread.
        // All subsequent OpenGL calls operate on this context.
        glfwMakeContextCurrent(m_Window);

        // Step 2: Load all OpenGL function pointers via GLAD.
        // glfwGetProcAddress returns the OS-level function pointer for each GL symbol.
        int status = gladLoadGLLoader(reinterpret_cast<GLADloadproc>(glfwGetProcAddress));
        LLAX_CORE_ASSERT(status, "Failed to initialize GLAD OpenGL loader!");

        // Log GPU info so we know exactly what hardware/driver we're running on.
        LLAX_CORE_INFO("OpenGL Context initialized:");
        LLAX_CORE_INFO("  Vendor:   {0}", reinterpret_cast<const char*>(glGetString(GL_VENDOR)));
        LLAX_CORE_INFO("  Renderer: {0}", reinterpret_cast<const char*>(glGetString(GL_RENDERER)));
        LLAX_CORE_INFO("  Version:  {0}", reinterpret_cast<const char*>(glGetString(GL_VERSION)));
    }

    void OpenGLContext::SwapBuffers()
    {
        // Swap the back buffer (what we just drew) with the front buffer (what's on screen).
        // This is what makes a rendered frame actually visible to the user.
        glfwSwapBuffers(m_Window);
    }
}

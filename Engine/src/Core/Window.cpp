#include "Window.h"
#include "Log.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

#include <glad/glad.h>
#include <GLFW/glfw3.h>

namespace LLaX
{
    static uint8_t s_GLFWWindowCount = 0;

    static void GLFWErrorCallback(int error, const char* description)
    {
        LLAX_CORE_ERROR("GLFW Error ({0}): {1}", error, description);
    }

    Window::Window(const WindowProps& props)
    {
        Init(props);
    }

    Window::~Window()
    {
        Shutdown();
    }

    void Window::Init(const WindowProps& props)
    {
        m_Data.Title = props.Title;
        m_Data.Width = props.Width;
        m_Data.Height = props.Height;

        LLAX_CORE_INFO("Creating window {0} ({1}, {2})", props.Title, props.Width, props.Height);

        if (s_GLFWWindowCount == 0)
        {
            int success = glfwInit();
            if (!success)
            {
                LLAX_CORE_CRITICAL("Could not initialize GLFW!");
                return;
            }
            glfwSetErrorCallback(GLFWErrorCallback);
        }

        // OpenGL context configuration
        glfwWindowHint(GLFW_CONTEXT_VERSION_MAJOR, 4);
        glfwWindowHint(GLFW_CONTEXT_VERSION_MINOR, 6);
        glfwWindowHint(GLFW_OPENGL_PROFILE, GLFW_OPENGL_CORE_PROFILE);

        m_Window = glfwCreateWindow((int)props.Width, (int)props.Height, m_Data.Title.c_str(), nullptr, nullptr);
        if (!m_Window)
        {
            LLAX_CORE_CRITICAL("Failed to create GLFW window!");
            return;
        }
        ++s_GLFWWindowCount;

        glfwMakeContextCurrent(m_Window);

        int status = gladLoadGLLoader((GLADloadproc)glfwGetProcAddress);
        if (!status)
        {
            LLAX_CORE_CRITICAL("Failed to initialize Glad OpenGL loader!");
        }
        else
        {
            LLAX_CORE_INFO("OpenGL Info:");
            LLAX_CORE_INFO("  Vendor:   {0}", (const char*)glGetString(GL_VENDOR));
            LLAX_CORE_INFO("  Renderer: {0}", (const char*)glGetString(GL_RENDERER));
            LLAX_CORE_INFO("  Version:  {0}", (const char*)glGetString(GL_VERSION));
        }

        glfwSetWindowUserPointer(m_Window, &m_Data);
        SetVSync(true);

        // GLFW Callbacks
        glfwSetWindowSizeCallback(m_Window, [](GLFWwindow* window, int width, int height)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            data.Width = width;
            data.Height = height;

            WindowResizeEvent event(width, height);
            if (data.EventCallback)
                data.EventCallback(event);
        });

        glfwSetWindowCloseCallback(m_Window, [](GLFWwindow* window)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            WindowCloseEvent event;
            if (data.EventCallback)
                data.EventCallback(event);
        });

        glfwSetKeyCallback(m_Window, [](GLFWwindow* window, int key, int scancode, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    KeyPressedEvent event(key, 0);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    KeyReleasedEvent event(key);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
                case GLFW_REPEAT:
                {
                    KeyPressedEvent event(key, 1);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetMouseButtonCallback(m_Window, [](GLFWwindow* window, int button, int action, int mods)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);

            switch (action)
            {
                case GLFW_PRESS:
                {
                    MouseButtonPressedEvent event(button);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
                case GLFW_RELEASE:
                {
                    MouseButtonReleasedEvent event(button);
                    if (data.EventCallback) data.EventCallback(event);
                    break;
                }
            }
        });

        glfwSetScrollCallback(m_Window, [](GLFWwindow* window, double xOffset, double yOffset)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            MouseScrolledEvent event((float)xOffset, (float)yOffset);
            if (data.EventCallback)
                data.EventCallback(event);
        });

        glfwSetCursorPosCallback(m_Window, [](GLFWwindow* window, double xPos, double yPos)
        {
            WindowData& data = *(WindowData*)glfwGetWindowUserPointer(window);
            MouseMovedEvent event((float)xPos, (float)yPos);
            if (data.EventCallback)
                data.EventCallback(event);
        });
    }

    void Window::Shutdown()
    {
        if (m_Window)
        {
            glfwDestroyWindow(m_Window);
            m_Window = nullptr;
            --s_GLFWWindowCount;
        }

        if (s_GLFWWindowCount == 0)
        {
            glfwTerminate();
        }
    }

    void Window::OnUpdate()
    {
        glfwPollEvents();
        glfwSwapBuffers(m_Window);
    }

    void Window::SetVSync(bool enabled)
    {
        if (enabled)
            glfwSwapInterval(1);
        else
            glfwSwapInterval(0);

        m_Data.VSync = enabled;
    }

    bool Window::IsVSync() const
    {
        return m_Data.VSync;
    }
}

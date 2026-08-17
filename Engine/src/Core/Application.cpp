#include "Application.h"
#include "Log.h"
#include "Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace LLaX
{
    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name)
    {
        s_Instance = this;

        Log::Init();
        LLAX_CORE_INFO("Initializing LLaX Engine...");

        m_Window = CreateScope<Window>(WindowProps(name));
        m_Window->SetEventCallback(LLAX_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        m_ImGuiLayer = CreateScope<ImGuiLayer>();
        m_ImGuiLayer->OnAttach();
    }

    Application::~Application()
    {
        m_ImGuiLayer->OnDetach();
        Renderer::Shutdown();
        LLAX_CORE_INFO("LLaX Engine terminated.");
    }

    void Application::Close()
    {
        m_Running = false;
    }

    void Application::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowCloseEvent>(LLAX_BIND_EVENT_FN(Application::OnWindowClose));
        dispatcher.Dispatch<WindowResizeEvent>(LLAX_BIND_EVENT_FN(Application::OnWindowResize));

        m_ImGuiLayer->OnEvent(e);
    }

    void Application::Run()
    {
        float lastFrameTime = (float)glfwGetTime();

        while (m_Running)
        {
            float time = (float)glfwGetTime();
            float timestep = time - lastFrameTime;
            lastFrameTime = time;

            if (!m_Minimized)
            {
                Renderer::SetClearColor({ 0.12f, 0.12f, 0.14f, 1.0f });
                Renderer::Clear();

                OnUpdate(timestep);

                m_ImGuiLayer->Begin();
                OnImGuiRender();
                m_ImGuiLayer->End();
            }

            m_Window->OnUpdate();
        }
    }

    bool Application::OnWindowClose(WindowCloseEvent& e)
    {
        m_Running = false;
        return true;
    }

    bool Application::OnWindowResize(WindowResizeEvent& e)
    {
        if (e.GetWidth() == 0 || e.GetHeight() == 0)
        {
            m_Minimized = true;
            return false;
        }

        m_Minimized = false;
        Renderer::SetViewport(0, 0, e.GetWidth(), e.GetHeight());

        return false;
    }
}

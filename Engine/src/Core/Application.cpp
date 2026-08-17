#include "Application.h"
#include "Log.h"
#include "Renderer/Renderer.h"

#include <GLFW/glfw3.h>

namespace LLaX
{
    Application* Application::s_Instance = nullptr;

    Application::Application(const std::string& name)
    {
        LLAX_CORE_ASSERT(!s_Instance, "Application already exists!");
        s_Instance = this;

        Log::Init();
        LLAX_CORE_INFO("Initializing LLaX Engine...");

        m_Window = CreateScope<Window>(WindowProps(name));
        m_Window->SetEventCallback(LLAX_BIND_EVENT_FN(Application::OnEvent));

        Renderer::Init();

        m_ImGuiLayer = new ImGuiLayer();
        PushOverlay(m_ImGuiLayer);
    }

    Application::~Application()
    {
        Renderer::Shutdown();
        LLAX_CORE_INFO("LLaX Engine terminated.");
    }

    void Application::PushLayer(Layer* layer)
    {
        m_LayerStack.PushLayer(layer);
    }

    void Application::PushOverlay(Layer* overlay)
    {
        m_LayerStack.PushOverlay(overlay);
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

        for (auto it = m_LayerStack.rbegin(); it != m_LayerStack.rend(); ++it)
        {
            if (e.Handled)
                break;
            (*it)->OnEvent(e);
        }
    }

    void Application::Run()
    {
        while (m_Running)
        {
            float time = (float)glfwGetTime();
            Timestep timestep = time - m_LastFrameTime;
            m_LastFrameTime = time;

            if (!m_Minimized)
            {
                Renderer::SetClearColor({ 0.12f, 0.12f, 0.14f, 1.0f });
                Renderer::Clear();

                for (Layer* layer : m_LayerStack)
                    layer->OnUpdate(timestep);

                m_ImGuiLayer->Begin();
                for (Layer* layer : m_LayerStack)
                    layer->OnImGuiRender();
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

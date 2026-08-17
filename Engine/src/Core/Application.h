#pragma once

#include "Core/Base.h"
#include "Core/Window.h"
#include "Core/LayerStack.h"
#include "Core/Timestep.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "ImGui/ImGuiLayer.h"

namespace LLaX
{
    class Application
    {
    public:
        Application(const std::string& name = "LLaX Engine App");
        virtual ~Application();

        void Run();
        void OnEvent(Event& e);

        void PushLayer(Layer* layer);
        void PushOverlay(Layer* overlay);

        void Close();

        inline Window& GetWindow() { return *m_Window; }
        inline ImGuiLayer* GetImGuiLayer() { return m_ImGuiLayer; }
        inline static Application& Get() { return *s_Instance; }

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:
        Scope<Window> m_Window;
        ImGuiLayer* m_ImGuiLayer;
        LayerStack m_LayerStack;
        bool m_Running = true;
        bool m_Minimized = false;
        float m_LastFrameTime = 0.0f;

        static Application* s_Instance;
    };
}

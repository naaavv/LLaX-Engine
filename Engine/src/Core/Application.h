#pragma once

#include "Core/Base.h"
#include "Core/Window.h"
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

        void Close();

        inline Window& GetWindow() { return *m_Window; }
        inline static Application& Get() { return *s_Instance; }

    protected:
        virtual void OnUpdate(float ts) {}
        virtual void OnImGuiRender() {}

    private:
        bool OnWindowClose(WindowCloseEvent& e);
        bool OnWindowResize(WindowResizeEvent& e);

    private:
        Scope<Window> m_Window;
        Scope<ImGuiLayer> m_ImGuiLayer;
        bool m_Running = true;
        bool m_Minimized = false;

        static Application* s_Instance;
    };
}

#pragma once

#include "Base.h"
#include "Events/Event.h"
#include <string>
#include <functional>

struct GLFWwindow;

namespace LLaX
{
    struct WindowProps
    {
        std::string Title;
        unsigned int Width;
        unsigned int Height;

        WindowProps(const std::string& title = "LLaX Engine",
                    unsigned int width = 1280,
                    unsigned int height = 720)
            : Title(title), Width(width), Height(height)
        {
        }
    };

    class Window
    {
    public:
        using EventCallbackFn = std::function<void(Event&)>;

        Window(const WindowProps& props = WindowProps());
        ~Window();

        void OnUpdate();

        inline unsigned int GetWidth() const { return m_Data.Width; }
        inline unsigned int GetHeight() const { return m_Data.Height; }

        inline void SetEventCallback(const EventCallbackFn& callback) { m_Data.EventCallback = callback; }
        void SetVSync(bool enabled);
        bool IsVSync() const;

        inline GLFWwindow* GetNativeWindow() const { return m_Window; }

    private:
        void Init(const WindowProps& props);
        void Shutdown();

    private:
        GLFWwindow* m_Window = nullptr;

        struct WindowData
        {
            std::string Title;
            unsigned int Width, Height;
            bool VSync;

            EventCallbackFn EventCallback;
        };

        WindowData m_Data;
    };
}

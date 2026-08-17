#pragma once

#include "Core/Base.h"
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

namespace LLaX
{
    class ImGuiLayer
    {
    public:
        ImGuiLayer();
        ~ImGuiLayer();

        void OnAttach();
        void OnDetach();
        void OnEvent(Event& event);

        void Begin();
        void End();

        void BlockEvents(bool block) { m_BlockEvents = block; }

    private:
        bool m_BlockEvents = true;
    };
}

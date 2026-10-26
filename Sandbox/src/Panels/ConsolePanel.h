#pragma once

#include <LLaX.h>
#include <string>

namespace LLaX
{
    class ConsolePanel
    {
    public:
        ConsolePanel();
        ~ConsolePanel() = default;

        void OnImGuiRender();

    private:
        bool m_AutoScroll = true;
        bool m_ShowTrace = true;
        bool m_ShowInfo = true;
        bool m_ShowWarn = true;
        bool m_ShowError = true;
        char m_FilterBuffer[256] = { 0 };
    };
}

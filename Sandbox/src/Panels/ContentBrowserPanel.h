#pragma once

#include <LLaX.h>
#include <filesystem>
#include <vector>
#include <algorithm>

namespace LLaX
{
    class ContentBrowserPanel
    {
    public:
        ContentBrowserPanel();

        void OnImGuiRender();

    private:
        std::filesystem::path m_BaseDirectory;
        std::filesystem::path m_CurrentDirectory;
        float m_Padding = 16.0f;
        float m_ThumbnailSize = 72.0f;
    };
}

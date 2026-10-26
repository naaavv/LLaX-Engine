#include "ConsolePanel.h"
#include <algorithm>

namespace LLaX
{
    ConsolePanel::ConsolePanel()
    {
    }

    void ConsolePanel::OnImGuiRender()
    {
        ImGui::Begin("Console");

        // Top Toolbar inside Console
        if (ImGui::Button("Clear"))
        {
            EditorLogSink::Clear();
        }

        ImGui::SameLine();
        ImGui::Checkbox("Auto-scroll", &m_AutoScroll);

        ImGui::SameLine();
        ImGui::SetNextItemWidth(180.0f);
        ImGui::InputTextWithHint("##Filter", "Search log...", m_FilterBuffer, sizeof(m_FilterBuffer));

        const auto& messages = EditorLogSink::GetMessages();

        // Count messages per category
        uint32_t traceCount = 0, infoCount = 0, warnCount = 0, errorCount = 0;
        for (const auto& msg : messages)
        {
            switch (msg.Level)
            {
                case spdlog::level::trace:
                case spdlog::level::debug: traceCount++; break;
                case spdlog::level::info:  infoCount++; break;
                case spdlog::level::warn:  warnCount++; break;
                case spdlog::level::err:
                case spdlog::level::critical: errorCount++; break;
                default: break;
            }
        }

        ImGui::SameLine();
        ImGui::Spacing();
        ImGui::SameLine();

        // Filter toggles
        auto drawToggle = [](const char* label, bool* state, uint32_t count, const ImVec4& color) {
            std::string text = std::string(label) + " (" + std::to_string(count) + ")";
            if (*state)
            {
                ImGui::PushStyleColor(ImGuiCol_Button, color);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
            }
            if (ImGui::Button(text.c_str()))
            {
                *state = !*state;
            }
            if (*state)
            {
                ImGui::PopStyleColor(2);
            }
            ImGui::SameLine();
        };

        drawToggle("Info", &m_ShowInfo, infoCount, ImVec4(0.15f, 0.45f, 0.25f, 1.0f));
        drawToggle("Warn", &m_ShowWarn, warnCount, ImVec4(0.55f, 0.45f, 0.10f, 1.0f));
        drawToggle("Error", &m_ShowError, errorCount, ImVec4(0.60f, 0.20f, 0.20f, 1.0f));
        drawToggle("Trace", &m_ShowTrace, traceCount, ImVec4(0.30f, 0.30f, 0.30f, 1.0f));

        ImGui::NewLine();
        ImGui::Separator();

        // Message List
        ImGui::BeginChild("ScrollingRegion", ImVec2(0, 0), false, ImGuiWindowFlags_HorizontalScrollbar);

        std::string filterStr = m_FilterBuffer;
        std::transform(filterStr.begin(), filterStr.end(), filterStr.begin(), ::tolower);

        for (const auto& msg : messages)
        {
            bool show = false;
            ImVec4 levelColor = ImVec4(0.85f, 0.85f, 0.85f, 1.0f);
            const char* levelStr = "INFO";

            switch (msg.Level)
            {
                case spdlog::level::trace:
                case spdlog::level::debug:
                    show = m_ShowTrace;
                    levelColor = ImVec4(0.55f, 0.55f, 0.55f, 1.0f);
                    levelStr = "TRACE";
                    break;
                case spdlog::level::info:
                    show = m_ShowInfo;
                    levelColor = ImVec4(0.35f, 0.85f, 0.45f, 1.0f);
                    levelStr = "INFO";
                    break;
                case spdlog::level::warn:
                    show = m_ShowWarn;
                    levelColor = ImVec4(1.0f, 0.82f, 0.20f, 1.0f);
                    levelStr = "WARN";
                    break;
                case spdlog::level::err:
                case spdlog::level::critical:
                    show = m_ShowError;
                    levelColor = ImVec4(0.95f, 0.30f, 0.30f, 1.0f);
                    levelStr = "ERROR";
                    break;
                default:
                    show = true;
                    break;
            }

            if (!show)
                continue;

            // Search filter check
            if (!filterStr.empty())
            {
                std::string lowerMsg = msg.Message;
                std::transform(lowerMsg.begin(), lowerMsg.end(), lowerMsg.begin(), ::tolower);
                if (lowerMsg.find(filterStr) == std::string::npos)
                    continue;
            }

            // Print item
            ImGui::TextDisabled("[%s]", msg.Timestamp.c_str());
            ImGui::SameLine();
            ImGui::TextColored(levelColor, "[%-5s]", levelStr);
            ImGui::SameLine();
            ImGui::TextDisabled("[%s]", msg.LoggerName.c_str());
            ImGui::SameLine();
            ImGui::TextUnformatted(msg.Message.c_str());
        }

        if (m_AutoScroll && ImGui::GetScrollY() >= ImGui::GetScrollMaxY())
            ImGui::SetScrollHereY(1.0f);

        ImGui::EndChild();
        ImGui::End();
    }
}

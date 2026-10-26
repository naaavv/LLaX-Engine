#include "ContentBrowserPanel.h"

namespace LLaX
{
    ContentBrowserPanel::ContentBrowserPanel()
        : m_BaseDirectory("Assets"), m_CurrentDirectory(m_BaseDirectory)
    {
        if (!std::filesystem::exists(m_BaseDirectory))
        {
            std::filesystem::create_directories(m_BaseDirectory);
        }
    }

    void ContentBrowserPanel::OnImGuiRender()
    {
        ImGui::Begin("Content Browser");

        // ── Navigation / breadcrumb bar ──────────────────────────────────────
        bool canGoBack = m_CurrentDirectory != m_BaseDirectory;

        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.17f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.36f, 0.68f, 1.0f));

        if (!canGoBack) ImGui::BeginDisabled();
        if (ImGui::Button("  <  "))
            m_CurrentDirectory = m_CurrentDirectory.parent_path();
        if (!canGoBack) ImGui::EndDisabled();

        ImGui::SameLine();
        if (ImGui::Button(" Home "))
            m_CurrentDirectory = m_BaseDirectory;

        ImGui::PopStyleColor(2);

        // Breadcrumb path
        ImGui::SameLine();
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.42f, 0.66f, 1.0f, 1.0f));
        // Show relative path from base
        auto rel = std::filesystem::relative(m_CurrentDirectory, m_BaseDirectory.parent_path());
        ImGui::Text("  %s", rel.generic_string().c_str());
        ImGui::PopStyleColor();

        ImGui::Separator();

        // ── Thumbnail size slider ─────────────────────────────────────────────
        {
            ImGui::PushItemWidth(100.0f);
            ImGui::SetNextItemWidth(100.0f);
            float sz = m_ThumbnailSize;
            if (ImGui::SliderFloat("##thumbsz", &sz, 60.0f, 160.0f, "%.0f px"))
                m_ThumbnailSize = sz;
            ImGui::PopItemWidth();
            ImGui::SameLine();
            ImGui::TextDisabled("Thumbnail size");
        }

        ImGui::Separator();
        ImGui::Spacing();

        // ── Grid layout ───────────────────────────────────────────────────────
        float cellSize   = m_ThumbnailSize + m_Padding;
        float panelWidth = ImGui::GetContentRegionAvail().x;
        int   colCount   = std::max(1, (int)(panelWidth / cellSize));

        if (ImGui::BeginTable("ContentBrowserTable", colCount))
        {
            if (std::filesystem::exists(m_CurrentDirectory))
            {
                // Sort: directories first
                std::vector<std::filesystem::directory_entry> entries;
                for (auto& e : std::filesystem::directory_iterator(m_CurrentDirectory))
                    entries.push_back(e);
                std::sort(entries.begin(), entries.end(), [](const auto& a, const auto& b) {
                    if (a.is_directory() != b.is_directory())
                        return a.is_directory() > b.is_directory();
                    return a.path().filename() < b.path().filename();
                });

                for (auto& directoryEntry : entries)
                {
                    const auto& path    = directoryEntry.path();
                    std::string fname   = path.filename().string();
                    bool isDirectory    = directoryEntry.is_directory();

                    ImGui::TableNextColumn();
                    ImGui::PushID(fname.c_str());

                    // ── Colour + emoji badge per file type ────────────────────
                    const char* badge    = "[?]";
                    ImVec4      btnColor = ImVec4(0.18f, 0.19f, 0.22f, 1.0f);
                    ImVec4      badgeCol = ImVec4(0.60f, 0.62f, 0.68f, 1.0f);

                    if (isDirectory)
                    {
                        badge    = "[DIR]";
                        btnColor = ImVec4(0.20f, 0.28f, 0.42f, 1.0f);
                        badgeCol = ImVec4(0.55f, 0.78f, 1.00f, 1.0f);
                    }
                    else
                    {
                        std::string ext = path.extension().string();
                        if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                        {
                            badge    = "[TEX]";
                            btnColor = ImVec4(0.28f, 0.18f, 0.38f, 1.0f);
                            badgeCol = ImVec4(0.85f, 0.58f, 1.00f, 1.0f);
                        }
                        else if (ext == ".glsl" || ext == ".shader" || ext == ".vert" || ext == ".frag")
                        {
                            badge    = "[SHD]";
                            btnColor = ImVec4(0.35f, 0.24f, 0.10f, 1.0f);
                            badgeCol = ImVec4(1.00f, 0.72f, 0.28f, 1.0f);
                        }
                        else if (ext == ".llax")
                        {
                            badge    = "[SCN]";
                            btnColor = ImVec4(0.10f, 0.30f, 0.38f, 1.0f);
                            badgeCol = ImVec4(0.28f, 0.88f, 0.90f, 1.0f);
                        }
                        else if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac")
                        {
                            badge    = "[SND]";
                            btnColor = ImVec4(0.40f, 0.15f, 0.25f, 1.0f);
                            badgeCol = ImVec4(1.00f, 0.40f, 0.65f, 1.0f);
                        }
                        else if (ext == ".lua" || ext == ".cs")
                        {
                            badge    = "[SCR]";
                            btnColor = ImVec4(0.10f, 0.32f, 0.18f, 1.0f);
                            badgeCol = ImVec4(0.40f, 0.98f, 0.55f, 1.0f);
                        }
                        else
                        {
                            badge    = "[FILE]";
                            btnColor = ImVec4(0.16f, 0.17f, 0.20f, 1.0f);
                            badgeCol = ImVec4(0.50f, 0.52f, 0.58f, 1.0f);
                        }
                    }

                    // Draw item button
                    ImGui::PushStyleColor(ImGuiCol_Button,        btnColor);
                    ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(btnColor.x + 0.08f, btnColor.y + 0.08f, btnColor.z + 0.12f, 1.0f));
                    ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(btnColor.x - 0.04f, btnColor.y - 0.04f, btnColor.z - 0.04f, 1.0f));
                    ImGui::Button("##item", { m_ThumbnailSize, m_ThumbnailSize });
                    ImGui::PopStyleColor(3);

                    // Badge label centred over button
                    {
                        ImVec2 bMin = ImGui::GetItemRectMin();
                        ImVec2 bMax = ImGui::GetItemRectMax();
                        ImVec2 badgeTextSz = ImGui::CalcTextSize(badge);
                        ImVec2 badgePos = {
                            bMin.x + (bMax.x - bMin.x - badgeTextSz.x) * 0.5f,
                            bMin.y + (bMax.y - bMin.y) * 0.30f
                        };
                        ImGui::GetWindowDrawList()->AddText(badgePos, ImGui::ColorConvertFloat4ToU32(badgeCol), badge);

                        // Filename centred at bottom portion
                        std::string shortName = fname.size() > 14 ? fname.substr(0, 12) + ".." : fname;
                        ImVec2 nameSz = ImGui::CalcTextSize(shortName.c_str());
                        ImVec2 namePos = {
                            bMin.x + (bMax.x - bMin.x - nameSz.x) * 0.5f,
                            bMin.y + (bMax.y - bMin.y) * 0.62f
                        };
                        ImGui::GetWindowDrawList()->AddText(namePos, IM_COL32(200, 202, 212, 255), shortName.c_str());
                    }

                    // Drag and Drop source
                    if (ImGui::BeginDragDropSource())
                    {
                        std::string itemPath = path.generic_string();
                        ImGui::SetDragDropPayload("CONTENT_BROWSER_ITEM", itemPath.c_str(), itemPath.size() + 1);
                        ImGui::PushStyleColor(ImGuiCol_Text, badgeCol);
                        ImGui::Text("%s  %s", badge, fname.c_str());
                        ImGui::PopStyleColor();
                        ImGui::EndDragDropSource();
                    }

                    // Double click: enter directory
                    if (ImGui::IsItemHovered() && ImGui::IsMouseDoubleClicked(ImGuiMouseButton_Left))
                    {
                        if (isDirectory)
                            m_CurrentDirectory /= path.filename();
                    }

                    // Tooltip
                    if (ImGui::IsItemHovered(ImGuiHoveredFlags_DelayShort))
                    {
                        ImGui::BeginTooltip();
                        ImGui::PushStyleColor(ImGuiCol_Text, badgeCol);
                        ImGui::Text("%s", badge);
                        ImGui::PopStyleColor();
                        ImGui::SameLine();
                        ImGui::Text("%s", fname.c_str());
                        if (!isDirectory)
                        {
                            uintmax_t sz = directoryEntry.file_size();
                            ImGui::TextDisabled("%.1f KB", sz / 1024.0f);
                        }
                        ImGui::EndTooltip();
                    }

                    ImGui::PopID();
                }
            }

            ImGui::EndTable();
        }

        ImGui::End();
    }
}

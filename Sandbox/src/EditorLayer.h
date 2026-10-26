#pragma once

#include <LLaX.h>
#include "Panels/SceneHierarchyPanel.h"
#include "Panels/ContentBrowserPanel.h"
#include "Panels/ConsolePanel.h"
#include <filesystem>

namespace LLaX
{
    class EditorLayer : public Layer
    {
    public:
        enum class SceneState
        {
            Edit = 0,
            Play = 1,
            Pause = 2
        };

        EditorLayer();
        ~EditorLayer() override = default;

        void OnAttach() override;
        void OnDetach() override;

        void OnUpdate(Timestep ts) override;
        void OnImGuiRender() override;
        void OnEvent(Event& e) override;

    private:
        bool OnKeyPressed(KeyPressedEvent& e);
        bool OnMouseButtonPressed(MouseButtonPressedEvent& e);

        void NewScene();
        void OpenScene();
        void OpenScene(const std::filesystem::path& path);
        void SaveScene();
        void SaveSceneAs();
        void ExportProject();
        void RunStandalone();

        void OnScenePlay();
        void OnSceneStop();
        void OnScenePause();

        void UI_Toolbar();
        void UI_Stats();
        void UI_AboutModal();

        void SetDarkThemeColors();

    private:
        OrthographicCameraController m_CameraController;

        Ref<Framebuffer> m_Framebuffer;
        Ref<Scene> m_ActiveScene;
        Ref<Scene> m_EditorScene;
        std::filesystem::path m_EditorScenePath;

        Ref<Texture2D> m_CheckerboardTexture;

        glm::vec2 m_ViewportSize{ 1280.0f, 720.0f };
        glm::vec2 m_ViewportBounds[2];
        bool m_ViewportFocused = false;
        bool m_ViewportHovered = false;

        SceneState m_SceneState = SceneState::Edit;

        // Panels
        SceneHierarchyPanel m_SceneHierarchyPanel;
        ContentBrowserPanel m_ContentBrowserPanel;
        ConsolePanel m_ConsolePanel;

        // Window visibility toggles
        bool m_ShowSceneHierarchy = true;
        bool m_ShowProperties = true;
        bool m_ShowContentBrowser = true;
        bool m_ShowConsole = true;
        bool m_ShowStats = true;
        bool m_ShowViewport = true;
        bool m_ShowAboutModal = false;

        // Stats & Viewport settings
        float m_FPS = 0.0f;
        float m_FrameTime = 0.0f;
        glm::vec4 m_ClearColor{ 0.192f, 0.212f, 0.231f, 1.0f }; // Unity style neutral dark gray viewport background
        bool m_ShowGrid = true;
        bool m_ShowPhysicsColliders = true;
        int m_GizmoType = 0; // 0: Select, 1: Translate, 2: Rotate, 3: Scale
        Entity m_HoveredEntity;
    };
}

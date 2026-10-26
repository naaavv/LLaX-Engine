#include "EditorLayer.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <ImGuizmo.h>
#include <filesystem>

// ── Lightweight text "icons" (no font atlas needed) ────────────────────────
#define ICON_NEW    "[+]"
#define ICON_OPEN   "[O]"
#define ICON_SAVE   "[S]"
#define ICON_SAVEAS "[A]"

namespace LLaX
{
    EditorLayer::EditorLayer()
        : Layer("EditorLayer"),
          m_CameraController(1280.0f / 720.0f, true)
    {
    }

    void EditorLayer::OnAttach()
    {
        LLAX_INFO("=================================================");
        LLAX_INFO("  LLaX Studio (Phase 3 Game Engine Editor) Loaded ");
        LLAX_INFO("=================================================");

        SetDarkThemeColors();

        // 1. Framebuffer setup (off-screen render target)
        FramebufferSpecification fbSpec;
        fbSpec.Attachments = { FramebufferTextureFormat::RGBA8, FramebufferTextureFormat::RED_INTEGER, FramebufferTextureFormat::DEPTH24STENCIL8 };
        fbSpec.Width = 1280;
        fbSpec.Height = 720;
        m_Framebuffer = Framebuffer::Create(fbSpec);

        // 2. Procedural 64x64 Checkerboard Texture
        m_CheckerboardTexture = Texture2D::Create(64, 64);
        uint32_t checkerboardData[64 * 64];
        for (int y = 0; y < 64; y++)
        {
            for (int x = 0; x < 64; x++)
            {
                checkerboardData[y * 64 + x] = ((x / 8) % 2 == (y / 8) % 2) ? 0xffffffff : 0xff202020;
            }
        }
        m_CheckerboardTexture->SetData(checkerboardData, sizeof(checkerboardData));

        // 3. Create Scene & Default Entities
        NewScene();
    }

    void EditorLayer::OnDetach()
    {
        LLAX_INFO("LLaX Studio detached.");
    }

    void EditorLayer::NewScene()
    {
        m_ActiveScene = CreateRef<Scene>();
        m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

        // Entity 1: Background Quad with Checkerboard (subtle Unity background floor)
        auto bg = m_ActiveScene->CreateEntity("Background");
        bg.GetComponent<TransformComponent>().Translation = { 0.0f, 0.0f, -0.95f };
        bg.GetComponent<TransformComponent>().Scale = { 30.0f, 30.0f, 1.0f };
        auto& bgSprite = bg.AddComponent<SpriteRendererComponent>();
        bgSprite.Texture = m_CheckerboardTexture;
        bgSprite.Color = { 0.28f, 0.30f, 0.32f, 0.15f }; // Subtle transparent dark overlay
        bgSprite.TilingFactor = 15.0f;

        // Entity 2: Player Quad (Dynamic physics body + box collider)
        auto player = m_ActiveScene->CreateEntity("Player");
        player.GetComponent<TransformComponent>().Translation = { 0.0f, 2.0f, 0.1f };
        player.GetComponent<TransformComponent>().Scale = { 1.0f, 1.0f, 1.0f };
        auto& playerSprite = player.AddComponent<SpriteRendererComponent>();
        playerSprite.Color = { 0.2f, 0.7f, 1.0f, 1.0f };
        auto& playerRb = player.AddComponent<Rigidbody2DComponent>();
        playerRb.Type = Rigidbody2DComponent::BodyType::Dynamic;
        auto& playerBc = player.AddComponent<BoxCollider2DComponent>();
        playerBc.Density = 1.0f;
        playerBc.Restitution = 0.6f;

        // Entity 3: Primary Camera
        auto camera = m_ActiveScene->CreateEntity("Main Camera");
        auto& camComp = camera.AddComponent<CameraComponent>();
        camComp.Primary = true;
        camComp.OrthographicSize = 10.0f;

        // Entity 4: Static Ground Floor
        auto ground = m_ActiveScene->CreateEntity("Ground");
        ground.GetComponent<TransformComponent>().Translation = { 0.0f, -3.0f, 0.0f };
        ground.GetComponent<TransformComponent>().Scale = { 10.0f, 0.8f, 1.0f };
        ground.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.3f, 0.35f, 0.4f, 1.0f });
        auto& groundRb = ground.AddComponent<Rigidbody2DComponent>();
        groundRb.Type = Rigidbody2DComponent::BodyType::Static;
        ground.AddComponent<BoxCollider2DComponent>();

        // Entity 5: Particle Torch Emitter
        auto torch = m_ActiveScene->CreateEntity("Particle Torch");
        torch.GetComponent<TransformComponent>().Translation = { -3.0f, -2.2f, 0.0f };
        torch.GetComponent<TransformComponent>().Scale = { 0.3f, 0.6f, 1.0f };
        torch.AddComponent<SpriteRendererComponent>(glm::vec4{ 0.8f, 0.5f, 0.2f, 1.0f });
        auto& pe = torch.AddComponent<ParticleEmitterComponent>();
        pe.EmissionRate = 35.0f;
        pe.VelocityMin = { -0.3f, 1.5f };
        pe.VelocityMax = { 0.3f, 3.0f };
        pe.ColorBegin = { 1.0f, 0.8f, 0.2f, 1.0f };
        pe.ColorEnd = { 0.8f, 0.1f, 0.0f, 0.0f };
        pe.SizeBegin = 0.12f;
        pe.SizeEnd = 0.02f;

        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        m_SceneHierarchyPanel.SetSelectedEntity(player);
        m_EditorScenePath = std::filesystem::path{};

        LLAX_INFO("Created new default scene with 5 entities.");
    }

    void EditorLayer::OpenScene()
    {
        std::string filepath = FileDialogs::OpenFile("LLaX Scene (*.llax)\0*.llax\0");
        if (!filepath.empty())
            OpenScene(filepath);
    }

    void EditorLayer::OpenScene(const std::filesystem::path& path)
    {
        if (m_SceneState != SceneState::Edit)
            OnSceneStop();

        if (path.extension().string() != ".llax")
        {
            LLAX_WARN("Could not load '{0}' - not a .llax scene file!", path.string());
            return;
        }

        Ref<Scene> newScene = CreateRef<Scene>();
        SceneSerializer serializer(newScene);

        if (serializer.Deserialize(path.string()))
        {
            m_EditorScene = newScene;
            m_EditorScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);

            m_ActiveScene = m_EditorScene;
            m_EditorScenePath = path;

            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
            m_SceneHierarchyPanel.SetSelectedEntity({});
            LLAX_INFO("Opened scene: '{0}'", path.string());
        }
        else
        {
            LLAX_ERROR("Failed to open scene: '{0}'", path.string());
        }
    }

    void EditorLayer::SaveScene()
    {
        if (!m_EditorScenePath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(m_EditorScenePath.string());
            LLAX_INFO("Scene saved to '{0}'", m_EditorScenePath.string());
        }
        else
        {
            SaveSceneAs();
        }
    }

    void EditorLayer::SaveSceneAs()
    {
        std::string filepath = FileDialogs::SaveFile("LLaX Scene (*.llax)\0*.llax\0");
        if (!filepath.empty())
        {
            SceneSerializer serializer(m_ActiveScene);
            serializer.Serialize(filepath);
            m_EditorScenePath = filepath;
            LLAX_INFO("Scene saved as '{0}'", filepath);
        }
    }

    void EditorLayer::OnScenePlay()
    {
        m_SceneState = SceneState::Play;
        m_EditorScene = Scene::Copy(m_ActiveScene);
        m_ActiveScene->OnPhysics2DStart();
        m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        LLAX_INFO("Scene Runtime simulation started (Play Mode & Physics2D active).");
    }

    void EditorLayer::OnSceneStop()
    {
        m_SceneState = SceneState::Edit;
        m_ActiveScene->OnPhysics2DStop();
        if (m_EditorScene)
        {
            m_ActiveScene = Scene::Copy(m_EditorScene);
            m_EditorScene = nullptr;
            m_SceneHierarchyPanel.SetContext(m_ActiveScene);
        }
        LLAX_INFO("Scene Runtime simulation stopped (Edit Mode restored).");
    }

    void EditorLayer::OnScenePause()
    {
        if (m_SceneState == SceneState::Play)
        {
            m_SceneState = SceneState::Pause;
            LLAX_INFO("Scene simulation paused.");
        }
        else if (m_SceneState == SceneState::Pause)
        {
            m_SceneState = SceneState::Play;
            LLAX_INFO("Scene simulation resumed.");
        }
    }

    void EditorLayer::OnUpdate(Timestep ts)
    {
        m_FrameTime = ts.GetSeconds();
        m_FPS = ts > 0.0f ? 1.0f / ts.GetSeconds() : 0.0f;

        // Framebuffer Resize if Viewport size changed
        if (m_ViewportSize.x > 0.0f && m_ViewportSize.y > 0.0f &&
            (m_Framebuffer->GetSpecification().Width != (uint32_t)m_ViewportSize.x ||
             m_Framebuffer->GetSpecification().Height != (uint32_t)m_ViewportSize.y))
        {
            m_Framebuffer->Resize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
            m_CameraController.OnResize(m_ViewportSize.x, m_ViewportSize.y);
            m_ActiveScene->OnViewportResize((uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        }

        // Render Scene into Framebuffer
        m_Framebuffer->Bind();
        Renderer::SetViewport(0, 0, (uint32_t)m_ViewportSize.x, (uint32_t)m_ViewportSize.y);
        Renderer::SetClearColor(m_ClearColor);
        Renderer::Clear();

        // Clear red-integer entity ID attachment to -1
        m_Framebuffer->ClearAttachment(1, -1);

        m_ActiveScene->SetPhysicsDebug(m_ShowPhysicsColliders);
        Renderer2D::ResetStats();

        switch (m_SceneState)
        {
            case SceneState::Edit:
            {
                if (m_ViewportFocused || m_ViewportHovered)
                    m_CameraController.OnUpdate(ts);

                m_ActiveScene->OnUpdateEditor(ts, m_CameraController.GetCamera());
                break;
            }
            case SceneState::Play:
            {
                m_ActiveScene->OnUpdateRuntime(ts);
                break;
            }
            case SceneState::Pause:
            {
                m_ActiveScene->OnUpdateRuntime(0.0f);
                break;
            }
        }

        // Viewport mouse picking: read pixel under cursor
        auto [mx, my] = ImGui::GetMousePos();
        mx -= m_ViewportBounds[0].x;
        my -= m_ViewportBounds[0].y;
        glm::vec2 viewportSize = m_ViewportBounds[1] - m_ViewportBounds[0];
        my = viewportSize.y - my; // Invert Y for OpenGL texture coordinates

        int mouseX = (int)mx;
        int mouseY = (int)my;

        if (mouseX >= 0 && mouseY >= 0 && mouseX < (int)viewportSize.x && mouseY < (int)viewportSize.y)
        {
            int pixelData = m_Framebuffer->ReadPixel(1, mouseX, mouseY);
            m_HoveredEntity = (pixelData == -1 || !m_ActiveScene->Reg().valid((entt::entity)pixelData))
                ? Entity{}
                : Entity{ (entt::entity)pixelData, m_ActiveScene.get() };
        }
        else
        {
            m_HoveredEntity = Entity{};
        }

        m_Framebuffer->Unbind();
    }

    void EditorLayer::OnImGuiRender()
    {
        // ─── 1. Main DockSpace Over Viewport ─────────────────────────────────
        static bool dockspaceOpen = true;
        static bool opt_fullscreen = true;
        static bool s_FirstLayout = true;  // build default layout once
        static ImGuiDockNodeFlags dockspace_flags = ImGuiDockNodeFlags_None;

        ImGuiWindowFlags window_flags = ImGuiWindowFlags_MenuBar | ImGuiWindowFlags_NoDocking;
        if (opt_fullscreen)
        {
            const ImGuiViewport* viewport = ImGui::GetMainViewport();
            ImGui::SetNextWindowPos(viewport->WorkPos);
            ImGui::SetNextWindowSize(viewport->WorkSize);
            ImGui::SetNextWindowViewport(viewport->ID);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 0.0f);
            ImGui::PushStyleVar(ImGuiStyleVar_WindowBorderSize, 0.0f);
            window_flags |= ImGuiWindowFlags_NoTitleBar | ImGuiWindowFlags_NoCollapse | ImGuiWindowFlags_NoResize | ImGuiWindowFlags_NoMove;
            window_flags |= ImGuiWindowFlags_NoBringToFrontOnFocus | ImGuiWindowFlags_NoNavFocus;
        }

        if (dockspace_flags & ImGuiDockNodeFlags_PassthruCentralNode)
            window_flags |= ImGuiWindowFlags_NoBackground;

        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(0.0f, 0.0f));
        ImGui::Begin("LLaXStudioDockSpace", &dockspaceOpen, window_flags);
        ImGui::PopStyleVar();

        if (opt_fullscreen)
            ImGui::PopStyleVar(2);

        // ── Submit DockSpace + build default layout ───────────────────────────
        ImGuiIO& io = ImGui::GetIO();
        if (io.ConfigFlags & ImGuiConfigFlags_DockingEnable)
        {
            ImGuiID dockspace_id = ImGui::GetID("MyDockSpace");
            ImGui::DockSpace(dockspace_id, ImVec2(0.0f, 0.0f), dockspace_flags);

            if (s_FirstLayout)
            {
                s_FirstLayout = false;

                // Clean reset of existing configuration
                ImGui::DockBuilderRemoveNode(dockspace_id);
                ImGui::DockBuilderAddNode(dockspace_id, dockspace_flags | ImGuiDockNodeFlags_DockSpace);
                ImGui::DockBuilderSetNodeSize(dockspace_id, ImGui::GetMainViewport()->WorkSize);

                ImGuiID dock_main = dockspace_id;

                // Left: Hierarchy (20%)
                ImGuiID dock_left;
                ImGuiID dock_remainder = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Left, 0.20f, &dock_left, &dock_main);

                // Right: Inspector / Properties (24%)
                ImGuiID dock_right;
                ImGuiID dock_center_area = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Right, 0.24f, &dock_right, &dock_main);

                // Bottom: Browser, Console, Stats (30%)
                ImGuiID dock_bottom;
                ImGuiID dock_center_top = ImGui::DockBuilderSplitNode(dock_main, ImGuiDir_Down, 0.30f, &dock_bottom, &dock_main);

                // Split bottom into Content Browser (50%) and Console / Stats (50%)
                ImGuiID dock_bottom_left, dock_bottom_right;
                ImGui::DockBuilderSplitNode(dock_bottom, ImGuiDir_Left, 0.52f, &dock_bottom_left, &dock_bottom_right);

                // Top: Toolbar (5%)
                ImGuiID dock_toolbar, dock_viewport;
                ImGui::DockBuilderSplitNode(dock_center_top, ImGuiDir_Up, 0.055f, &dock_toolbar, &dock_viewport);

                // Dock individual windows
                ImGui::DockBuilderDockWindow("Scene Hierarchy",  dock_left);
                ImGui::DockBuilderDockWindow("Properties",       dock_right);
                ImGui::DockBuilderDockWindow("Content Browser",  dock_bottom_left);
                ImGui::DockBuilderDockWindow("Console",          dock_bottom_right);
                ImGui::DockBuilderDockWindow("Statistics",       dock_bottom_right);
                ImGui::DockBuilderDockWindow("Viewport",         dock_viewport);
                ImGui::DockBuilderDockWindow("##Toolbar",        dock_toolbar);

                if (ImGuiDockNode* toolbarNode = ImGui::DockBuilderGetNode(dock_toolbar))
                {
                    toolbarNode->LocalFlags |= ImGuiDockNodeFlags_NoTabBar
                                            | ImGuiDockNodeFlags_NoResize
                                            | ImGuiDockNodeFlags_NoDockingOverMe;
                }

                ImGui::DockBuilderFinish(dockspace_id);
            }
        }

        // ─── 2. Main Menu Bar ────────────────────────────────────────────────
        if (ImGui::BeginMenuBar())
        {
            // ── Engine logo / title text on left ─────────────────────────────
            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.42f, 0.66f, 1.0f, 1.0f));
            ImGui::Text("  LLaX Studio");
            ImGui::PopStyleColor();

            ImGui::SeparatorEx(ImGuiSeparatorFlags_Vertical);
            ImGui::Spacing();

            if (ImGui::BeginMenu("File"))
            {
                if (ImGui::MenuItem(ICON_NEW " New Scene", "Ctrl+N"))
                    NewScene();

                if (ImGui::MenuItem(ICON_OPEN " Open Scene...", "Ctrl+O"))
                    OpenScene();

                ImGui::Separator();

                if (ImGui::MenuItem(ICON_SAVE " Save Scene", "Ctrl+S"))
                    SaveScene();

                if (ImGui::MenuItem(ICON_SAVEAS " Save Scene As...", "Ctrl+Shift+S"))
                    SaveSceneAs();

                ImGui::Separator();
                if (ImGui::MenuItem("Exit", "Alt+F4"))
                    Application::Get().Close();

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("View"))
            {
                ImGui::MenuItem("Viewport",        nullptr, &m_ShowViewport);
                ImGui::MenuItem("Scene Hierarchy", nullptr, &m_ShowSceneHierarchy);
                ImGui::MenuItem("Properties",      nullptr, &m_ShowProperties);
                ImGui::MenuItem("Content Browser", nullptr, &m_ShowContentBrowser);
                ImGui::MenuItem("Console",         nullptr, &m_ShowConsole);
                ImGui::MenuItem("Statistics",      nullptr, &m_ShowStats);
                ImGui::Separator();
                ImGui::MenuItem("Physics Colliders", nullptr, &m_ShowPhysicsColliders);
                ImGui::Separator();
                if (ImGui::MenuItem("Reset Layout"))
                    s_FirstLayout = true;
                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Entity"))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                    m_ActiveScene->CreateEntity("Empty Entity");

                if (ImGui::MenuItem("Create Sprite Entity"))
                {
                    Entity sprite = m_ActiveScene->CreateEntity("Sprite");
                    sprite.AddComponent<SpriteRendererComponent>();
                }

                if (ImGui::MenuItem("Create Camera Entity"))
                {
                    Entity cam = m_ActiveScene->CreateEntity("Camera");
                    cam.AddComponent<CameraComponent>();
                }

                ImGui::EndMenu();
            }

            if (ImGui::BeginMenu("Help"))
            {
                if (ImGui::MenuItem("About LLaX Studio"))
                    m_ShowAboutModal = true;

                ImGui::EndMenu();
            }

            // ── Right-side: scene path display ───────────────────────────────
            {
                std::string sceneLabel = m_EditorScenePath.empty()
                    ? "  Untitled Scene"
                    : "  " + m_EditorScenePath.filename().string();
                float textWidth = ImGui::CalcTextSize(sceneLabel.c_str()).x + 16.0f;
                ImGui::SetCursorPosX(ImGui::GetContentRegionMax().x - textWidth);
                ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.55f, 0.57f, 0.62f, 1.0f));
                ImGui::Text("%s", sceneLabel.c_str());
                ImGui::PopStyleColor();
            }

            ImGui::EndMenuBar();
        }

        // ─── 3. Panels ───────────────────────────────────────────────────────
        if (m_ShowSceneHierarchy || m_ShowProperties)
            m_SceneHierarchyPanel.OnImGuiRender();

        if (m_ShowContentBrowser)
            m_ContentBrowserPanel.OnImGuiRender();

        if (m_ShowConsole)
            m_ConsolePanel.OnImGuiRender();

        if (m_ShowStats)
            UI_Stats();

        // ─── 4. Viewport Panel ───────────────────────────────────────────────
        if (m_ShowViewport)
        {
            ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2{ 0, 0 });
            ImGui::Begin("Viewport");

            auto viewportMinRegion = ImGui::GetWindowContentRegionMin();
            auto viewportMaxRegion = ImGui::GetWindowContentRegionMax();
            auto viewportOffset = ImGui::GetWindowPos();
            m_ViewportBounds[0] = { viewportMinRegion.x + viewportOffset.x, viewportMinRegion.y + viewportOffset.y };
            m_ViewportBounds[1] = { viewportMaxRegion.x + viewportOffset.x, viewportMaxRegion.y + viewportOffset.y };

            m_ViewportFocused = ImGui::IsWindowFocused();
            m_ViewportHovered = ImGui::IsWindowHovered();
            Application::Get().GetImGuiLayer()->BlockEvents(!m_ViewportHovered);

            ImVec2 viewportPanelSize = ImGui::GetContentRegionAvail();
            if (viewportPanelSize.x > 0 && viewportPanelSize.y > 0)
                m_ViewportSize = { viewportPanelSize.x, viewportPanelSize.y };

            uint32_t textureID = m_Framebuffer->GetColorAttachmentRendererID();
            ImGui::Image((void*)(uintptr_t)textureID, ImVec2{ m_ViewportSize.x, m_ViewportSize.y }, ImVec2{ 0, 1 }, ImVec2{ 1, 0 });

            // ── Play-mode tinted border overlay ──────────────────────────────
            if (m_SceneState == SceneState::Play)
            {
                ImVec2 pMin = ImGui::GetItemRectMin();
                ImVec2 pMax = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddRect(pMin, pMax, IM_COL32(40, 200, 80, 200), 0.0f, 0, 3.0f);
            }
            else if (m_SceneState == SceneState::Pause)
            {
                ImVec2 pMin = ImGui::GetItemRectMin();
                ImVec2 pMax = ImGui::GetItemRectMax();
                ImGui::GetWindowDrawList()->AddRect(pMin, pMax, IM_COL32(230, 175, 30, 200), 0.0f, 0, 3.0f);
            }

            // ── Unity-style Viewport Tools Overlay (Top-Left: Gizmo mode / Hand / Move / Rotate / Scale) ──
            {
                ImVec2 overlayPos = ImGui::GetItemRectMin();
                overlayPos.x += 10.0f;
                overlayPos.y += 10.0f;
                ImGui::SetNextWindowPos(overlayPos, ImGuiCond_Always);
                ImGui::SetNextWindowBgAlpha(0.75f);
                ImGuiWindowFlags overlayFlags = ImGuiWindowFlags_NoDecoration
                    | ImGuiWindowFlags_NoDocking
                    | ImGuiWindowFlags_AlwaysAutoResize
                    | ImGuiWindowFlags_NoSavedSettings
                    | ImGuiWindowFlags_NoFocusOnAppearing
                    | ImGuiWindowFlags_NoNav
                    | ImGuiWindowFlags_NoMove;

                ImGui::PushStyleVar(ImGuiStyleVar_WindowRounding, 4.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_FrameRounding, 3.0f);
                ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2(3.0f, 2.0f));

                if (ImGui::Begin("##ViewportOverlay", nullptr, overlayFlags))
                {
                    // State Badge
                    if (m_SceneState == SceneState::Edit)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.45f, 0.78f, 0.42f, 1.0f));
                        ImGui::Text("EDIT");
                        ImGui::PopStyleColor();
                    }
                    else if (m_SceneState == SceneState::Play)
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.25f, 0.88f, 0.45f, 1.0f));
                        ImGui::Text("PLAY");
                        ImGui::PopStyleColor();
                    }
                    else
                    {
                        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.95f, 0.78f, 0.20f, 1.0f));
                        ImGui::Text("PAUSED");
                        ImGui::PopStyleColor();
                    }
                    
                    ImGui::SameLine();
                    ImGui::TextDisabled("| %.0f FPS", m_FPS);

                    ImGui::Separator();

                    // Unity Transform Tools Toolbar: [Q: View/Hand] [W: Translate] [E: Rotate] [R: Scale]
                    auto drawToolBtn = [&](const char* label, int typeId, const char* tooltip) {
                        bool active = (m_GizmoType == typeId);
                        if (active)
                        {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.24f, 0.46f, 0.85f, 1.0f));
                            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(1.0f, 1.0f, 1.0f, 1.0f));
                        }
                        else
                        {
                            ImGui::PushStyleColor(ImGuiCol_Button, ImVec4(0.14f, 0.15f, 0.17f, 0.85f));
                        }

                        if (ImGui::Button(label, ImVec2(24.0f, 20.0f)))
                            m_GizmoType = typeId;

                        if (active)
                            ImGui::PopStyleColor(2);
                        else
                            ImGui::PopStyleColor(1);

                        if (ImGui::IsItemHovered())
                            ImGui::SetTooltip("%s", tooltip);

                        ImGui::SameLine();
                    };

                    drawToolBtn("Q", 0, "Select / Hand Tool (Q)");
                    drawToolBtn("W", 1, "Translate Tool (W)");
                    drawToolBtn("E", 2, "Rotate Tool (E)");
                    drawToolBtn("R", 3, "Scale Tool (R)");

                    // 2D Camera / Grid Toggle
                    if (ImGui::Button(m_ShowGrid ? "Grid [ON]" : "Grid [OFF]", ImVec2(64.0f, 20.0f)))
                        m_ShowGrid = !m_ShowGrid;

                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Toggle Viewport Coordinate Grid");

                    ImGui::SameLine();

                    // Physics Colliders Wireframe Toggle
                    if (ImGui::Button(m_ShowPhysicsColliders ? "Colliders [ON]" : "Colliders [OFF]", ImVec2(90.0f, 20.0f)))
                        m_ShowPhysicsColliders = !m_ShowPhysicsColliders;

                    if (ImGui::IsItemHovered())
                        ImGui::SetTooltip("Toggle 2D Physics Collider Wireframe Debug Overlay");

                    if (m_HoveredEntity)
                    {
                        ImGui::Separator();
                        ImGui::TextColored(ImVec4(0.35f, 0.75f, 1.0f, 1.0f), "Hover: %s",
                            m_HoveredEntity.GetComponent<TagComponent>().Tag.c_str());
                    }
                }
                ImGui::End();
                ImGui::PopStyleVar(3);
            }

            // Drag and Drop into Viewport
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const char* path = (const char*)payload->Data;
                    std::filesystem::path droppedPath(path);
                    std::string ext = droppedPath.extension().string();

                    // Drop a .llax scene file → open it
                    if (ext == ".llax")
                    {
                        OpenScene(droppedPath);
                    }
                    // Drop a texture image
                    else if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                    {
                        Entity selected = m_SceneHierarchyPanel.GetSelectedEntity();
                        if (selected && selected.HasComponent<SpriteRendererComponent>())
                        {
                            selected.GetComponent<SpriteRendererComponent>().Texture = Texture2D::Create(path);
                        }
                        else
                        {
                            Entity newSprite = m_ActiveScene->CreateEntity(droppedPath.stem().string());
                            auto& src = newSprite.AddComponent<SpriteRendererComponent>();
                            src.Texture = Texture2D::Create(path);
                            m_SceneHierarchyPanel.SetSelectedEntity(newSprite);
                        }
                    }
                }
                ImGui::EndDragDropTarget();
            }

            // ── ImGuizmo 3D/2D Manipulation Gizmos ───────────────────────────
            Entity selectedEntity = m_SceneHierarchyPanel.GetSelectedEntity();
            if (selectedEntity && m_GizmoType > 0 && m_SceneState == SceneState::Edit)
            {
                ImGuizmo::SetOrthographic(true);
                ImGuizmo::SetDrawlist();

                ImGuizmo::SetRect(m_ViewportBounds[0].x, m_ViewportBounds[0].y,
                    m_ViewportBounds[1].x - m_ViewportBounds[0].x,
                    m_ViewportBounds[1].y - m_ViewportBounds[0].y);

                // Camera matrices
                const glm::mat4& cameraProjection = m_CameraController.GetCamera().GetProjectionMatrix();
                glm::mat4 cameraView = m_CameraController.GetCamera().GetViewMatrix();

                // Entity transform
                auto& tc = selectedEntity.GetComponent<TransformComponent>();
                glm::mat4 transform = tc.GetTransform();

                // Snapping
                bool snap = Input::IsKeyPressed(Key::LeftControl);
                float snapValue = 0.5f; // 0.5m for translation/scale
                if (m_GizmoType == 2)
                    snapValue = 45.0f; // 45 deg for rotation

                float snapValues[3] = { snapValue, snapValue, snapValue };

                ImGuizmo::OPERATION operation = ImGuizmo::TRANSLATE;
                if (m_GizmoType == 1) operation = ImGuizmo::TRANSLATE;
                else if (m_GizmoType == 2) operation = ImGuizmo::ROTATE;
                else if (m_GizmoType == 3) operation = ImGuizmo::SCALE;

                ImGuizmo::Manipulate(glm::value_ptr(cameraView), glm::value_ptr(cameraProjection),
                    operation, ImGuizmo::LOCAL, glm::value_ptr(transform),
                    nullptr, snap ? snapValues : nullptr);

                if (ImGuizmo::IsUsing())
                {
                    glm::vec3 translation, rotation, scale;
                    ImGuizmo::DecomposeMatrixToComponents(glm::value_ptr(transform),
                        glm::value_ptr(translation), glm::value_ptr(rotation), glm::value_ptr(scale));

                    tc.Translation = translation;
                    tc.Rotation = glm::radians(rotation);
                    tc.Scale = scale;
                }
            }

            ImGui::End();
            ImGui::PopStyleVar();
        }

        // ─── 5. Toolbar Panel ────────────────────────────────────────────────
        UI_Toolbar();

        // ─── 6. About Modal ──────────────────────────────────────────────────
        UI_AboutModal();

        ImGui::End(); // LLaXStudioDockSpace
    }

    void EditorLayer::UI_Toolbar()
    {
        // Toolbar sits in its own docked window (no title bar, no padding)
        ImGui::PushStyleVar(ImGuiStyleVar_WindowPadding, ImVec2(4.0f, 3.0f));
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing,   ImVec2(4.0f, 0.0f));
        ImGui::PushStyleColor(ImGuiCol_WindowBg, ImVec4(0.075f, 0.078f, 0.090f, 1.0f));

        ImGuiWindowFlags tbFlags = ImGuiWindowFlags_NoDecoration
                                 | ImGuiWindowFlags_NoScrollbar
                                 | ImGuiWindowFlags_NoScrollWithMouse;
        ImGui::Begin("##Toolbar", nullptr, tbFlags);

        float btnH    = ImGui::GetContentRegionAvail().y - 2.0f;
        float btnW    = btnH * 2.4f;   // wider pill shape
        float centerX = ImGui::GetContentRegionMax().x * 0.5f;
        float totalW  = btnW * 3.0f + ImGui::GetStyle().ItemSpacing.x * 2.0f;
        ImGui::SetCursorPosX(centerX - totalW * 0.5f);

        bool isPlaying = (m_SceneState == SceneState::Play || m_SceneState == SceneState::Pause);

        // ── Play / Stop ───────────────────────────────────────────────────────
        if (isPlaying)
        {
            // Stop — red accent
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.55f, 0.12f, 0.12f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.72f, 0.16f, 0.16f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.45f, 0.09f, 0.09f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(1.00f, 0.85f, 0.85f, 1.0f));
            if (ImGui::Button("  Stop  ", ImVec2(btnW, btnH)))
                OnSceneStop();
            ImGui::PopStyleColor(4);
        }
        else
        {
            // Play — green accent
            ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.12f, 0.45f, 0.18f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.16f, 0.58f, 0.24f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.09f, 0.36f, 0.14f, 1.0f));
            ImGui::PushStyleColor(ImGuiCol_Text,          ImVec4(0.85f, 1.00f, 0.87f, 1.0f));
            if (ImGui::Button("  Play  ", ImVec2(btnW, btnH)))
                OnScenePlay();
            ImGui::PopStyleColor(4);
        }

        ImGui::SameLine();

        // ── Pause / Resume ────────────────────────────────────────────────────
        bool isPaused = (m_SceneState == SceneState::Pause);
        ImVec4 pauseBtn   = isPaused ? ImVec4(0.50f, 0.38f, 0.08f, 1.0f) : ImVec4(0.16f, 0.17f, 0.20f, 1.0f);
        ImVec4 pauseHov   = isPaused ? ImVec4(0.65f, 0.50f, 0.10f, 1.0f) : ImVec4(0.22f, 0.36f, 0.65f, 1.0f);
        ImVec4 pauseAct   = isPaused ? ImVec4(0.40f, 0.30f, 0.06f, 1.0f) : ImVec4(0.14f, 0.25f, 0.48f, 1.0f);
        ImVec4 pauseTxt   = isPaused ? ImVec4(1.00f, 0.90f, 0.40f, 1.0f) : ImVec4(0.80f, 0.82f, 0.88f, 1.0f);
        ImGui::PushStyleColor(ImGuiCol_Button,        pauseBtn);
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, pauseHov);
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  pauseAct);
        ImGui::PushStyleColor(ImGuiCol_Text,          pauseTxt);
        const char* pauseLabel = isPaused ? " Resume " : " Pause  ";
        if (ImGui::Button(pauseLabel, ImVec2(btnW, btnH)))
            OnScenePause();
        ImGui::PopStyleColor(4);

        ImGui::SameLine();

        // ── Step ──────────────────────────────────────────────────────────────
        bool canStep = (m_SceneState == SceneState::Pause);
        if (!canStep) ImGui::BeginDisabled();
        ImGui::PushStyleColor(ImGuiCol_Button,        ImVec4(0.16f, 0.17f, 0.20f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4(0.22f, 0.36f, 0.65f, 1.0f));
        ImGui::PushStyleColor(ImGuiCol_ButtonActive,  ImVec4(0.14f, 0.25f, 0.48f, 1.0f));
        if (ImGui::Button("  Step  ", ImVec2(btnW, btnH)))
            m_ActiveScene->OnUpdateRuntime(1.0f / 60.0f);
        ImGui::PopStyleColor(3);
        if (!canStep) ImGui::EndDisabled();

        ImGui::PopStyleColor();  // WindowBg
        ImGui::PopStyleVar(2);
        ImGui::End();
    }

    void EditorLayer::UI_Stats()
    {
        ImGui::Begin("Statistics");

        // ── Header ────────────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.42f, 0.66f, 1.0f, 1.0f));
        ImGui::Text("Performance");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();

        // FPS gauge with colour: green > 60, yellow 30-60, red < 30
        ImVec4 fpsColor = m_FPS >= 60.0f
            ? ImVec4(0.30f, 0.88f, 0.44f, 1.0f)
            : (m_FPS >= 30.0f ? ImVec4(0.95f, 0.78f, 0.20f, 1.0f)
                              : ImVec4(0.90f, 0.28f, 0.28f, 1.0f));
        ImGui::TextColored(fpsColor, "%.1f FPS", m_FPS);
        ImGui::TextDisabled("Frame time:  %.3f ms", m_FrameTime * 1000.0f);

        ImGui::Spacing();
        ImGui::Separator();

        // ── Renderer stats ────────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.42f, 0.66f, 1.0f, 1.0f));
        ImGui::Text("Renderer2D");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();

        auto stats = Renderer2D::GetStats();

        auto statRow = [](const char* label, int value) {
            ImGui::TextDisabled("%s", label);
            ImGui::SameLine(110.0f);
            ImGui::Text("%d", value);
        };
        statRow("Draw Calls", stats.DrawCalls);
        statRow("Quads",      stats.QuadCount);
        statRow("Lines",      stats.LineCount);
        statRow("Vertices",   stats.GetTotalVertexCount());
        statRow("Indices",    stats.GetTotalIndexCount());

        ImGui::Spacing();
        ImGui::Separator();

        // ── Viewport settings ─────────────────────────────────────────────────
        ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.42f, 0.66f, 1.0f, 1.0f));
        ImGui::Text("Viewport");
        ImGui::PopStyleColor();
        ImGui::Separator();
        ImGui::Spacing();
        ImGui::SetNextItemWidth(-1.0f);
        ImGui::ColorEdit4("##ClearColor", &m_ClearColor.r);
        ImGui::SameLine(0, 4);
        ImGui::TextDisabled("Clear Color");

        ImGui::End();
    }

    void EditorLayer::UI_AboutModal()
    {
        if (m_ShowAboutModal)
        {
            ImGui::OpenPopup("About LLaX Studio");
        }

        if (ImGui::BeginPopupModal("About LLaX Studio", &m_ShowAboutModal, ImGuiWindowFlags_AlwaysAutoResize))
        {
            ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "LLaX Studio v0.1.0");
            ImGui::Text("Phase 3: Game Engine Editor");
            ImGui::Separator();
            ImGui::Spacing();

            ImGui::Text("Features:");
            ImGui::BulletText("Modular Docking Workspace with ImGui DockSpace");
            ImGui::BulletText("Off-screen Framebuffer Viewport with dynamic resize");
            ImGui::BulletText("Scene Hierarchy Panel with Entity creation & deletion");
            ImGui::BulletText("Properties / Inspector Panel with multi-axis transform controls");
            ImGui::BulletText("Content Browser Panel with drag-and-drop asset assignment");
            ImGui::BulletText("Console / Log Panel capturing live spdlog output with level filters");
            ImGui::BulletText("Simulation Runtime Toolbar: Play / Pause / Step / Stop");

            ImGui::Spacing();
            ImGui::Separator();
            if (ImGui::Button("Close", ImVec2(120, 0)))
            {
                m_ShowAboutModal = false;
                ImGui::CloseCurrentPopup();
            }

            ImGui::EndPopup();
        }
    }

    void EditorLayer::OnEvent(Event& e)
    {
        if (m_SceneState == SceneState::Edit)
        {
            m_CameraController.OnEvent(e);
        }

        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<KeyPressedEvent>(LLAX_BIND_EVENT_FN(EditorLayer::OnKeyPressed));
        dispatcher.Dispatch<MouseButtonPressedEvent>(LLAX_BIND_EVENT_FN(EditorLayer::OnMouseButtonPressed));
    }

    bool EditorLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        // Shortcuts
        if (e.GetRepeatCount() > 0)
            return false;

        bool control = Input::IsKeyPressed(Key::LeftControl) || Input::IsKeyPressed(Key::RightControl);
        bool shift = Input::IsKeyPressed(Key::LeftShift) || Input::IsKeyPressed(Key::RightShift);

        switch (e.GetKeyCode())
        {
            case Key::N:
            {
                if (control)
                    NewScene();
                break;
            }
            case Key::O:
            {
                if (control)
                    OpenScene();
                break;
            }
            case Key::S:
            {
                if (control && shift)
                    SaveSceneAs();
                else if (control)
                    SaveScene();
                break;
            }
            case Key::D:
            {
                if (control)
                {
                    Entity selected = m_SceneHierarchyPanel.GetSelectedEntity();
                    if (selected)
                        m_SceneHierarchyPanel.SetSelectedEntity(m_ActiveScene->DuplicateEntity(selected));
                }
                break;
            }
            case Key::F5:
            {
                if (m_SceneState == SceneState::Edit)
                    OnScenePlay();
                else
                    OnSceneStop();
                break;
            }
            case Key::Delete:
            {
                Entity selected = m_SceneHierarchyPanel.GetSelectedEntity();
                if (selected)
                {
                    m_ActiveScene->DestroyEntity(selected);
                    m_SceneHierarchyPanel.SetSelectedEntity({});
                }
                break;
            }

            // Unity Viewport Tools (Q, W, E, R)
            case Key::Q:
            {
                if (!control)
                    m_GizmoType = 0; // Hand / Select
                break;
            }
            case Key::W:
            {
                if (!control)
                    m_GizmoType = 1; // Translate
                break;
            }
            case Key::E:
            {
                if (!control)
                    m_GizmoType = 2; // Rotate
                break;
            }
            case Key::R:
            {
                if (!control)
                    m_GizmoType = 3; // Scale
                break;
            }
        }

        return false;
    }

    bool EditorLayer::OnMouseButtonPressed(MouseButtonPressedEvent& e)
    {
        if (e.GetMouseButton() == Mouse::ButtonLeft)
        {
            if (m_ViewportHovered && !ImGuizmo::IsOver() && !Input::IsKeyPressed(Key::LeftAlt) && !ImGui::IsAnyItemHovered())
                m_SceneHierarchyPanel.SetSelectedEntity(m_HoveredEntity);
        }
        return false;
    }

    void EditorLayer::SetDarkThemeColors()
    {
        ImGuiStyle& style = ImGui::GetStyle();
        ImVec4* colors   = style.Colors;

        // ── Accent colour: a vivid blue-indigo ──────────────────────────────
        // Base surfaces
        colors[ImGuiCol_WindowBg]             = ImVec4(0.098f, 0.102f, 0.110f, 1.000f); // near-black
        colors[ImGuiCol_ChildBg]              = ImVec4(0.082f, 0.086f, 0.094f, 1.000f);
        colors[ImGuiCol_PopupBg]              = ImVec4(0.118f, 0.122f, 0.133f, 0.980f);
        colors[ImGuiCol_Border]               = ImVec4(0.180f, 0.184f, 0.200f, 1.000f);
        colors[ImGuiCol_BorderShadow]         = ImVec4(0.000f, 0.000f, 0.000f, 0.000f);

        // Frame (inputs, sliders, etc.)
        colors[ImGuiCol_FrameBg]              = ImVec4(0.145f, 0.150f, 0.165f, 1.000f);
        colors[ImGuiCol_FrameBgHovered]       = ImVec4(0.200f, 0.208f, 0.230f, 1.000f);
        colors[ImGuiCol_FrameBgActive]        = ImVec4(0.165f, 0.172f, 0.190f, 1.000f);

        // Title bars
        colors[ImGuiCol_TitleBg]              = ImVec4(0.075f, 0.078f, 0.086f, 1.000f);
        colors[ImGuiCol_TitleBgActive]        = ImVec4(0.100f, 0.105f, 0.118f, 1.000f);
        colors[ImGuiCol_TitleBgCollapsed]     = ImVec4(0.075f, 0.078f, 0.086f, 0.750f);

        // Scrollbar
        colors[ImGuiCol_ScrollbarBg]          = ImVec4(0.075f, 0.078f, 0.086f, 1.000f);
        colors[ImGuiCol_ScrollbarGrab]        = ImVec4(0.240f, 0.248f, 0.270f, 1.000f);
        colors[ImGuiCol_ScrollbarGrabHovered] = ImVec4(0.310f, 0.318f, 0.345f, 1.000f);
        colors[ImGuiCol_ScrollbarGrabActive]  = ImVec4(0.380f, 0.388f, 0.420f, 1.000f);

        // Check marks & sliders
        colors[ImGuiCol_CheckMark]            = ImVec4(0.345f, 0.565f, 1.000f, 1.000f);
        colors[ImGuiCol_SliderGrab]           = ImVec4(0.280f, 0.475f, 0.900f, 1.000f);
        colors[ImGuiCol_SliderGrabActive]     = ImVec4(0.345f, 0.565f, 1.000f, 1.000f);

        // Buttons — subtle dark, accent on hover/active
        colors[ImGuiCol_Button]               = ImVec4(0.165f, 0.172f, 0.190f, 1.000f);
        colors[ImGuiCol_ButtonHovered]        = ImVec4(0.235f, 0.380f, 0.720f, 1.000f);
        colors[ImGuiCol_ButtonActive]         = ImVec4(0.280f, 0.460f, 0.880f, 1.000f);

        // Headers (collapsing headers, tree nodes, selectables)
        colors[ImGuiCol_Header]               = ImVec4(0.180f, 0.188f, 0.210f, 1.000f);
        colors[ImGuiCol_HeaderHovered]        = ImVec4(0.235f, 0.380f, 0.720f, 0.800f);
        colors[ImGuiCol_HeaderActive]         = ImVec4(0.280f, 0.460f, 0.880f, 1.000f);

        // Separator
        colors[ImGuiCol_Separator]            = ImVec4(0.200f, 0.205f, 0.225f, 1.000f);
        colors[ImGuiCol_SeparatorHovered]     = ImVec4(0.280f, 0.460f, 0.880f, 0.780f);
        colors[ImGuiCol_SeparatorActive]      = ImVec4(0.345f, 0.565f, 1.000f, 1.000f);

        // Resize grips
        colors[ImGuiCol_ResizeGrip]           = ImVec4(0.200f, 0.208f, 0.230f, 0.500f);
        colors[ImGuiCol_ResizeGripHovered]    = ImVec4(0.280f, 0.460f, 0.880f, 0.670f);
        colors[ImGuiCol_ResizeGripActive]     = ImVec4(0.345f, 0.565f, 1.000f, 0.950f);

        // Tabs
        colors[ImGuiCol_Tab]                  = ImVec4(0.098f, 0.102f, 0.112f, 1.000f);
        colors[ImGuiCol_TabHovered]           = ImVec4(0.235f, 0.380f, 0.720f, 0.900f);
        colors[ImGuiCol_TabSelected]          = ImVec4(0.165f, 0.265f, 0.540f, 1.000f);
        colors[ImGuiCol_TabDimmed]            = ImVec4(0.098f, 0.102f, 0.112f, 1.000f);
        colors[ImGuiCol_TabDimmedSelected]    = ImVec4(0.130f, 0.185f, 0.340f, 1.000f);

        // DockingPreview / Empty
        colors[ImGuiCol_DockingPreview]       = ImVec4(0.280f, 0.460f, 0.880f, 0.700f);
        colors[ImGuiCol_DockingEmptyBg]       = ImVec4(0.060f, 0.062f, 0.070f, 1.000f);

        // Plot lines
        colors[ImGuiCol_PlotLines]            = ImVec4(0.345f, 0.565f, 1.000f, 1.000f);
        colors[ImGuiCol_PlotLinesHovered]     = ImVec4(0.440f, 0.660f, 1.000f, 1.000f);
        colors[ImGuiCol_PlotHistogram]        = ImVec4(0.280f, 0.460f, 0.880f, 1.000f);
        colors[ImGuiCol_PlotHistogramHovered] = ImVec4(0.345f, 0.565f, 1.000f, 1.000f);

        // Text
        colors[ImGuiCol_Text]                 = ImVec4(0.900f, 0.905f, 0.920f, 1.000f);
        colors[ImGuiCol_TextDisabled]         = ImVec4(0.420f, 0.430f, 0.460f, 1.000f);
        colors[ImGuiCol_TextSelectedBg]       = ImVec4(0.280f, 0.460f, 0.880f, 0.350f);

        // Nav
        colors[ImGuiCol_NavHighlight]         = ImVec4(0.345f, 0.565f, 1.000f, 1.000f);

        // ── Geometry / Sizing ────────────────────────────────────────────────
        style.WindowPadding          = ImVec2(10.0f, 10.0f);
        style.FramePadding           = ImVec2(6.0f,  4.0f);
        style.CellPadding            = ImVec2(6.0f,  4.0f);
        style.ItemSpacing            = ImVec2(8.0f,  5.0f);
        style.ItemInnerSpacing       = ImVec2(5.0f,  4.0f);
        style.IndentSpacing          = 18.0f;
        style.ScrollbarSize          = 12.0f;
        style.GrabMinSize            = 10.0f;

        // ── Roundness ───────────────────────────────────────────────────────
        style.WindowRounding         = 6.0f;
        style.ChildRounding          = 4.0f;
        style.FrameRounding          = 4.0f;
        style.PopupRounding          = 6.0f;
        style.ScrollbarRounding      = 6.0f;
        style.GrabRounding           = 4.0f;
        style.TabRounding            = 5.0f;

        // ── Borders ─────────────────────────────────────────────────────────
        style.WindowBorderSize       = 1.0f;
        style.ChildBorderSize        = 1.0f;
        style.FrameBorderSize        = 0.0f;
        style.PopupBorderSize        = 1.0f;

        // Hide the little window-menu collapse arrow (cleaner look)
        style.WindowMenuButtonPosition = ImGuiDir_None;
    }

} // namespace LLaX

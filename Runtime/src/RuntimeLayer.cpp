#include "RuntimeLayer.h"

namespace LLaX
{
    RuntimeLayer::RuntimeLayer(const std::filesystem::path& scenePath)
        : Layer("RuntimeLayer"), m_ScenePath(scenePath)
    {
    }

    void RuntimeLayer::OnAttach()
    {
        LLAX_INFO("=================================================");
        LLAX_INFO("   LLaX Standalone Game Runtime Player Started   ");
        LLAX_INFO("=================================================");

        // 1. Determine scene path
        if (m_ScenePath.empty() || !std::filesystem::exists(m_ScenePath))
        {
            std::filesystem::path candidates[] = {
                "Assets/scenes/Main.llax",
                "Assets/scenes/Default.llax",
                "Main.llax"
            };

            for (const auto& path : candidates)
            {
                if (std::filesystem::exists(path))
                {
                    m_ScenePath = path;
                    break;
                }
            }
        }

        m_Scene = CreateRef<Scene>();

        bool loaded = false;
        if (!m_ScenePath.empty() && std::filesystem::exists(m_ScenePath))
        {
            SceneSerializer serializer(m_Scene);
            loaded = serializer.Deserialize(m_ScenePath.string());
            if (loaded)
                LLAX_INFO("[Runtime] Loaded game scene: '{0}'", m_ScenePath.string());
        }

        if (!loaded)
        {
            LLAX_WARN("[Runtime] No scene file found; creating fallback default game scene.");
            // Create a simple player and camera
            auto player = m_Scene->CreateEntity("Player");
            player.GetComponent<TransformComponent>().Translation = { 0.0f, 0.0f, 0.0f };
            auto& sprite = player.AddComponent<SpriteRendererComponent>();
            sprite.Color = { 0.2f, 0.7f, 1.0f, 1.0f };
            player.AddComponent<CSharpScriptComponent>("Sandbox.Player");

            auto camera = m_Scene->CreateEntity("Main Camera");
            auto& cam = camera.AddComponent<CameraComponent>();
            cam.Primary = true;
            cam.OrthographicSize = 10.0f;
        }

        auto& app = Application::Get();
        m_Scene->OnViewportResize(app.GetWindow().GetWidth(), app.GetWindow().GetHeight());

        // Start Physics, Audio and C# Scripts immediately
        m_Scene->OnPhysics2DStart();
        LLAX_INFO("[Runtime] Game simulation active.");
    }

    void RuntimeLayer::OnDetach()
    {
        if (m_Scene)
            m_Scene->OnPhysics2DStop();

        LLAX_INFO("[Runtime] Game stopped.");
    }

    void RuntimeLayer::OnUpdate(Timestep ts)
    {
        auto& app = Application::Get();
        uint32_t width = app.GetWindow().GetWidth();
        uint32_t height = app.GetWindow().GetHeight();

        Renderer::SetViewport(0, 0, width, height);
        Renderer::SetClearColor({ 0.12f, 0.13f, 0.15f, 1.0f });
        Renderer::Clear();

        if (!m_IsPaused && m_Scene)
        {
            m_Scene->OnUpdateRuntime(ts);
        }
    }

    void RuntimeLayer::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<WindowResizeEvent>(LLAX_BIND_EVENT_FN(RuntimeLayer::OnWindowResize));
        dispatcher.Dispatch<KeyPressedEvent>(LLAX_BIND_EVENT_FN(RuntimeLayer::OnKeyPressed));
    }

    bool RuntimeLayer::OnWindowResize(WindowResizeEvent& e)
    {
        if (m_Scene && e.GetWidth() > 0 && e.GetHeight() > 0)
        {
            m_Scene->OnViewportResize(e.GetWidth(), e.GetHeight());
        }
        return false;
    }

    bool RuntimeLayer::OnKeyPressed(KeyPressedEvent& e)
    {
        if (e.GetKeyCode() == Key::Escape)
        {
            Application::Get().Close();
            return true;
        }
        return false;
    }
}

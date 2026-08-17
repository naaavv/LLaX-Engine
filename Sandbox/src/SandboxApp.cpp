#include <LLaX.h>

struct TransformComponent
{
    glm::vec3 Position{ 0.0f };
    glm::vec3 Rotation{ 0.0f };
    glm::vec3 Scale{ 1.0f };
};

struct TagComponent
{
    std::string Tag;
};

class SandboxApp : public LLaX::Application
{
public:
    SandboxApp()
        : LLaX::Application("LLaX Engine - Sandbox")
    {
        LLAX_INFO("Sandbox Application Started!");

        // Demo: Setup EnTT Entity Component System registry
        auto entity = m_Registry.create();
        m_Registry.emplace<TagComponent>(entity, "Main Camera");
        m_Registry.emplace<TransformComponent>(entity, glm::vec3(0.0f, 0.0f, 5.0f));

        auto light = m_Registry.create();
        m_Registry.emplace<TagComponent>(light, "Directional Light");
        m_Registry.emplace<TransformComponent>(light, glm::vec3(2.0f, 4.0f, 2.0f));
    }

    ~SandboxApp() override
    {
        LLAX_INFO("Sandbox Application Exiting!");
    }

protected:
    void OnUpdate(float ts) override
    {
        m_FrameTime = ts;
        m_FPS = ts > 0.0f ? 1.0f / ts : 0.0f;

        // Apply background clear color
        LLaX::Renderer::SetClearColor(m_ClearColor);
        LLaX::Renderer::Clear();
    }

    void OnImGuiRender() override
    {
        // Demonstration ImGui Panel
        ImGui::Begin("LLaX Engine Control Panel");

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "LLaX Engine v0.1.0");
        ImGui::Separator();

        ImGui::Text("Performance:");
        ImGui::Text("  FPS: %.1f", m_FPS);
        ImGui::Text("  Frame Time: %.3f ms", m_FrameTime * 1000.0f);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Renderer Settings:");
        ImGui::ColorEdit4("Clear Color", &m_ClearColor.r);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("EnTT Scene Hierarchy (Entities):");
        
        auto view = m_Registry.view<TagComponent, TransformComponent>();
        for (auto entity : view)
        {
            auto& tag = view.get<TagComponent>(entity);
            auto& transform = view.get<TransformComponent>(entity);

            if (ImGui::TreeNode(tag.Tag.c_str()))
            {
                ImGui::DragFloat3("Position", &transform.Position.x, 0.1f);
                ImGui::DragFloat3("Rotation", &transform.Rotation.x, 0.5f);
                ImGui::DragFloat3("Scale", &transform.Scale.x, 0.1f);
                ImGui::TreePop();
            }
        }

        if (ImGui::Button("Spawn Demo Entity"))
        {
            auto e = m_Registry.create();
            static int entityCount = 1;
            m_Registry.emplace<TagComponent>(e, "Entity #" + std::to_string(entityCount++));
            m_Registry.emplace<TransformComponent>(e);
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Integrated Libraries:");
        ImGui::BulletText("GLFW 3.4 (Windowing & Events)");
        ImGui::BulletText("GLAD (OpenGL 4.6 Loader)");
        ImGui::BulletText("Dear ImGui (Docking Enabled)");
        ImGui::BulletText("GLM 1.0.1 (Mathematics)");
        ImGui::BulletText("EnTT 3.13.2 (Entity Component System)");
        ImGui::BulletText("Assimp 5.4.3 (3D Asset Importer)");
        ImGui::BulletText("spdlog 1.14.1 (Fast Logging)");

        ImGui::End();

        // Optional: show ImGui demo window
        if (m_ShowImGuiDemo)
            ImGui::ShowDemoWindow(&m_ShowImGuiDemo);
    }

private:
    entt::registry m_Registry;
    glm::vec4 m_ClearColor{ 0.10f, 0.11f, 0.13f, 1.0f };
    float m_FrameTime = 0.0f;
    float m_FPS = 0.0f;
    bool m_ShowImGuiDemo = false;
};

int main(int argc, char** argv)
{
    auto app = std::make_unique<SandboxApp>();
    app->Run();
    return 0;
}

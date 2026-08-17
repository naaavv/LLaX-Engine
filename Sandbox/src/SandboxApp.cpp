#include <LLaX.h>

struct Transform
{
    glm::vec3 Position{ 0.0f };
    glm::vec3 Rotation{ 0.0f };
    glm::vec3 Scale{ 1.0f };
};

struct GameObject
{
    std::string Name;
    Transform Transform;
};

class ExampleLayer : public LLaX::Layer
{
public:
    ExampleLayer()
        : Layer("ExampleLayer")
    {
    }

    void OnAttach() override
    {
        LLAX_INFO("ExampleLayer attached to LayerStack!");

        // Test Assertion macro (will succeed)
        LLAX_ASSERT(true, "Assertion test passed!");

        m_Objects.push_back({ "Main Camera", { glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(1.0f) } });
        m_Objects.push_back({ "Player Quad", { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f) } });
        m_Objects.push_back({ "Directional Light", { glm::vec3(2.0f, 4.0f, 2.0f), glm::vec3(0.0f), glm::vec3(1.0f) } });
    }

    void OnDetach() override
    {
        LLAX_INFO("ExampleLayer detached!");
    }

    void OnUpdate(LLaX::Timestep ts) override
    {
        m_FrameTime = ts.GetSeconds();
        m_FPS = ts > 0.0f ? 1.0f / ts.GetSeconds() : 0.0f;

        // Input Polling: Move Player with WASD keys
        if (m_Objects.size() > 1)
        {
            auto& playerTransform = m_Objects[1].Transform;
            float speed = 3.0f * ts;

            if (LLaX::Input::IsKeyPressed(LLaX::Key::W)) playerTransform.Position.y += speed;
            if (LLaX::Input::IsKeyPressed(LLaX::Key::S)) playerTransform.Position.y -= speed;
            if (LLaX::Input::IsKeyPressed(LLaX::Key::A)) playerTransform.Position.x -= speed;
            if (LLaX::Input::IsKeyPressed(LLaX::Key::D)) playerTransform.Position.x += speed;
        }

        // Apply background clear color
        LLaX::Renderer::SetClearColor(m_ClearColor);
        LLaX::Renderer::Clear();
    }

    void OnImGuiRender() override
    {
        ImGui::Begin("LLaX Engine - Layer & Controls");

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "LLaX Engine v0.1.0 (LayerStack Enabled)");
        ImGui::Separator();

        ImGui::Text("Performance:");
        ImGui::Text("  FPS: %.1f", m_FPS);
        ImGui::Text("  Frame Time: %.3f ms", m_FrameTime * 1000.0f);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Real-Time Input Polling:");
        glm::vec2 mousePos = LLaX::Input::GetMousePosition();
        ImGui::Text("  Mouse Pos: (%.1f, %.1f)", mousePos.x, mousePos.y);
        ImGui::Text("  Controls: Hold [W / A / S / D] to move Player Quad");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Renderer Settings:");
        ImGui::ColorEdit4("Clear Color", &m_ClearColor.r);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Scene Hierarchy (Layer Objects):");

        for (size_t i = 0; i < m_Objects.size(); ++i)
        {
            auto& obj = m_Objects[i];
            if (ImGui::TreeNode((obj.Name + "##" + std::to_string(i)).c_str()))
            {
                ImGui::DragFloat3("Position", &obj.Transform.Position.x, 0.05f);
                ImGui::DragFloat3("Rotation", &obj.Transform.Rotation.x, 0.5f);
                ImGui::DragFloat3("Scale", &obj.Transform.Scale.x, 0.05f);
                ImGui::TreePop();
            }
        }

        if (ImGui::Button("+ Add GameObject"))
        {
            static int count = 1;
            m_Objects.push_back({ "GameObject #" + std::to_string(count++), {} });
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Active Engine Features:");
        ImGui::BulletText("Layer & LayerStack System [ACTIVE]");
        ImGui::BulletText("Assertion & Debugging Macros (LLAX_ASSERT) [ACTIVE]");
        ImGui::BulletText("Real-Time Input Polling [ACTIVE]");
        ImGui::BulletText("Dear ImGui Docking Layer [ACTIVE]");

        ImGui::End();

        if (m_ShowImGuiDemo)
            ImGui::ShowDemoWindow(&m_ShowImGuiDemo);
    }

    void OnEvent(LLaX::Event& event) override
    {
        // Handle layer-specific events here if needed
    }

private:
    std::vector<GameObject> m_Objects;
    glm::vec4 m_ClearColor{ 0.10f, 0.11f, 0.13f, 1.0f };
    float m_FrameTime = 0.0f;
    float m_FPS = 0.0f;
    bool m_ShowImGuiDemo = false;
};

class SandboxApp : public LLaX::Application
{
public:
    SandboxApp()
        : LLaX::Application("LLaX Engine - LayerStack Sandbox")
    {
        PushLayer(new ExampleLayer());
    }

    ~SandboxApp() override
    {
    }
};

int main(int argc, char** argv)
{
    auto app = std::make_unique<SandboxApp>();
    app->Run();
    return 0;
}

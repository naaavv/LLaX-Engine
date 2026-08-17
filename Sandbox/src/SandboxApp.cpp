#include <LLaX.h>

struct Transform
{
    glm::vec3 Position{ 0.0f };
    glm::vec3 Rotation{ 0.0f };
    glm::vec3 Scale{ 1.0f };
};

struct Object
{
    std::string Name;
    Transform Transform;
};

class SandboxApp : public LLaX::Application
{
public:
    SandboxApp()
        : LLaX::Application("LLaX Engine - Sandbox")
    {
        LLAX_INFO("Sandbox Application Initialized!");

        m_Objects.push_back({ "Main Camera", { glm::vec3(0.0f, 0.0f, 5.0f), glm::vec3(0.0f), glm::vec3(1.0f) } });
        m_Objects.push_back({ "Player Quad", { glm::vec3(0.0f, 0.0f, 0.0f), glm::vec3(0.0f), glm::vec3(1.0f) } });
        m_Objects.push_back({ "Directional Light", { glm::vec3(2.0f, 4.0f, 2.0f), glm::vec3(0.0f), glm::vec3(1.0f) } });
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

        // Move Player Quad with WASD keys
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
        ImGui::Begin("LLaX Engine Control Panel");

        ImGui::TextColored(ImVec4(0.2f, 0.8f, 1.0f, 1.0f), "LLaX Engine v0.1.0");
        ImGui::Separator();

        ImGui::Text("Performance:");
        ImGui::Text("  FPS: %.1f", m_FPS);
        ImGui::Text("  Frame Time: %.3f ms", m_FrameTime * 1000.0f);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Input Polling:");
        glm::vec2 mousePos = LLaX::Input::GetMousePosition();
        ImGui::Text("  Mouse: (%.1f, %.1f)", mousePos.x, mousePos.y);
        ImGui::Text("  Press [W/A/S/D] to move Player Quad");

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Renderer Settings:");
        ImGui::ColorEdit4("Clear Color", &m_ClearColor.r);

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Scene Hierarchy (Objects):");

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

        if (ImGui::Button("+ Add Object"))
        {
            static int objectCount = 1;
            m_Objects.push_back({ "Object #" + std::to_string(objectCount++), {} });
        }

        ImGui::Spacing();
        ImGui::Separator();
        ImGui::Text("Integrated Core Vendors:");
        ImGui::BulletText("GLFW 3.4 (Windowing, Context & Events)");
        ImGui::BulletText("GLAD (OpenGL 4.6 Core Loader)");
        ImGui::BulletText("Dear ImGui (Docking & Multi-Viewports)");
        ImGui::BulletText("GLM 1.0.1 (Vector & Matrix Math)");
        ImGui::BulletText("stb_image (Texture & Image Loader)");
        ImGui::BulletText("spdlog 1.14.1 (Fast Logging)");

        ImGui::End();

        if (m_ShowImGuiDemo)
            ImGui::ShowDemoWindow(&m_ShowImGuiDemo);
    }

private:
    std::vector<Object> m_Objects;
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

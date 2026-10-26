#pragma once

#include <LLaX.h>
#include <filesystem>

namespace LLaX
{
    class RuntimeLayer : public Layer
    {
    public:
        RuntimeLayer(const std::filesystem::path& scenePath = "");
        virtual ~RuntimeLayer() override = default;

        virtual void OnAttach() override;
        virtual void OnDetach() override;

        virtual void OnUpdate(Timestep ts) override;
        virtual void OnEvent(Event& e) override;

    private:
        bool OnWindowResize(WindowResizeEvent& e);
        bool OnKeyPressed(KeyPressedEvent& e);

    private:
        Ref<Scene> m_Scene;
        std::filesystem::path m_ScenePath;
        bool m_IsPaused = false;
    };
}

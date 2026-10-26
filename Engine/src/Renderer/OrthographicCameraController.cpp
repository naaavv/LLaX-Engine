#include "OrthographicCameraController.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include <algorithm>

namespace LLaX
{
    OrthographicCameraController::OrthographicCameraController(float aspectRatio, bool rotation)
        : m_AspectRatio(aspectRatio),
          m_Camera(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel),
          m_Rotation(rotation)
    {
    }

    void OrthographicCameraController::CalculateView()
    {
        m_Camera.SetProjection(-m_AspectRatio * m_ZoomLevel, m_AspectRatio * m_ZoomLevel, -m_ZoomLevel, m_ZoomLevel);
    }

    void OrthographicCameraController::OnUpdate(Timestep ts)
    {
        float speed = m_CameraTranslationSpeed * ts;

        if (Input::IsKeyPressed(Key::A))
        {
            m_CameraPosition.x -= cos(glm::radians(m_CameraRotation)) * speed;
            m_CameraPosition.y -= sin(glm::radians(m_CameraRotation)) * speed;
        }
        else if (Input::IsKeyPressed(Key::D))
        {
            m_CameraPosition.x += cos(glm::radians(m_CameraRotation)) * speed;
            m_CameraPosition.y += sin(glm::radians(m_CameraRotation)) * speed;
        }

        if (Input::IsKeyPressed(Key::W))
        {
            m_CameraPosition.x += -sin(glm::radians(m_CameraRotation)) * speed;
            m_CameraPosition.y += cos(glm::radians(m_CameraRotation)) * speed;
        }
        else if (Input::IsKeyPressed(Key::S))
        {
            m_CameraPosition.x -= -sin(glm::radians(m_CameraRotation)) * speed;
            m_CameraPosition.y -= cos(glm::radians(m_CameraRotation)) * speed;
        }

        if (m_Rotation)
        {
            if (Input::IsKeyPressed(Key::Q))
                m_CameraRotation += m_CameraRotationSpeed * ts;
            if (Input::IsKeyPressed(Key::E))
                m_CameraRotation -= m_CameraRotationSpeed * ts;

            if (m_CameraRotation > 180.0f)
                m_CameraRotation -= 360.0f;
            else if (m_CameraRotation <= -180.0f)
                m_CameraRotation += 360.0f;

            m_Camera.SetRotation(m_CameraRotation);
        }

        m_Camera.SetPosition(m_CameraPosition);
        m_CameraTranslationSpeed = m_ZoomLevel * 2.5f;
    }

    void OrthographicCameraController::OnEvent(Event& e)
    {
        EventDispatcher dispatcher(e);
        dispatcher.Dispatch<MouseScrolledEvent>([this](MouseScrolledEvent& event) { return OnMouseScrolled(event); });
        dispatcher.Dispatch<WindowResizeEvent>([this](WindowResizeEvent& event) { return OnWindowResized(event); });
    }

    void OrthographicCameraController::OnResize(float width, float height)
    {
        if (height > 0.0f)
        {
            m_AspectRatio = width / height;
            CalculateView();
        }
    }

    bool OrthographicCameraController::OnMouseScrolled(MouseScrolledEvent& e)
    {
        m_ZoomLevel -= e.GetYOffset() * 0.25f;
        m_ZoomLevel = std::max(m_ZoomLevel, 0.25f);
        CalculateView();
        return false;
    }

    bool OrthographicCameraController::OnWindowResized(WindowResizeEvent& e)
    {
        OnResize(static_cast<float>(e.GetWidth()), static_cast<float>(e.GetHeight()));
        return false;
    }
}

#pragma once

#include "Core/Base.h"
#include "Core/KeyCodes.h"
#include "Core/MouseButtonCodes.h"
#include <glm/glm.hpp>

namespace LLaX
{
    class Input
    {
    public:
        static bool IsKeyPressed(KeyCode key);
        static bool IsMouseButtonPressed(MouseCode button);
        static glm::vec2 GetMousePosition();
        static float GetMouseX();
        static float GetMouseY();
    };
}

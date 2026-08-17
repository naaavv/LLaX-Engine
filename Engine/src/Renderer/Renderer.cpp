#include "Renderer.h"
#include "Core/Log.h"
#include <glad/glad.h>

namespace LLaX
{
    void Renderer::Init()
    {
        LLAX_CORE_INFO("Renderer initialized.");
        glEnable(GL_BLEND);
        glBlendFunc(GL_SRC_ALPHA, GL_ONE_MINUS_SRC_ALPHA);
        glEnable(GL_DEPTH_TEST);
    }

    void Renderer::Shutdown()
    {
        LLAX_CORE_INFO("Renderer shutdown.");
    }

    void Renderer::SetViewport(uint32_t x, uint32_t y, uint32_t width, uint32_t height)
    {
        glViewport(x, y, width, height);
    }

    void Renderer::SetClearColor(const glm::vec4& color)
    {
        glClearColor(color.r, color.g, color.b, color.a);
    }

    void Renderer::Clear()
    {
        glClear(GL_COLOR_BUFFER_BIT | GL_DEPTH_BUFFER_BIT);
    }
}

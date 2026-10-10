#pragma once

namespace LLaX
{
    // Abstract interface for a graphics context.
    // Each platform (OpenGL, Vulkan, D3D12, etc.) provides its own
    // concrete implementation of this class.
    class GraphicsContext
    {
    public:
        virtual ~GraphicsContext() = default;

        // Make this context current on the calling thread and
        // load/initialize the underlying graphics API.
        virtual void Init() = 0;

        // Present the back buffer to the screen (swap buffers).
        virtual void SwapBuffers() = 0;
    };
}

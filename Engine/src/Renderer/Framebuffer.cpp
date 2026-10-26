#include "Framebuffer.h"
#include "Platform/OpenGL/OpenGLFramebuffer.h"

namespace LLaX
{
    Ref<Framebuffer> Framebuffer::Create(const FramebufferSpecification& spec)
    {
        return CreateRef<OpenGLFramebuffer>(spec);
    }
}

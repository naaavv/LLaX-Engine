#include "VertexArray.h"
#include "Platform/OpenGL/OpenGLVertexArray.h"

namespace LLaX
{
    Ref<VertexArray> VertexArray::Create()
    {
        return CreateRef<OpenGLVertexArray>();
    }
}

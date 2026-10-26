#include "Shader.h"
#include "Platform/OpenGL/OpenGLShader.h"
#include "Core/Assert.h"

namespace LLaX
{
    Ref<Shader> Shader::Create(const std::string& filepath)
    {
        return CreateRef<OpenGLShader>(filepath);
    }

    Ref<Shader> Shader::Create(const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        return CreateRef<OpenGLShader>("UnnamedShader", vertexSrc, fragmentSrc);
    }

    Ref<Shader> Shader::Create(const std::string& name, const std::string& vertexSrc, const std::string& fragmentSrc)
    {
        return CreateRef<OpenGLShader>(name, vertexSrc, fragmentSrc);
    }

    // =========================================================================
    // ShaderLibrary
    // =========================================================================

    void ShaderLibrary::Add(const std::string& name, const Ref<Shader>& shader)
    {
        LLAX_CORE_ASSERT(!Exists(name), "Shader already exists!");
        m_Shaders[name] = shader;
    }

    void ShaderLibrary::Add(const Ref<Shader>& shader)
    {
        auto& name = shader->GetName();
        Add(name, shader);
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& filepath)
    {
        auto shader = Shader::Create(filepath);
        Add(shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Load(const std::string& name, const std::string& filepath)
    {
        auto shader = Shader::Create(filepath);
        Add(name, shader);
        return shader;
    }

    Ref<Shader> ShaderLibrary::Get(const std::string& name)
    {
        LLAX_CORE_ASSERT(Exists(name), "Shader not found!");
        return m_Shaders[name];
    }

    bool ShaderLibrary::Exists(const std::string& name) const
    {
        return m_Shaders.find(name) != m_Shaders.end();
    }
}

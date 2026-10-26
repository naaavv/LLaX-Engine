#include "ScriptEngine.h"
#include "Scene/Scene.h"
#include "Scene/Components.h"
#include "Core/Log.h"
#include "Core/Input.h"
#include <dotnet/hostfxr.h>
#include <dotnet/coreclr_delegates.h>

#include <windows.h>
#include <iostream>

namespace LLaX
{
    // C# Native Delegates matching [UnmanagedCallersOnly]
    typedef void (*InitFn)(void* isKeyDown, void* getTranslation, void* setTranslation, void* getRotation, void* setRotation, void* logMessage);
    typedef void (*OnCreateEntityFn)(uint64_t entityID, const char* className);
    typedef void (*OnUpdateEntityFn)(uint64_t entityID, float ts);
    typedef void (*OnDestroyEntityFn)(uint64_t entityID);

    struct ScriptEngineData
    {
        HMODULE HostfxrLib = nullptr;
        hostfxr_initialize_for_runtime_config_fn InitConfigFn = nullptr;
        hostfxr_get_runtime_delegate_fn GetDelegateFn = nullptr;
        hostfxr_close_fn CloseFn = nullptr;
        load_assembly_and_get_function_pointer_fn LoadAssemblyAndGetFunctionPointer = nullptr;

        InitFn InitFunc = nullptr;
        OnCreateEntityFn OnCreateEntityFunc = nullptr;
        OnUpdateEntityFn OnUpdateEntityFunc = nullptr;
        OnDestroyEntityFn OnDestroyEntityFunc = nullptr;

        Scene* SceneContext = nullptr;
        bool Initialized = false;
    };

    static ScriptEngineData s_Data;

    // ── Internal Call Native Implementations ─────────────────────────────────
    static bool CSharp_IsKeyDown(int keyCode)
    {
        return Input::IsKeyPressed(static_cast<KeyCode>(keyCode));
    }

    static void CSharp_GetTranslation(uint64_t entityID, glm::vec3* outTranslation)
    {
        if (!s_Data.SceneContext || !outTranslation) return;
        Entity entity{ static_cast<entt::entity>(entityID), s_Data.SceneContext };
        if (entity && entity.HasComponent<TransformComponent>())
            *outTranslation = entity.GetComponent<TransformComponent>().Translation;
    }

    static void CSharp_SetTranslation(uint64_t entityID, const glm::vec3* translation)
    {
        if (!s_Data.SceneContext || !translation) return;
        Entity entity{ static_cast<entt::entity>(entityID), s_Data.SceneContext };
        if (entity && entity.HasComponent<TransformComponent>())
            entity.GetComponent<TransformComponent>().Translation = *translation;
    }

    static void CSharp_GetRotation(uint64_t entityID, glm::vec3* outRotation)
    {
        if (!s_Data.SceneContext || !outRotation) return;
        Entity entity{ static_cast<entt::entity>(entityID), s_Data.SceneContext };
        if (entity && entity.HasComponent<TransformComponent>())
            *outRotation = entity.GetComponent<TransformComponent>().Rotation;
    }

    static void CSharp_SetRotation(uint64_t entityID, const glm::vec3* rotation)
    {
        if (!s_Data.SceneContext || !rotation) return;
        Entity entity{ static_cast<entt::entity>(entityID), s_Data.SceneContext };
        if (entity && entity.HasComponent<TransformComponent>())
            entity.GetComponent<TransformComponent>().Rotation = *rotation;
    }

    static void CSharp_LogMessage(int level, const char* message)
    {
        if (!message) return;
        if (level == 0)      LLAX_INFO("[C#] {0}", message);
        else if (level == 1) LLAX_WARN("[C#] {0}", message);
        else                 LLAX_ERROR("[C#] {0}", message);
    }

    // ── Hostfxr Discovery ────────────────────────────────────────────────────
    static std::wstring GetHostfxrPath()
    {
        WIN32_FIND_DATAW findData;
        HANDLE hFind = FindFirstFileW(L"C:\\Program Files\\dotnet\\host\\fxr\\*", &findData);
        if (hFind != INVALID_HANDLE_VALUE)
        {
            std::wstring latestVersion;
            do
            {
                if ((findData.dwFileAttributes & FILE_ATTRIBUTE_DIRECTORY) &&
                    findData.cFileName[0] != L'.')
                {
                    latestVersion = findData.cFileName;
                }
            } while (FindNextFileW(hFind, &findData));
            FindClose(hFind);

            if (!latestVersion.empty())
            {
                std::wstring path = L"C:\\Program Files\\dotnet\\host\\fxr\\" + latestVersion + L"\\hostfxr.dll";
                if (std::filesystem::exists(path))
                    return path;
            }
        }
        return L"";
    }

    void ScriptEngine::Init()
    {
        InitHost();

        std::filesystem::path assemblyPath = "Assets/scripts/bin/LLaX-ScriptCore.dll";
        if (std::filesystem::exists(assemblyPath))
        {
            LoadAssembly(assemblyPath);
        }
        else
        {
            LLAX_WARN("[ScriptEngine] C# assembly '{0}' not found yet. Compile LLaX-ScriptCore to enable C# scripts.", assemblyPath.string());
        }
    }

    void ScriptEngine::InitHost()
    {
        std::wstring hostfxrPath = GetHostfxrPath();
        if (hostfxrPath.empty())
        {
            LLAX_WARN("[ScriptEngine] .NET Hostfxr not found. C# scripting is inactive.");
            return;
        }

        s_Data.HostfxrLib = LoadLibraryW(hostfxrPath.c_str());
        if (!s_Data.HostfxrLib)
        {
            LLAX_WARN("[ScriptEngine] Failed to load hostfxr.dll from {0}", std::filesystem::path(hostfxrPath).string());
            return;
        }

        s_Data.InitConfigFn = (hostfxr_initialize_for_runtime_config_fn)GetProcAddress(s_Data.HostfxrLib, "hostfxr_initialize_for_runtime_config");
        s_Data.GetDelegateFn = (hostfxr_get_runtime_delegate_fn)GetProcAddress(s_Data.HostfxrLib, "hostfxr_get_runtime_delegate");
        s_Data.CloseFn = (hostfxr_close_fn)GetProcAddress(s_Data.HostfxrLib, "hostfxr_close");

        if (!s_Data.InitConfigFn || !s_Data.GetDelegateFn || !s_Data.CloseFn)
        {
            LLAX_ERROR("[ScriptEngine] Failed to acquire hostfxr function pointers!");
            return;
        }

        LLAX_INFO("[ScriptEngine] Microsoft .NET Hostfxr initialized successfully.");
    }

    bool ScriptEngine::LoadAssembly(const std::filesystem::path& filepath)
    {
        if (!s_Data.InitConfigFn)
            return false;

        std::filesystem::path configPath = filepath;
        configPath.replace_extension(".runtimeconfig.json");

        if (!std::filesystem::exists(configPath))
        {
            LLAX_ERROR("[ScriptEngine] Missing runtime config: {0}", configPath.string());
            return false;
        }

        std::wstring wConfigPath = configPath.wstring();
        hostfxr_handle cxt = nullptr;
        int rc = s_Data.InitConfigFn(wConfigPath.c_str(), nullptr, &cxt);
        if (rc != 0 || !cxt)
        {
            LLAX_ERROR("[ScriptEngine] hostfxr_initialize_for_runtime_config failed with code {0}", rc);
            return false;
        }

        rc = s_Data.GetDelegateFn(cxt, hdt_load_assembly_and_get_function_pointer, (void**)&s_Data.LoadAssemblyAndGetFunctionPointer);
        s_Data.CloseFn(cxt);

        if (rc != 0 || !s_Data.LoadAssemblyAndGetFunctionPointer)
        {
            LLAX_ERROR("[ScriptEngine] Failed to get load_assembly_and_get_function_pointer delegate!");
            return false;
        }

        std::wstring wAssemblyPath = std::filesystem::absolute(filepath).wstring();

        // Load entry points from LLaX.Runtime
        auto loadMethod = [&](const wchar_t* methodName, void** funcPtr) -> bool {
            int err = s_Data.LoadAssemblyAndGetFunctionPointer(
                wAssemblyPath.c_str(),
                L"LLaX.Runtime, LLaX-ScriptCore",
                methodName,
                UNMANAGEDCALLERSONLY_METHOD,
                nullptr,
                funcPtr
            );
            return err == 0 && *funcPtr != nullptr;
        };

        if (!loadMethod(L"Init", (void**)&s_Data.InitFunc) ||
            !loadMethod(L"OnCreateEntity", (void**)&s_Data.OnCreateEntityFunc) ||
            !loadMethod(L"OnUpdateEntity", (void**)&s_Data.OnUpdateEntityFunc) ||
            !loadMethod(L"OnDestroyEntity", (void**)&s_Data.OnDestroyEntityFunc))
        {
            LLAX_ERROR("[ScriptEngine] Failed to bind C# Runtime entry points!");
            return false;
        }

        // Initialize C# Runtime with native delegates
        s_Data.InitFunc(
            (void*)&CSharp_IsKeyDown,
            (void*)&CSharp_GetTranslation,
            (void*)&CSharp_SetTranslation,
            (void*)&CSharp_GetRotation,
            (void*)&CSharp_SetRotation,
            (void*)&CSharp_LogMessage
        );

        s_Data.Initialized = true;
        LLAX_INFO("[ScriptEngine] C# Assembly loaded and bound: {0}", filepath.filename().string());
        return true;
    }

    void ScriptEngine::Shutdown()
    {
        if (s_Data.HostfxrLib)
        {
            FreeLibrary(s_Data.HostfxrLib);
            s_Data.HostfxrLib = nullptr;
        }
        s_Data.Initialized = false;
        LLAX_INFO("[ScriptEngine] Shutdown complete.");
    }

    void ScriptEngine::OnRuntimeStart(Scene* scene)
    {
        s_Data.SceneContext = scene;
    }

    void ScriptEngine::OnRuntimeStop()
    {
        s_Data.SceneContext = nullptr;
    }

    void ScriptEngine::OnCreateEntity(Entity entity)
    {
        if (!s_Data.Initialized || !s_Data.OnCreateEntityFunc) return;

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            const auto& sc = entity.GetComponent<CSharpScriptComponent>();
            if (!sc.ClassName.empty())
            {
                s_Data.OnCreateEntityFunc((uint64_t)(uint32_t)entity, sc.ClassName.c_str());
            }
        }
    }

    void ScriptEngine::OnUpdateEntity(Entity entity, Timestep ts)
    {
        if (!s_Data.Initialized || !s_Data.OnUpdateEntityFunc) return;

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            s_Data.OnUpdateEntityFunc((uint64_t)(uint32_t)entity, ts.GetSeconds());
        }
    }

    void ScriptEngine::OnDestroyEntity(Entity entity)
    {
        if (!s_Data.Initialized || !s_Data.OnDestroyEntityFunc) return;

        if (entity.HasComponent<CSharpScriptComponent>())
        {
            s_Data.OnDestroyEntityFunc((uint64_t)(uint32_t)entity);
        }
    }

    Scene* ScriptEngine::GetSceneContext()
    {
        return s_Data.SceneContext;
    }

    bool ScriptEngine::IsInitialized()
    {
        return s_Data.Initialized;
    }
}

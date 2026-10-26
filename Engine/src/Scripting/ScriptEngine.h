#pragma once

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Scene/Entity.h"
#include <filesystem>
#include <string>

namespace LLaX
{
    class Scene;

    class ScriptEngine
    {
    public:
        static void Init();
        static void Shutdown();

        static bool LoadAssembly(const std::filesystem::path& filepath);

        static void OnRuntimeStart(Scene* scene);
        static void OnRuntimeStop();

        static void OnCreateEntity(Entity entity);
        static void OnUpdateEntity(Entity entity, Timestep ts);
        static void OnDestroyEntity(Entity entity);

        static Scene* GetSceneContext();
        static bool IsInitialized();

    private:
        static void InitHost();
    };
}

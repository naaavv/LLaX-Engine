#pragma once

#include "Core/Base.h"
#include "Core/Timestep.h"
#include "Renderer/OrthographicCamera.h"
#include "Entity.h"
#include <entt/entt.hpp>
#include <string>

// Forward-declare Box2D world to avoid including box2d headers everywhere
class b2World;

namespace LLaX
{
    class Scene
    {
    public:
        Scene();
        ~Scene();

        Entity CreateEntity(const std::string& name = "Entity");
        void   DestroyEntity(Entity entity);
        Entity DuplicateEntity(Entity entity);

        void OnUpdateEditor(Timestep ts, OrthographicCamera& camera);
        void OnUpdateRuntime(Timestep ts);
        void OnViewportResize(uint32_t width, uint32_t height);

        // Physics lifecycle (called by EditorLayer on Play/Stop)
        void OnPhysics2DStart();
        void OnPhysics2DStop();

        Entity GetPrimaryCameraEntity();

        template<typename... Components>
        auto GetAllEntitiesWith()
        {
            return m_Registry.view<Components...>();
        }

        entt::registry& Reg() { return m_Registry; }
        const entt::registry& Reg() const { return m_Registry; }

        static Ref<Scene> Copy(Ref<Scene> other);

    private:
        entt::registry m_Registry;
        uint32_t       m_ViewportWidth  = 0;
        uint32_t       m_ViewportHeight = 0;

        // Box2D physics world (null when not in play mode)
        b2World*       m_PhysicsWorld   = nullptr;

        friend class Entity;
        friend class SceneHierarchyPanel;
        friend class SceneSerializer;
    };

    template<typename T, typename... Args>
    T& Entity::AddComponent(Args&&... args)
    {
        LLAX_CORE_ASSERT(!HasComponent<T>(), "Entity already has component!");
        return m_Scene->m_Registry.emplace<T>(m_EntityHandle, std::forward<Args>(args)...);
    }

    template<typename T>
    T& Entity::GetComponent()
    {
        LLAX_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        return m_Scene->m_Registry.get<T>(m_EntityHandle);
    }

    template<typename T>
    const T& Entity::GetComponent() const
    {
        LLAX_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        return m_Scene->m_Registry.get<T>(m_EntityHandle);
    }

    template<typename T>
    bool Entity::HasComponent() const
    {
        return m_Scene->m_Registry.all_of<T>(m_EntityHandle);
    }

    template<typename T>
    void Entity::RemoveComponent()
    {
        LLAX_CORE_ASSERT(HasComponent<T>(), "Entity does not have component!");
        m_Scene->m_Registry.remove<T>(m_EntityHandle);
    }
}

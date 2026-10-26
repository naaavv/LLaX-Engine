#pragma once

#include "Entity.h"
#include "Core/Timestep.h"

namespace LLaX
{
    class ScriptableEntity
    {
    public:
        virtual ~ScriptableEntity() = default;

        template<typename T>
        T& GetComponent()
        {
            return m_Entity.GetComponent<T>();
        }

        template<typename T>
        bool HasComponent() const
        {
            return m_Entity.HasComponent<T>();
        }

    protected:
        virtual void OnCreate() {}
        virtual void OnDestroy() {}
        virtual void OnUpdate(Timestep ts) {}

    private:
        Entity m_Entity;
        friend class Scene;
    };
}

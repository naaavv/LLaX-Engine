#pragma once

#include "Core/Base.h"
#include "Core/Timestep.h"
#include <glm/glm.hpp>
#include <vector>

namespace LLaX
{
    // ─────────────────────────────────────────────────────────────────────────
    //  Internal particle state (not a component — managed by ParticleSystem)
    // ─────────────────────────────────────────────────────────────────────────
    struct Particle
    {
        glm::vec2 Position   { 0.0f };
        glm::vec2 Velocity   { 0.0f };
        glm::vec4 ColorBegin { 1.0f };
        glm::vec4 ColorEnd   { 0.0f };
        float     SizeBegin  = 0.1f;
        float     SizeEnd    = 0.01f;
        float     Lifetime   = 1.0f;
        float     LifeRemaining = 1.0f;
        bool      Active     = false;
    };

    // ─────────────────────────────────────────────────────────────────────────
    //  ParticleSystem  — global pool of CPU particles
    // ─────────────────────────────────────────────────────────────────────────
    class ParticleSystem
    {
    public:
        static void Init(uint32_t maxParticles = 10000);
        static void Shutdown();

        // Emit one particle with the given parameters from world-space origin
        static void Emit(
            glm::vec2 origin,
            glm::vec2 velocity,
            glm::vec4 colorBegin,
            glm::vec4 colorEnd,
            float sizeBegin,
            float sizeEnd,
            float lifetime
        );

        static void OnUpdate(Timestep ts);
        static void OnRender();

        static uint32_t GetActiveCount();

    private:
        static std::vector<Particle> s_Pool;
        static uint32_t              s_PoolIndex; // ring-buffer head
    };
}

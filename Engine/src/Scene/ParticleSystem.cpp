#include "ParticleSystem.h"
#include "Renderer/Renderer2D.h"
#include "Core/Log.h"
#include <glm/gtc/matrix_transform.hpp>
#include <random>

namespace LLaX
{
    // ─── Static data ─────────────────────────────────────────────────────────
    std::vector<Particle> ParticleSystem::s_Pool;
    uint32_t              ParticleSystem::s_PoolIndex = 0;

    // ─── Random helpers ───────────────────────────────────────────────────────
    static std::mt19937                          s_RandEngine{ std::random_device{}() };
    static std::uniform_real_distribution<float> s_Dist{ 0.0f, 1.0f };

    static float RandFloat() { return s_Dist(s_RandEngine); }

    // ─── Init / Shutdown ─────────────────────────────────────────────────────
    void ParticleSystem::Init(uint32_t maxParticles)
    {
        s_Pool.resize(maxParticles);
        s_PoolIndex = maxParticles - 1;
        LLAX_CORE_INFO("[Particles] Pool initialised with {0} slots.", maxParticles);
    }

    void ParticleSystem::Shutdown()
    {
        s_Pool.clear();
        LLAX_CORE_INFO("[Particles] Particle system shut down.");
    }

    // ─── Emit ────────────────────────────────────────────────────────────────
    void ParticleSystem::Emit(
        glm::vec2 origin,
        glm::vec2 velocity,
        glm::vec4 colorBegin,
        glm::vec4 colorEnd,
        float     sizeBegin,
        float     sizeEnd,
        float     lifetime)
    {
        if (s_Pool.empty()) return;

        Particle& p = s_Pool[s_PoolIndex];
        s_PoolIndex  = (s_PoolIndex == 0) ? (uint32_t)(s_Pool.size() - 1) : s_PoolIndex - 1;

        p.Active         = true;
        p.Position       = origin;
        p.Velocity       = velocity;
        p.ColorBegin     = colorBegin;
        p.ColorEnd       = colorEnd;
        p.SizeBegin      = sizeBegin;
        p.SizeEnd        = sizeEnd;
        p.Lifetime       = lifetime;
        p.LifeRemaining  = lifetime;
    }

    // ─── Update ──────────────────────────────────────────────────────────────
    void ParticleSystem::OnUpdate(Timestep ts)
    {
        float dt = ts.GetSeconds();
        for (auto& p : s_Pool)
        {
            if (!p.Active) continue;

            p.LifeRemaining -= dt;
            if (p.LifeRemaining <= 0.0f)
            {
                p.Active = false;
                continue;
            }

            p.Position += p.Velocity * dt;
        }
    }

    // ─── Render ──────────────────────────────────────────────────────────────
    void ParticleSystem::OnRender()
    {
        for (auto& p : s_Pool)
        {
            if (!p.Active) continue;

            float life   = p.LifeRemaining / p.Lifetime;  // 1→0
            glm::vec4 color = glm::mix(p.ColorEnd, p.ColorBegin, life);
            float size   = glm::mix(p.SizeEnd, p.SizeBegin, life);

            glm::mat4 transform = glm::translate(glm::mat4(1.0f), { p.Position.x, p.Position.y, 0.05f })
                                * glm::scale(glm::mat4(1.0f), { size, size, 1.0f });

            Renderer2D::DrawQuad(transform, color);
        }
    }

    // ─── Stats ───────────────────────────────────────────────────────────────
    uint32_t ParticleSystem::GetActiveCount()
    {
        uint32_t count = 0;
        for (auto& p : s_Pool)
            if (p.Active) count++;
        return count;
    }
}

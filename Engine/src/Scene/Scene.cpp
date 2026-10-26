#include "Scene.h"
#include "Components.h"
#include "ScriptableEntity.h"
#include "ParticleSystem.h"
#include "Renderer/Renderer2D.h"
#include "Audio/AudioSystem.h"
#include "Core/Log.h"
#include <glm/glm.hpp>

// Box2D
#include <box2d/b2_world.h>
#include <box2d/b2_body.h>
#include <box2d/b2_fixture.h>
#include <box2d/b2_polygon_shape.h>
#include <box2d/b2_circle_shape.h>

namespace LLaX
{
    // ─── Helpers ──────────────────────────────────────────────────────────────
    static b2BodyType Rigidbody2DTypeToBox2DType(Rigidbody2DComponent::BodyType type)
    {
        switch (type)
        {
            case Rigidbody2DComponent::BodyType::Static:    return b2_staticBody;
            case Rigidbody2DComponent::BodyType::Dynamic:   return b2_dynamicBody;
            case Rigidbody2DComponent::BodyType::Kinematic: return b2_kinematicBody;
        }
        LLAX_CORE_ASSERT(false, "Unknown body type");
        return b2_staticBody;
    }

    // ─── Scene lifecycle ─────────────────────────────────────────────────────
    Scene::Scene()
    {
    }

    Scene::~Scene()
    {
        OnPhysics2DStop();
    }

    // ─── Entity management ───────────────────────────────────────────────────
    Entity Scene::CreateEntity(const std::string& name)
    {
        Entity entity = { m_Registry.create(), this };
        entity.AddComponent<TransformComponent>();
        auto& tag = entity.AddComponent<TagComponent>();
        tag.Tag = name.empty() ? "Entity" : name;
        return entity;
    }

    void Scene::DestroyEntity(Entity entity)
    {
        m_Registry.destroy(entity);
    }

    Entity Scene::DuplicateEntity(Entity entity)
    {
        std::string name = entity.GetComponent<TagComponent>().Tag;
        Entity newEntity = CreateEntity(name + " (Copy)");

        if (entity.HasComponent<TransformComponent>())
            newEntity.GetComponent<TransformComponent>() = entity.GetComponent<TransformComponent>();
        if (entity.HasComponent<SpriteRendererComponent>())
            newEntity.AddComponent<SpriteRendererComponent>(entity.GetComponent<SpriteRendererComponent>());
        if (entity.HasComponent<CameraComponent>())
            newEntity.AddComponent<CameraComponent>(entity.GetComponent<CameraComponent>());
        if (entity.HasComponent<Rigidbody2DComponent>())
            newEntity.AddComponent<Rigidbody2DComponent>(entity.GetComponent<Rigidbody2DComponent>());
        if (entity.HasComponent<BoxCollider2DComponent>())
            newEntity.AddComponent<BoxCollider2DComponent>(entity.GetComponent<BoxCollider2DComponent>());
        if (entity.HasComponent<CircleCollider2DComponent>())
            newEntity.AddComponent<CircleCollider2DComponent>(entity.GetComponent<CircleCollider2DComponent>());
        if (entity.HasComponent<AudioSourceComponent>())
            newEntity.AddComponent<AudioSourceComponent>(entity.GetComponent<AudioSourceComponent>());
        if (entity.HasComponent<ParticleEmitterComponent>())
            newEntity.AddComponent<ParticleEmitterComponent>(entity.GetComponent<ParticleEmitterComponent>());
        if (entity.HasComponent<NativeScriptComponent>())
            newEntity.AddComponent<NativeScriptComponent>(entity.GetComponent<NativeScriptComponent>());

        return newEntity;
    }

    // ─── Physics2D lifecycle ─────────────────────────────────────────────────
    void Scene::OnPhysics2DStart()
    {
        m_PhysicsWorld = new b2World({ 0.0f, -9.81f });
        LLAX_CORE_INFO("[Physics2D] World created (gravity = -9.81).");

        // Create b2Body for every Rigidbody2D entity
        auto view = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
        for (auto e : view)
        {
            Entity entity = { e, this };
            auto& tf  = entity.GetComponent<TransformComponent>();
            auto& rb  = entity.GetComponent<Rigidbody2DComponent>();

            b2BodyDef bodyDef;
            bodyDef.type     = Rigidbody2DTypeToBox2DType(rb.Type);
            bodyDef.position = { tf.Translation.x, tf.Translation.y };
            bodyDef.angle    = tf.Rotation.z;
            bodyDef.fixedRotation = rb.FixedRotation;

            b2Body* body = m_PhysicsWorld->CreateBody(&bodyDef);
            rb.RuntimeBody = body;

            // BoxCollider
            if (entity.HasComponent<BoxCollider2DComponent>())
            {
                auto& bc = entity.GetComponent<BoxCollider2DComponent>();
                b2PolygonShape boxShape;
                boxShape.SetAsBox(
                    bc.Size.x * tf.Scale.x,
                    bc.Size.y * tf.Scale.y,
                    { bc.Offset.x, bc.Offset.y },
                    0.0f
                );

                b2FixtureDef fixtureDef;
                fixtureDef.shape       = &boxShape;
                fixtureDef.density     = bc.Density;
                fixtureDef.friction    = bc.Friction;
                fixtureDef.restitution = bc.Restitution;
                bc.RuntimeFixture = body->CreateFixture(&fixtureDef);
            }

            // CircleCollider
            if (entity.HasComponent<CircleCollider2DComponent>())
            {
                auto& cc = entity.GetComponent<CircleCollider2DComponent>();
                b2CircleShape circleShape;
                circleShape.m_p.Set(cc.Offset.x, cc.Offset.y);
                circleShape.m_radius = cc.Radius * tf.Scale.x;

                b2FixtureDef fixtureDef;
                fixtureDef.shape       = &circleShape;
                fixtureDef.density     = cc.Density;
                fixtureDef.friction    = cc.Friction;
                fixtureDef.restitution = cc.Restitution;
                cc.RuntimeFixture = body->CreateFixture(&fixtureDef);
            }
        }

        // Play-on-awake audio
        auto audioView = m_Registry.view<AudioSourceComponent>();
        for (auto e : audioView)
        {
            auto& audio = audioView.get<AudioSourceComponent>(e);
            if (audio.PlayOnAwake && !audio.AudioPath.empty() && !audio.Mute)
            {
                AudioSystem::Play(audio.AudioPath, audio.Volume, audio.Loop);
                audio._Playing = true;
            }
        }
    }

    void Scene::OnPhysics2DStop()
    {
        // Destroy Native Scripts
        {
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
                if (nsc.Instance)
                {
                    nsc.Instance->OnDestroy();
                    nsc.DestroyScript(&nsc);
                }
            });
        }

        if (m_PhysicsWorld)
        {
            delete m_PhysicsWorld;
            m_PhysicsWorld = nullptr;
            LLAX_CORE_INFO("[Physics2D] World destroyed.");
        }

        AudioSystem::StopAll();

        // Clear runtime handles
        auto rbView = m_Registry.view<Rigidbody2DComponent>();
        for (auto e : rbView)
            rbView.get<Rigidbody2DComponent>(e).RuntimeBody = nullptr;

        auto bcView = m_Registry.view<BoxCollider2DComponent>();
        for (auto e : bcView)
            bcView.get<BoxCollider2DComponent>(e).RuntimeFixture = nullptr;

        auto ccView = m_Registry.view<CircleCollider2DComponent>();
        for (auto e : ccView)
            ccView.get<CircleCollider2DComponent>(e).RuntimeFixture = nullptr;
    }

    // ─── Editor update ───────────────────────────────────────────────────────
    void Scene::OnUpdateEditor(Timestep ts, OrthographicCamera& camera)
    {
        Renderer2D::BeginScene(camera);

        // ── Unity-style Editor Background Grid Lines ──────────────────────────
        {
            glm::vec4 gridColor(0.24f, 0.26f, 0.28f, 0.45f);
            glm::vec4 axisColorX(0.85f, 0.25f, 0.25f, 0.65f); // Red X axis line
            glm::vec4 axisColorY(0.25f, 0.85f, 0.35f, 0.65f); // Green Y axis line

            const float gridExtent = 40.0f;
            const float lineThickness = 0.02f;

            // Minor grid lines
            for (float x = -gridExtent; x <= gridExtent; x += 1.0f)
            {
                if (std::abs(x) < 0.01f) continue;
                Renderer2D::DrawQuad({ x, 0.0f, -0.9f }, { lineThickness, gridExtent * 2.0f }, gridColor);
            }
            for (float y = -gridExtent; y <= gridExtent; y += 1.0f)
            {
                if (std::abs(y) < 0.01f) continue;
                Renderer2D::DrawQuad({ 0.0f, y, -0.9f }, { gridExtent * 2.0f, lineThickness }, gridColor);
            }

            // Origin Axes
            Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.85f }, { gridExtent * 2.0f, lineThickness * 1.8f }, axisColorX);
            Renderer2D::DrawQuad({ 0.0f, 0.0f, -0.85f }, { lineThickness * 1.8f, gridExtent * 2.0f }, axisColorY);
        }

        // Sprites
        auto view = m_Registry.view<TransformComponent, SpriteRendererComponent>();
        for (auto entity : view)
        {
            auto [transform, sprite] = view.get<TransformComponent, SpriteRendererComponent>(entity);
            if (sprite.Texture)
                Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TilingFactor, sprite.Color);
            else
                Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
        }

        // Particles (preview in editor too)
        ParticleSystem::OnUpdate(ts);
        ParticleSystem::OnRender();

        Renderer2D::EndScene();
    }

    // ─── Runtime update ──────────────────────────────────────────────────────
    void Scene::OnUpdateRuntime(Timestep ts)
    {
        // 1. Update Native Scripts
        {
            m_Registry.view<NativeScriptComponent>().each([=](auto entity, auto& nsc) {
                if (!nsc.Instance)
                {
                    nsc.Instance = nsc.InstantiateScript();
                    nsc.Instance->m_Entity = Entity{ entity, this };
                    nsc.Instance->OnCreate();
                }

                nsc.Instance->OnUpdate(ts);
            });
        }

        // 2. Physics step
        if (m_PhysicsWorld)
        {
            const int32_t velocityIter = 6;
            const int32_t positionIter = 2;
            m_PhysicsWorld->Step(ts.GetSeconds(), velocityIter, positionIter);

            // Sync transforms back from Box2D
            auto rbView = m_Registry.view<TransformComponent, Rigidbody2DComponent>();
            for (auto e : rbView)
            {
                auto [tf, rb] = rbView.get<TransformComponent, Rigidbody2DComponent>(e);
                if (rb.RuntimeBody)
                {
                    b2Body* body = static_cast<b2Body*>(rb.RuntimeBody);
                    const auto& pos = body->GetPosition();
                    tf.Translation.x = pos.x;
                    tf.Translation.y = pos.y;
                    tf.Rotation.z    = body->GetAngle();
                }
            }
        }

        // 2. Particle emitters
        {
            auto emitterView = m_Registry.view<TransformComponent, ParticleEmitterComponent>();
            for (auto e : emitterView)
            {
                auto [tf, em] = emitterView.get<TransformComponent, ParticleEmitterComponent>(e);
                if (!em.Active) continue;

                em._Accumulator += em.EmissionRate * ts.GetSeconds();
                int toEmit = (int)em._Accumulator;
                em._Accumulator -= (float)toEmit;

                for (int i = 0; i < toEmit; i++)
                {
                    auto randRange = [](float mn, float mx) -> float {
                        return mn + ((float)rand() / RAND_MAX) * (mx - mn);
                    };
                    glm::vec2 vel = {
                        randRange(em.VelocityMin.x, em.VelocityMax.x),
                        randRange(em.VelocityMin.y, em.VelocityMax.y)
                    };
                    float life    = randRange(em.LifetimeMin, em.LifetimeMax);
                    float size    = em.SizeBegin + randRange(-em.SizeVariation, em.SizeVariation);

                    ParticleSystem::Emit(
                        { tf.Translation.x, tf.Translation.y },
                        vel,
                        em.ColorBegin, em.ColorEnd,
                        size, em.SizeEnd,
                        life
                    );
                }
            }
            ParticleSystem::OnUpdate(ts);
        }

        // 3. Find primary camera and render
        CameraComponent* mainCamera      = nullptr;
        glm::mat4        cameraTransform = glm::mat4(1.0f);

        {
            auto camView = m_Registry.view<TransformComponent, CameraComponent>();
            for (auto entity : camView)
            {
                auto [transform, camera] = camView.get<TransformComponent, CameraComponent>(entity);
                if (camera.Primary)
                {
                    mainCamera      = &camera;
                    cameraTransform = transform.GetTransform();
                    break;
                }
            }
        }

        if (mainCamera)
        {
            Renderer2D::BeginScene(mainCamera->GetProjection(), cameraTransform);

            // Sprites
            auto sprites = m_Registry.view<TransformComponent, SpriteRendererComponent>();
            for (auto entity : sprites)
            {
                auto [transform, sprite] = sprites.get<TransformComponent, SpriteRendererComponent>(entity);
                if (sprite.Texture)
                    Renderer2D::DrawQuad(transform.GetTransform(), sprite.Texture, sprite.TilingFactor, sprite.Color);
                else
                    Renderer2D::DrawQuad(transform.GetTransform(), sprite.Color);
            }

            // Particles
            ParticleSystem::OnRender();

            Renderer2D::EndScene();
        }
    }

    // ─── Viewport resize ─────────────────────────────────────────────────────
    void Scene::OnViewportResize(uint32_t width, uint32_t height)
    {
        m_ViewportWidth  = width;
        m_ViewportHeight = height;

        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            auto& cameraComponent = view.get<CameraComponent>(entity);
            if (!cameraComponent.FixedAspectRatio)
                cameraComponent.SetViewportSize(width, height);
        }
    }

    // ─── Primary camera ──────────────────────────────────────────────────────
    Entity Scene::GetPrimaryCameraEntity()
    {
        auto view = m_Registry.view<CameraComponent>();
        for (auto entity : view)
        {
            const auto& camera = view.get<CameraComponent>(entity);
            if (camera.Primary)
                return Entity{ entity, this };
        }
        return {};
    }

    // ─── Scene Copy ──────────────────────────────────────────────────────────
    Ref<Scene> Scene::Copy(Ref<Scene> other)
    {
        Ref<Scene> newScene = CreateRef<Scene>();
        newScene->m_ViewportWidth  = other->m_ViewportWidth;
        newScene->m_ViewportHeight = other->m_ViewportHeight;

        auto& srcReg = other->m_Registry;

        auto view = srcReg.view<TagComponent>();
        for (auto e : view)
        {
            Entity srcEntity = { e, other.get() };
            Entity dstEntity = newScene->CreateEntity(srcEntity.GetComponent<TagComponent>().Tag);

            if (srcEntity.HasComponent<TransformComponent>())
                dstEntity.GetComponent<TransformComponent>() = srcEntity.GetComponent<TransformComponent>();
            if (srcEntity.HasComponent<SpriteRendererComponent>())
                dstEntity.AddComponent<SpriteRendererComponent>(srcEntity.GetComponent<SpriteRendererComponent>());
            if (srcEntity.HasComponent<CameraComponent>())
                dstEntity.AddComponent<CameraComponent>(srcEntity.GetComponent<CameraComponent>());
            if (srcEntity.HasComponent<Rigidbody2DComponent>())
                dstEntity.AddComponent<Rigidbody2DComponent>(srcEntity.GetComponent<Rigidbody2DComponent>());
            if (srcEntity.HasComponent<BoxCollider2DComponent>())
                dstEntity.AddComponent<BoxCollider2DComponent>(srcEntity.GetComponent<BoxCollider2DComponent>());
            if (srcEntity.HasComponent<CircleCollider2DComponent>())
                dstEntity.AddComponent<CircleCollider2DComponent>(srcEntity.GetComponent<CircleCollider2DComponent>());
            if (srcEntity.HasComponent<AudioSourceComponent>())
                dstEntity.AddComponent<AudioSourceComponent>(srcEntity.GetComponent<AudioSourceComponent>());
            if (srcEntity.HasComponent<ParticleEmitterComponent>())
                dstEntity.AddComponent<ParticleEmitterComponent>(srcEntity.GetComponent<ParticleEmitterComponent>());
            if (srcEntity.HasComponent<NativeScriptComponent>())
                dstEntity.AddComponent<NativeScriptComponent>(srcEntity.GetComponent<NativeScriptComponent>());
        }

        return newScene;
    }

} // namespace LLaX

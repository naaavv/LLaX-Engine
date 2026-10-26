#pragma once

#include "Core/Base.h"
#include "Renderer/Texture.h"

#include <string>
#include <functional>
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#define GLM_ENABLE_EXPERIMENTAL
#include <glm/gtx/quaternion.hpp>

namespace LLaX
{
    // ─── Tag ─────────────────────────────────────────────────────────────────
    struct TagComponent
    {
        std::string Tag;

        TagComponent() = default;
        TagComponent(const TagComponent&) = default;
        TagComponent(const std::string& tag)
            : Tag(tag) {}
    };

    // ─── Transform ───────────────────────────────────────────────────────────
    struct TransformComponent
    {
        glm::vec3 Translation{ 0.0f, 0.0f, 0.0f };
        glm::vec3 Rotation{ 0.0f, 0.0f, 0.0f }; // Radians
        glm::vec3 Scale{ 1.0f, 1.0f, 1.0f };

        TransformComponent() = default;
        TransformComponent(const TransformComponent&) = default;
        TransformComponent(const glm::vec3& translation)
            : Translation(translation) {}

        glm::mat4 GetTransform() const
        {
            glm::mat4 rotation = glm::toMat4(glm::quat(Rotation));

            return glm::translate(glm::mat4(1.0f), Translation)
                * rotation
                * glm::scale(glm::mat4(1.0f), Scale);
        }
    };

    // ─── Sprite Renderer ─────────────────────────────────────────────────────
    struct SpriteRendererComponent
    {
        glm::vec4 Color{ 1.0f, 1.0f, 1.0f, 1.0f };
        Ref<Texture2D> Texture = nullptr;
        float TilingFactor = 1.0f;

        SpriteRendererComponent() = default;
        SpriteRendererComponent(const SpriteRendererComponent&) = default;
        SpriteRendererComponent(const glm::vec4& color)
            : Color(color) {}
        SpriteRendererComponent(const Ref<Texture2D>& texture, const glm::vec4& tint = glm::vec4(1.0f))
            : Texture(texture), Color(tint) {}
    };

    // ─── Camera ──────────────────────────────────────────────────────────────
    struct CameraComponent
    {
        bool Primary = true;
        bool FixedAspectRatio = false;

        float OrthographicSize = 10.0f;
        float OrthographicNear = -1.0f;
        float OrthographicFar = 1.0f;
        float AspectRatio = 16.0f / 9.0f;

        CameraComponent() = default;
        CameraComponent(const CameraComponent&) = default;

        glm::mat4 GetProjection() const
        {
            float orthoLeft   = -OrthographicSize * AspectRatio * 0.5f;
            float orthoRight  =  OrthographicSize * AspectRatio * 0.5f;
            float orthoBottom = -OrthographicSize * 0.5f;
            float orthoTop    =  OrthographicSize * 0.5f;

            return glm::ortho(orthoLeft, orthoRight, orthoBottom, orthoTop, OrthographicNear, OrthographicFar);
        }

        void SetViewportSize(uint32_t width, uint32_t height)
        {
            if (width > 0 && height > 0)
                AspectRatio = (float)width / (float)height;
        }
    };

    // ─── Physics: Rigidbody2D ─────────────────────────────────────────────────
    struct Rigidbody2DComponent
    {
        enum class BodyType { Static = 0, Dynamic, Kinematic };

        BodyType Type           = BodyType::Static;
        bool     FixedRotation  = false;

        // Runtime body handle (Box2D b2Body*) — not serialized
        void* RuntimeBody = nullptr;

        Rigidbody2DComponent() = default;
        Rigidbody2DComponent(const Rigidbody2DComponent&) = default;
    };

    // ─── Physics: BoxCollider2D ───────────────────────────────────────────────
    struct BoxCollider2DComponent
    {
        glm::vec2 Offset  { 0.0f, 0.0f };
        glm::vec2 Size    { 0.5f, 0.5f };  // half-extents

        float Density     = 1.0f;
        float Friction    = 0.5f;
        float Restitution = 0.0f;

        // Runtime fixture handle — not serialized
        void* RuntimeFixture = nullptr;

        BoxCollider2DComponent() = default;
        BoxCollider2DComponent(const BoxCollider2DComponent&) = default;
    };

    // ─── Physics: CircleCollider2D ────────────────────────────────────────────
    struct CircleCollider2DComponent
    {
        glm::vec2 Offset { 0.0f, 0.0f };
        float     Radius  = 0.5f;

        float Density     = 1.0f;
        float Friction    = 0.5f;
        float Restitution = 0.0f;

        // Runtime fixture handle — not serialized
        void* RuntimeFixture = nullptr;

        CircleCollider2DComponent() = default;
        CircleCollider2DComponent(const CircleCollider2DComponent&) = default;
    };

    // ─── Audio: AudioSource ───────────────────────────────────────────────────
    struct AudioSourceComponent
    {
        std::string AudioPath;       // relative path, e.g. "Assets/audio/jump.wav"
        float       Volume      = 1.0f;
        bool        Loop        = false;
        bool        PlayOnAwake = false;
        bool        Mute        = false;

        // Runtime state (not serialized)
        bool _Playing = false;

        AudioSourceComponent() = default;
        AudioSourceComponent(const AudioSourceComponent&) = default;
    };

    // ─── Native Scripting (C++) ──────────────────────────────────────────────
    class ScriptableEntity;

    struct NativeScriptComponent
    {
        ScriptableEntity* Instance = nullptr;

        ScriptableEntity* (*InstantiateScript)() = nullptr;
        void (*DestroyScript)(NativeScriptComponent*) = nullptr;

        template<typename T>
        void Bind()
        {
            InstantiateScript = []() { return static_cast<ScriptableEntity*>(new T()); };
            DestroyScript = [](NativeScriptComponent* nsc) { delete nsc->Instance; nsc->Instance = nullptr; };
        }
    };

    // ─── Particles: ParticleEmitter ───────────────────────────────────────────
    struct ParticleEmitterComponent
    {
        bool  Active          = true;
        bool  Looping         = true;
        float EmissionRate    = 20.0f;   // particles per second
        float Duration        = 5.0f;   // total duration (if not looping)

        // Lifetime
        float LifetimeMin     = 0.8f;
        float LifetimeMax     = 1.5f;

        // Velocity (local space, random in range)
        glm::vec2 VelocityMin { -1.5f, 1.0f };
        glm::vec2 VelocityMax {  1.5f, 3.0f };

        // Size over lifetime
        float SizeBegin       = 0.15f;
        float SizeEnd         = 0.02f;
        float SizeVariation   = 0.05f;

        // Colour over lifetime
        glm::vec4 ColorBegin  { 1.0f, 0.6f, 0.1f, 1.0f };
        glm::vec4 ColorEnd    { 0.9f, 0.1f, 0.0f, 0.0f };

        // Gravity scale applied to each particle
        float GravityScale    = 0.0f;

        // Runtime: accumulated fractional particles
        float _Accumulator    = 0.0f;

        ParticleEmitterComponent() = default;
        ParticleEmitterComponent(const ParticleEmitterComponent&) = default;
    };

} // namespace LLaX

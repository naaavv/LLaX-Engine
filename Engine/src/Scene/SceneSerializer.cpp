#include "SceneSerializer.h"
#include "Components.h"
#include "Entity.h"
#include "Core/Log.h"

#include <yaml-cpp/yaml.h>
#include <fstream>
#include <filesystem>

namespace YAML
{
    template<>
    struct convert<glm::vec2>
    {
        static Node encode(const glm::vec2& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec2& rhs)
        {
            if (!node.IsSequence() || node.size() != 2)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec3>
    {
        static Node encode(const glm::vec3& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec3& rhs)
        {
            if (!node.IsSequence() || node.size() != 3)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            return true;
        }
    };

    template<>
    struct convert<glm::vec4>
    {
        static Node encode(const glm::vec4& rhs)
        {
            Node node;
            node.push_back(rhs.x);
            node.push_back(rhs.y);
            node.push_back(rhs.z);
            node.push_back(rhs.w);
            node.SetStyle(EmitterStyle::Flow);
            return node;
        }

        static bool decode(const Node& node, glm::vec4& rhs)
        {
            if (!node.IsSequence() || node.size() != 4)
                return false;

            rhs.x = node[0].as<float>();
            rhs.y = node[1].as<float>();
            rhs.z = node[2].as<float>();
            rhs.w = node[3].as<float>();
            return true;
        }
    };
}

namespace LLaX
{
    static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec2& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << YAML::EndSeq;
        return out;
    }

    static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec3& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << YAML::EndSeq;
        return out;
    }

    static YAML::Emitter& operator<<(YAML::Emitter& out, const glm::vec4& v)
    {
        out << YAML::Flow;
        out << YAML::BeginSeq << v.x << v.y << v.z << v.w << YAML::EndSeq;
        return out;
    }

    SceneSerializer::SceneSerializer(const Ref<Scene>& scene)
        : m_Scene(scene)
    {
    }

    static void SerializeEntity(YAML::Emitter& out, Entity entity)
    {
        out << YAML::BeginMap; // Entity
        out << YAML::Key << "Entity" << YAML::Value << (uint32_t)entity;

        if (entity.HasComponent<TagComponent>())
        {
            out << YAML::Key << "TagComponent";
            out << YAML::BeginMap; // TagComponent
            auto& tag = entity.GetComponent<TagComponent>().Tag;
            out << YAML::Key << "Tag" << YAML::Value << tag;
            out << YAML::EndMap; // TagComponent
        }

        if (entity.HasComponent<TransformComponent>())
        {
            out << YAML::Key << "TransformComponent";
            out << YAML::BeginMap; // TransformComponent
            auto& tc = entity.GetComponent<TransformComponent>();
            out << YAML::Key << "Translation" << YAML::Value << tc.Translation;
            out << YAML::Key << "Rotation" << YAML::Value << tc.Rotation;
            out << YAML::Key << "Scale" << YAML::Value << tc.Scale;
            out << YAML::EndMap; // TransformComponent
        }

        if (entity.HasComponent<SpriteRendererComponent>())
        {
            out << YAML::Key << "SpriteRendererComponent";
            out << YAML::BeginMap; // SpriteRendererComponent
            auto& src = entity.GetComponent<SpriteRendererComponent>();
            out << YAML::Key << "Color" << YAML::Value << src.Color;
            out << YAML::Key << "TexturePath" << YAML::Value << (src.Texture ? src.Texture->GetPath() : "");
            out << YAML::Key << "TilingFactor" << YAML::Value << src.TilingFactor;
            out << YAML::EndMap; // SpriteRendererComponent
        }

        if (entity.HasComponent<CameraComponent>())
        {
            out << YAML::Key << "CameraComponent";
            out << YAML::BeginMap; // CameraComponent
            auto& cc = entity.GetComponent<CameraComponent>();
            out << YAML::Key << "Primary" << YAML::Value << cc.Primary;
            out << YAML::Key << "FixedAspectRatio" << YAML::Value << cc.FixedAspectRatio;
            out << YAML::Key << "OrthographicSize" << YAML::Value << cc.OrthographicSize;
            out << YAML::Key << "OrthographicNear" << YAML::Value << cc.OrthographicNear;
            out << YAML::Key << "OrthographicFar" << YAML::Value << cc.OrthographicFar;
            out << YAML::EndMap; // CameraComponent
        }

        if (entity.HasComponent<Rigidbody2DComponent>())
        {
            out << YAML::Key << "Rigidbody2DComponent";
            out << YAML::BeginMap;
            auto& rb2d = entity.GetComponent<Rigidbody2DComponent>();
            out << YAML::Key << "BodyType" << YAML::Value << (int)rb2d.Type;
            out << YAML::Key << "FixedRotation" << YAML::Value << rb2d.FixedRotation;
            out << YAML::EndMap;
        }

        if (entity.HasComponent<BoxCollider2DComponent>())
        {
            out << YAML::Key << "BoxCollider2DComponent";
            out << YAML::BeginMap;
            auto& bc2d = entity.GetComponent<BoxCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << bc2d.Offset;
            out << YAML::Key << "Size" << YAML::Value << bc2d.Size;
            out << YAML::Key << "Density" << YAML::Value << bc2d.Density;
            out << YAML::Key << "Friction" << YAML::Value << bc2d.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << bc2d.Restitution;
            out << YAML::EndMap;
        }

        if (entity.HasComponent<CircleCollider2DComponent>())
        {
            out << YAML::Key << "CircleCollider2DComponent";
            out << YAML::BeginMap;
            auto& cc2d = entity.GetComponent<CircleCollider2DComponent>();
            out << YAML::Key << "Offset" << YAML::Value << cc2d.Offset;
            out << YAML::Key << "Radius" << YAML::Value << cc2d.Radius;
            out << YAML::Key << "Density" << YAML::Value << cc2d.Density;
            out << YAML::Key << "Friction" << YAML::Value << cc2d.Friction;
            out << YAML::Key << "Restitution" << YAML::Value << cc2d.Restitution;
            out << YAML::EndMap;
        }

        if (entity.HasComponent<AudioSourceComponent>())
        {
            out << YAML::Key << "AudioSourceComponent";
            out << YAML::BeginMap;
            auto& audio = entity.GetComponent<AudioSourceComponent>();
            out << YAML::Key << "AudioPath" << YAML::Value << audio.AudioPath;
            out << YAML::Key << "Volume" << YAML::Value << audio.Volume;
            out << YAML::Key << "Loop" << YAML::Value << audio.Loop;
            out << YAML::Key << "PlayOnAwake" << YAML::Value << audio.PlayOnAwake;
            out << YAML::Key << "Mute" << YAML::Value << audio.Mute;
            out << YAML::EndMap;
        }

        if (entity.HasComponent<ParticleEmitterComponent>())
        {
            out << YAML::Key << "ParticleEmitterComponent";
            out << YAML::BeginMap;
            auto& pe = entity.GetComponent<ParticleEmitterComponent>();
            out << YAML::Key << "Active" << YAML::Value << pe.Active;
            out << YAML::Key << "EmissionRate" << YAML::Value << pe.EmissionRate;
            out << YAML::Key << "LifetimeMin" << YAML::Value << pe.LifetimeMin;
            out << YAML::Key << "LifetimeMax" << YAML::Value << pe.LifetimeMax;
            out << YAML::Key << "VelocityMin" << YAML::Value << pe.VelocityMin;
            out << YAML::Key << "VelocityMax" << YAML::Value << pe.VelocityMax;
            out << YAML::Key << "SizeBegin" << YAML::Value << pe.SizeBegin;
            out << YAML::Key << "SizeEnd" << YAML::Value << pe.SizeEnd;
            out << YAML::Key << "ColorBegin" << YAML::Value << pe.ColorBegin;
            out << YAML::Key << "ColorEnd" << YAML::Value << pe.ColorEnd;
            out << YAML::EndMap;
        }

        out << YAML::EndMap; // Entity
    }

    void SceneSerializer::Serialize(const std::string& filepath)
    {
        YAML::Emitter out;
        out << YAML::BeginMap;
        out << YAML::Key << "Scene" << YAML::Value << "Untitled Scene";
        out << YAML::Key << "Entities" << YAML::Value << YAML::BeginSeq;

        auto view = m_Scene->m_Registry.view<TagComponent>();
        for (auto entityID : view)
        {
            Entity entity = { entityID, m_Scene.get() };
            if (!entity)
                continue;

            SerializeEntity(out, entity);
        }

        out << YAML::EndSeq;
        out << YAML::EndMap;

        std::ofstream fout(filepath);
        fout << out.c_str();

        LLAX_CORE_INFO("Scene serialized successfully to '{0}'", filepath);
    }

    void SceneSerializer::SerializeRuntime(const std::string& filepath)
    {
        LLAX_CORE_ASSERT(false, "Not implemented!");
    }

    bool SceneSerializer::Deserialize(const std::string& filepath)
    {
        YAML::Node data;
        try
        {
            data = YAML::LoadFile(filepath);
        }
        catch (const YAML::ParserException& e)
        {
            LLAX_CORE_ERROR("Failed to load .llax file '{0}': {1}", filepath, e.what());
            return false;
        }

        if (!data["Scene"])
            return false;

        std::string sceneName = data["Scene"].as<std::string>();
        LLAX_CORE_INFO("Deserializing scene '{0}' from '{1}'", sceneName, filepath);

        auto entities = data["Entities"];
        if (entities)
        {
            for (auto entity : entities)
            {
                uint64_t id = entity["Entity"].as<uint64_t>();

                std::string name = "Entity";
                auto tagComponent = entity["TagComponent"];
                if (tagComponent)
                    name = tagComponent["Tag"].as<std::string>();

                Entity deserializedEntity = m_Scene->CreateEntity(name);

                auto transformComponent = entity["TransformComponent"];
                if (transformComponent)
                {
                    auto& tc = deserializedEntity.GetComponent<TransformComponent>();
                    tc.Translation = transformComponent["Translation"].as<glm::vec3>();
                    tc.Rotation = transformComponent["Rotation"].as<glm::vec3>();
                    tc.Scale = transformComponent["Scale"].as<glm::vec3>();
                }

                auto spriteRendererComponent = entity["SpriteRendererComponent"];
                if (spriteRendererComponent)
                {
                    auto& src = deserializedEntity.AddComponent<SpriteRendererComponent>();
                    src.Color = spriteRendererComponent["Color"].as<glm::vec4>();

                    if (spriteRendererComponent["TexturePath"])
                    {
                        std::string texPath = spriteRendererComponent["TexturePath"].as<std::string>();
                        if (!texPath.empty() && std::filesystem::exists(texPath))
                            src.Texture = Texture2D::Create(texPath);
                    }

                    if (spriteRendererComponent["TilingFactor"])
                        src.TilingFactor = spriteRendererComponent["TilingFactor"].as<float>();
                }

                auto cameraComponent = entity["CameraComponent"];
                if (cameraComponent)
                {
                    auto& cc = deserializedEntity.AddComponent<CameraComponent>();
                    if (cameraComponent["Primary"])
                        cc.Primary = cameraComponent["Primary"].as<bool>();
                    if (cameraComponent["FixedAspectRatio"])
                        cc.FixedAspectRatio = cameraComponent["FixedAspectRatio"].as<bool>();
                    if (cameraComponent["OrthographicSize"])
                        cc.OrthographicSize = cameraComponent["OrthographicSize"].as<float>();
                    if (cameraComponent["OrthographicNear"])
                        cc.OrthographicNear = cameraComponent["OrthographicNear"].as<float>();
                    if (cameraComponent["OrthographicFar"])
                        cc.OrthographicFar = cameraComponent["OrthographicFar"].as<float>();
                }

                auto rigidbody2DComponent = entity["Rigidbody2DComponent"];
                if (rigidbody2DComponent)
                {
                    auto& rb2d = deserializedEntity.AddComponent<Rigidbody2DComponent>();
                    if (rigidbody2DComponent["BodyType"])
                        rb2d.Type = (Rigidbody2DComponent::BodyType)rigidbody2DComponent["BodyType"].as<int>();
                    if (rigidbody2DComponent["FixedRotation"])
                        rb2d.FixedRotation = rigidbody2DComponent["FixedRotation"].as<bool>();
                }

                auto boxCollider2DComponent = entity["BoxCollider2DComponent"];
                if (boxCollider2DComponent)
                {
                    auto& bc2d = deserializedEntity.AddComponent<BoxCollider2DComponent>();
                    if (boxCollider2DComponent["Offset"])
                        bc2d.Offset = boxCollider2DComponent["Offset"].as<glm::vec2>();
                    if (boxCollider2DComponent["Size"])
                        bc2d.Size = boxCollider2DComponent["Size"].as<glm::vec2>();
                    if (boxCollider2DComponent["Density"])
                        bc2d.Density = boxCollider2DComponent["Density"].as<float>();
                    if (boxCollider2DComponent["Friction"])
                        bc2d.Friction = boxCollider2DComponent["Friction"].as<float>();
                    if (boxCollider2DComponent["Restitution"])
                        bc2d.Restitution = boxCollider2DComponent["Restitution"].as<float>();
                }

                auto circleCollider2DComponent = entity["CircleCollider2DComponent"];
                if (circleCollider2DComponent)
                {
                    auto& cc2d = deserializedEntity.AddComponent<CircleCollider2DComponent>();
                    if (circleCollider2DComponent["Offset"])
                        cc2d.Offset = circleCollider2DComponent["Offset"].as<glm::vec2>();
                    if (circleCollider2DComponent["Radius"])
                        cc2d.Radius = circleCollider2DComponent["Radius"].as<float>();
                    if (circleCollider2DComponent["Density"])
                        cc2d.Density = circleCollider2DComponent["Density"].as<float>();
                    if (circleCollider2DComponent["Friction"])
                        cc2d.Friction = circleCollider2DComponent["Friction"].as<float>();
                    if (circleCollider2DComponent["Restitution"])
                        cc2d.Restitution = circleCollider2DComponent["Restitution"].as<float>();
                }

                auto audioSourceComponent = entity["AudioSourceComponent"];
                if (audioSourceComponent)
                {
                    auto& audio = deserializedEntity.AddComponent<AudioSourceComponent>();
                    if (audioSourceComponent["AudioPath"])
                        audio.AudioPath = audioSourceComponent["AudioPath"].as<std::string>();
                    if (audioSourceComponent["Volume"])
                        audio.Volume = audioSourceComponent["Volume"].as<float>();
                    if (audioSourceComponent["Loop"])
                        audio.Loop = audioSourceComponent["Loop"].as<bool>();
                    if (audioSourceComponent["PlayOnAwake"])
                        audio.PlayOnAwake = audioSourceComponent["PlayOnAwake"].as<bool>();
                    if (audioSourceComponent["Mute"])
                        audio.Mute = audioSourceComponent["Mute"].as<bool>();
                }

                auto particleEmitterComponent = entity["ParticleEmitterComponent"];
                if (particleEmitterComponent)
                {
                    auto& pe = deserializedEntity.AddComponent<ParticleEmitterComponent>();
                    if (particleEmitterComponent["Active"])
                        pe.Active = particleEmitterComponent["Active"].as<bool>();
                    if (particleEmitterComponent["EmissionRate"])
                        pe.EmissionRate = particleEmitterComponent["EmissionRate"].as<float>();
                    if (particleEmitterComponent["LifetimeMin"])
                        pe.LifetimeMin = particleEmitterComponent["LifetimeMin"].as<float>();
                    if (particleEmitterComponent["LifetimeMax"])
                        pe.LifetimeMax = particleEmitterComponent["LifetimeMax"].as<float>();
                    if (particleEmitterComponent["VelocityMin"])
                        pe.VelocityMin = particleEmitterComponent["VelocityMin"].as<glm::vec2>();
                    if (particleEmitterComponent["VelocityMax"])
                        pe.VelocityMax = particleEmitterComponent["VelocityMax"].as<glm::vec2>();
                    if (particleEmitterComponent["SizeBegin"])
                        pe.SizeBegin = particleEmitterComponent["SizeBegin"].as<float>();
                    if (particleEmitterComponent["SizeEnd"])
                        pe.SizeEnd = particleEmitterComponent["SizeEnd"].as<float>();
                    if (particleEmitterComponent["ColorBegin"])
                        pe.ColorBegin = particleEmitterComponent["ColorBegin"].as<glm::vec4>();
                    if (particleEmitterComponent["ColorEnd"])
                        pe.ColorEnd = particleEmitterComponent["ColorEnd"].as<glm::vec4>();
                }
            }
        }

        LLAX_CORE_INFO("Scene deserialization completed successfully.");
        return true;
    }

    bool SceneSerializer::DeserializeRuntime(const std::string& filepath)
    {
        LLAX_CORE_ASSERT(false, "Not implemented!");
        return false;
    }
}

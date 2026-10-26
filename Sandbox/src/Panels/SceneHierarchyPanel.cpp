#include "SceneHierarchyPanel.h"
#include <imgui.h>
#include <imgui_internal.h>
#include <filesystem>

namespace LLaX
{
    SceneHierarchyPanel::SceneHierarchyPanel(const Ref<Scene>& context)
    {
        SetContext(context);
    }

    void SceneHierarchyPanel::SetContext(const Ref<Scene>& context)
    {
        m_Context = context;
        m_SelectionContext = {};
    }

    void SceneHierarchyPanel::OnImGuiRender()
    {
        // ─── Scene Hierarchy Window ──────────────────────────────────────────
        ImGui::Begin("Scene Hierarchy");

        if (m_Context)
        {
            // ── Header: entity count ─────────────────────────────────────────
            auto view = m_Context->m_Registry.view<TagComponent>();
            int entityCount = (int)view.size();

            ImGui::PushStyleColor(ImGuiCol_Text, ImVec4(0.42f, 0.66f, 1.0f, 1.0f));
            ImGui::Text("Entities (%d)", entityCount);
            ImGui::PopStyleColor();
            ImGui::Separator();
            ImGui::Spacing();

            for (auto entityID : view)
            {
                Entity entity{ entityID, m_Context.get() };
                DrawEntityNode(entity);
            }

            // Deselect when clicking on empty space
            if (ImGui::IsMouseDown(0) && ImGui::IsWindowHovered())
            {
                m_SelectionContext = {};
            }

            // Right-click blank area context menu
            if (ImGui::BeginPopupContextWindow(nullptr, ImGuiPopupFlags_MouseButtonRight | ImGuiPopupFlags_NoOpenOverItems))
            {
                if (ImGui::MenuItem("Create Empty Entity"))
                {
                    m_SelectionContext = m_Context->CreateEntity("Empty Entity");
                }
                if (ImGui::MenuItem("Create Sprite Entity"))
                {
                    Entity spriteEntity = m_Context->CreateEntity("Sprite");
                    spriteEntity.AddComponent<SpriteRendererComponent>();
                    m_SelectionContext = spriteEntity;
                }
                if (ImGui::MenuItem("Create Camera Entity"))
                {
                    Entity camEntity = m_Context->CreateEntity("Camera");
                    camEntity.AddComponent<CameraComponent>();
                    m_SelectionContext = camEntity;
                }

                ImGui::EndPopup();
            }
        }

        ImGui::End();

        // ─── Properties / Inspector Window ───────────────────────────────────
        ImGui::Begin("Properties");

        if (m_SelectionContext)
        {
            DrawComponents(m_SelectionContext);
        }
        else
        {
            ImGui::Spacing();
            float w = ImGui::GetContentRegionAvail().x;
            float tw = ImGui::CalcTextSize("No entity selected").x;
            ImGui::SetCursorPosX((w - tw) * 0.5f);
            ImGui::TextDisabled("No entity selected");
        }

        ImGui::End();
    }

    void SceneHierarchyPanel::DrawEntityNode(Entity entity)
    {
        auto& tag = entity.GetComponent<TagComponent>().Tag;

        ImGuiTreeNodeFlags flags = ((m_SelectionContext == entity) ? ImGuiTreeNodeFlags_Selected : 0) | ImGuiTreeNodeFlags_OpenOnArrow;
        flags |= ImGuiTreeNodeFlags_SpanAvailWidth;

        bool opened = ImGui::TreeNodeEx((void*)(uint64_t)(uint32_t)entity, flags, "%s", tag.c_str());
        if (ImGui::IsItemClicked())
        {
            m_SelectionContext = entity;
        }

        bool entityDeleted = false;
        bool entityDuplicated = false;

        if (ImGui::BeginPopupContextItem())
        {
            if (ImGui::MenuItem("Duplicate Entity"))
                entityDuplicated = true;

            if (ImGui::MenuItem("Delete Entity"))
                entityDeleted = true;

            ImGui::EndPopup();
        }

        if (opened)
        {
            ImGui::TreePop();
        }

        if (entityDuplicated)
        {
            m_SelectionContext = m_Context->DuplicateEntity(entity);
        }

        if (entityDeleted)
        {
            m_Context->DestroyEntity(entity);
            if (m_SelectionContext == entity)
                m_SelectionContext = {};
        }
    }

    static void DrawVec3Control(const std::string& label, glm::vec3& values, float resetValue = 0.0f, float columnWidth = 100.0f)
    {
        ImGuiIO& io = ImGui::GetIO();
        auto boldFont = io.Fonts->Fonts[0];

        ImGui::PushID(label.c_str());

        ImGui::Columns(2);
        ImGui::SetColumnWidth(0, columnWidth);
        ImGui::Text("%s", label.c_str());
        ImGui::NextColumn();

        ImGui::PushMultiItemsWidths(3, ImGui::CalcItemWidth());
        ImGui::PushStyleVar(ImGuiStyleVar_ItemSpacing, ImVec2{ 0, 0 });

        float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
        ImVec2 buttonSize = { lineHeight + 3.0f, lineHeight };

        // X Axis
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.9f, 0.2f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.8f, 0.1f, 0.15f, 1.0f });
        if (ImGui::Button("X", buttonSize))
            values.x = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##X", &values.x, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Y Axis
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.3f, 0.8f, 0.3f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.2f, 0.7f, 0.2f, 1.0f });
        if (ImGui::Button("Y", buttonSize))
            values.y = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Y", &values.y, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();
        ImGui::SameLine();

        // Z Axis
        ImGui::PushStyleColor(ImGuiCol_Button, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonHovered, ImVec4{ 0.2f, 0.35f, 0.9f, 1.0f });
        ImGui::PushStyleColor(ImGuiCol_ButtonActive, ImVec4{ 0.1f, 0.25f, 0.8f, 1.0f });
        if (ImGui::Button("Z", buttonSize))
            values.z = resetValue;
        ImGui::PopStyleColor(3);

        ImGui::SameLine();
        ImGui::DragFloat("##Z", &values.z, 0.1f, 0.0f, 0.0f, "%.2f");
        ImGui::PopItemWidth();

        ImGui::PopStyleVar();
        ImGui::Columns(1);
        ImGui::PopID();
    }

    template<typename T, typename UIFunction>
    static void DrawComponent(const std::string& name, Entity entity, UIFunction uiFunction, bool canDelete = true)
    {
        const ImGuiTreeNodeFlags treeNodeFlags = ImGuiTreeNodeFlags_DefaultOpen | ImGuiTreeNodeFlags_Framed |
                                                 ImGuiTreeNodeFlags_SpanAvailWidth | ImGuiTreeNodeFlags_AllowOverlap |
                                                 ImGuiTreeNodeFlags_FramePadding;

        if (entity.HasComponent<T>())
        {
            auto& component = entity.GetComponent<T>();
            ImVec2 contentRegionAvailable = ImGui::GetContentRegionAvail();

            ImGui::PushStyleVar(ImGuiStyleVar_FramePadding, ImVec2{ 4, 4 });
            float lineHeight = ImGui::GetFontSize() + ImGui::GetStyle().FramePadding.y * 2.0f;
            ImGui::Separator();
            bool open = ImGui::TreeNodeEx((void*)typeid(T).hash_code(), treeNodeFlags, "%s", name.c_str());
            ImGui::PopStyleVar();

            bool removeComponent = false;
            if (canDelete)
            {
                ImGui::SameLine(contentRegionAvailable.x - lineHeight * 0.5f);
                if (ImGui::Button("+", ImVec2{ lineHeight, lineHeight }))
                {
                    ImGui::OpenPopup("ComponentSettings");
                }

                if (ImGui::BeginPopup("ComponentSettings"))
                {
                    if (ImGui::MenuItem("Remove Component"))
                        removeComponent = true;

                    ImGui::EndPopup();
                }
            }

            if (open)
            {
                uiFunction(component);
                ImGui::TreePop();
            }

            if (removeComponent)
                entity.RemoveComponent<T>();
        }
    }

    void SceneHierarchyPanel::DrawComponents(Entity entity)
    {
        // ─── Tag Component ───────────────────────────────────────────────────
        if (entity.HasComponent<TagComponent>())
        {
            auto& tag = entity.GetComponent<TagComponent>().Tag;

            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, sizeof(buffer), tag.c_str(), sizeof(buffer) - 1);

            ImGui::Text("Tag:");
            ImGui::SameLine();
            if (ImGui::InputText("##Tag", buffer, sizeof(buffer)))
            {
                tag = std::string(buffer);
            }
        }

        ImGui::SameLine();
        ImGui::PushItemWidth(-1);

        // Add Component Button
        if (ImGui::Button("+ Add Component"))
            ImGui::OpenPopup("AddComponent");

        if (ImGui::BeginPopup("AddComponent"))
        {
            if (!entity.HasComponent<SpriteRendererComponent>())
            {
                if (ImGui::MenuItem("Sprite Renderer"))
                {
                    entity.AddComponent<SpriteRendererComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!entity.HasComponent<CameraComponent>())
            {
                if (ImGui::MenuItem("Camera"))
                {
                    entity.AddComponent<CameraComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!entity.HasComponent<Rigidbody2DComponent>())
            {
                if (ImGui::MenuItem("Rigidbody 2D"))
                {
                    entity.AddComponent<Rigidbody2DComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!entity.HasComponent<BoxCollider2DComponent>())
            {
                if (ImGui::MenuItem("Box Collider 2D"))
                {
                    entity.AddComponent<BoxCollider2DComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!entity.HasComponent<CircleCollider2DComponent>())
            {
                if (ImGui::MenuItem("Circle Collider 2D"))
                {
                    entity.AddComponent<CircleCollider2DComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!entity.HasComponent<AudioSourceComponent>())
            {
                if (ImGui::MenuItem("Audio Source"))
                {
                    entity.AddComponent<AudioSourceComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!entity.HasComponent<ParticleEmitterComponent>())
            {
                if (ImGui::MenuItem("Particle Emitter"))
                {
                    entity.AddComponent<ParticleEmitterComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            if (!entity.HasComponent<NativeScriptComponent>())
            {
                if (ImGui::MenuItem("C++ Native Script"))
                {
                    entity.AddComponent<NativeScriptComponent>();
                    ImGui::CloseCurrentPopup();
                }
            }

            ImGui::EndPopup();
        }
        ImGui::PopItemWidth();

        // ─── Transform Component ─────────────────────────────────────────────
        DrawComponent<TransformComponent>("Transform", entity, [](auto& component) {
            DrawVec3Control("Translation", component.Translation);

            glm::vec3 rotationDegrees = glm::degrees(component.Rotation);
            DrawVec3Control("Rotation", rotationDegrees);
            component.Rotation = glm::radians(rotationDegrees);

            DrawVec3Control("Scale", component.Scale, 1.0f);
        }, false);

        // ─── Sprite Renderer Component ───────────────────────────────────────
        DrawComponent<SpriteRendererComponent>("Sprite Renderer", entity, [](auto& component) {
            ImGui::ColorEdit4("Color", glm::value_ptr(component.Color));

            ImGui::Spacing();
            ImGui::Text("Texture:");
            ImGui::SameLine();

            // Texture slot display
            std::string texLabel = component.Texture ? "Custom Texture" : "(Drag & drop texture here)";
            ImGui::Button(texLabel.c_str(), ImVec2(180, 0));

            // Drag and Drop target for texture from Content Browser
            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const char* path = (const char*)payload->Data;
                    std::string ext = std::filesystem::path(path).extension().string();
                    if (ext == ".png" || ext == ".jpg" || ext == ".jpeg" || ext == ".bmp")
                    {
                        component.Texture = Texture2D::Create(path);
                    }
                }
                ImGui::EndDragDropTarget();
            }

            if (component.Texture)
            {
                ImGui::SameLine();
                if (ImGui::Button("Clear"))
                {
                    component.Texture = nullptr;
                }
            }

            ImGui::DragFloat("Tiling Factor", &component.TilingFactor, 0.1f, 0.0f, 100.0f);
        });

        // ─── Camera Component ────────────────────────────────────────────────
        DrawComponent<CameraComponent>("Camera", entity, [](auto& component) {
            ImGui::Checkbox("Primary Camera", &component.Primary);
            ImGui::Checkbox("Fixed Aspect Ratio", &component.FixedAspectRatio);

            ImGui::DragFloat("Orthographic Size", &component.OrthographicSize, 0.1f, 0.1f, 100.0f);
            ImGui::DragFloat("Near Clip", &component.OrthographicNear, 0.1f, -100.0f, 10.0f);
            ImGui::DragFloat("Far Clip", &component.OrthographicFar, 0.1f, -10.0f, 100.0f);
        });

        // ─── Rigidbody 2D Component ──────────────────────────────────────────
        DrawComponent<Rigidbody2DComponent>("Rigidbody 2D", entity, [](auto& component) {
            const char* bodyTypeStrings[] = { "Static", "Dynamic", "Kinematic" };
            const char* currentBodyTypeString = bodyTypeStrings[(int)component.Type];

            if (ImGui::BeginCombo("Body Type", currentBodyTypeString))
            {
                for (int i = 0; i < 3; i++)
                {
                    bool isSelected = currentBodyTypeString == bodyTypeStrings[i];
                    if (ImGui::Selectable(bodyTypeStrings[i], isSelected))
                    {
                        currentBodyTypeString = bodyTypeStrings[i];
                        component.Type = (Rigidbody2DComponent::BodyType)i;
                    }

                    if (isSelected)
                        ImGui::SetItemDefaultFocus();
                }
                ImGui::EndCombo();
            }

            ImGui::Checkbox("Fixed Rotation", &component.FixedRotation);
        });

        // ─── Box Collider 2D Component ───────────────────────────────────────
        DrawComponent<BoxCollider2DComponent>("Box Collider 2D", entity, [](auto& component) {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), 0.1f);
            ImGui::DragFloat2("Size", glm::value_ptr(component.Size), 0.1f);
            ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
        });

        // ─── Circle Collider 2D Component ────────────────────────────────────
        DrawComponent<CircleCollider2DComponent>("Circle Collider 2D", entity, [](auto& component) {
            ImGui::DragFloat2("Offset", glm::value_ptr(component.Offset), 0.1f);
            ImGui::DragFloat("Radius", &component.Radius, 0.1f);
            ImGui::DragFloat("Density", &component.Density, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Friction", &component.Friction, 0.01f, 0.0f, 1.0f);
            ImGui::DragFloat("Restitution", &component.Restitution, 0.01f, 0.0f, 1.0f);
        });

        // ─── Audio Source Component ──────────────────────────────────────────
        DrawComponent<AudioSourceComponent>("Audio Source", entity, [](auto& component) {
            char buffer[256];
            memset(buffer, 0, sizeof(buffer));
            strncpy_s(buffer, sizeof(buffer), component.AudioPath.c_str(), sizeof(buffer) - 1);

            if (ImGui::InputText("Audio Clip", buffer, sizeof(buffer)))
                component.AudioPath = std::string(buffer);

            if (ImGui::BeginDragDropTarget())
            {
                if (const ImGuiPayload* payload = ImGui::AcceptDragDropPayload("CONTENT_BROWSER_ITEM"))
                {
                    const char* path = (const char*)payload->Data;
                    std::string ext = std::filesystem::path(path).extension().string();
                    if (ext == ".wav" || ext == ".mp3" || ext == ".ogg" || ext == ".flac")
                        component.AudioPath = path;
                }
                ImGui::EndDragDropTarget();
            }

            ImGui::SliderFloat("Volume", &component.Volume, 0.0f, 1.0f);
            ImGui::Checkbox("Loop", &component.Loop);
            ImGui::Checkbox("Play On Awake", &component.PlayOnAwake);
            ImGui::Checkbox("Mute", &component.Mute);

            if (ImGui::Button("Preview Play"))
            {
                if (!component.AudioPath.empty())
                    AudioSystem::Play(component.AudioPath, component.Volume, component.Loop);
            }
            ImGui::SameLine();
            if (ImGui::Button("Stop All"))
            {
                AudioSystem::StopAll();
            }
        });

        // ─── Particle Emitter Component ──────────────────────────────────────
        DrawComponent<ParticleEmitterComponent>("Particle Emitter", entity, [](auto& component) {
            ImGui::Checkbox("Active", &component.Active);
            ImGui::DragFloat("Emission Rate", &component.EmissionRate, 1.0f, 0.0f, 500.0f);
            ImGui::DragFloat("Min Lifetime", &component.LifetimeMin, 0.1f, 0.01f, 10.0f);
            ImGui::DragFloat("Max Lifetime", &component.LifetimeMax, 0.1f, 0.01f, 10.0f);
            ImGui::DragFloat2("Min Velocity", glm::value_ptr(component.VelocityMin), 0.1f);
            ImGui::DragFloat2("Max Velocity", glm::value_ptr(component.VelocityMax), 0.1f);
            ImGui::DragFloat("Size Begin", &component.SizeBegin, 0.01f, 0.01f, 5.0f);
            ImGui::DragFloat("Size End", &component.SizeEnd, 0.01f, 0.0f, 5.0f);
            ImGui::ColorEdit4("Color Begin", glm::value_ptr(component.ColorBegin));
            ImGui::ColorEdit4("Color End", glm::value_ptr(component.ColorEnd));
        });

        // ─── Native Script Component ─────────────────────────────────────────
        DrawComponent<NativeScriptComponent>("C++ Script", entity, [](auto& component) {
            ImGui::TextColored(ImVec4(0.42f, 0.66f, 1.0f, 1.0f), "C++ ScriptableEntity");
            ImGui::TextWrapped("Instantiates OnCreate/OnUpdate/OnDestroy lifecycle hooks in Play mode.");
            if (component.Instance)
                ImGui::TextColored(ImVec4(0.25f, 0.88f, 0.45f, 1.0f), "Status: Running");
            else
                ImGui::TextDisabled("Status: Ready (Inactive in Edit mode)");
        });
    }
}

#pragma once

// Core Engine Systems
#include "Core/Base.h"
#include "Core/Assert.h"
#include "Core/Application.h"
#include "Core/Log.h"
#include "Core/LogSink.h"
#include "Core/Window.h"
#include "Core/Input.h"
#include "Core/KeyCodes.h"
#include "Core/MouseButtonCodes.h"
#include "Core/Timestep.h"
#include "Core/Layer.h"
#include "Core/LayerStack.h"

// Events System
#include "Events/Event.h"
#include "Events/ApplicationEvent.h"
#include "Events/KeyEvent.h"
#include "Events/MouseEvent.h"

// Scene & ECS
#include "Scene/Scene.h"
#include "Scene/Entity.h"
#include "Scene/ScriptableEntity.h"
#include "Scene/Components.h"
#include "Scene/SceneSerializer.h"
#include "Scene/ParticleSystem.h"

// Audio
#include "Audio/AudioSystem.h"

// Scripting
#include "Scripting/ScriptEngine.h"

// Utilities
#include "Utils/PlatformUtils.h"

// Renderer
#include "Renderer/Renderer.h"
#include "Renderer/Renderer2D.h"
#include "Renderer/Buffer.h"
#include "Renderer/VertexArray.h"
#include "Renderer/Shader.h"
#include "Renderer/Texture.h"
#include "Renderer/SubTexture2D.h"
#include "Renderer/OrthographicCamera.h"
#include "Renderer/OrthographicCameraController.h"
#include "Renderer/Framebuffer.h"

// Third-Party & Vendor
#include <glm/glm.hpp>
#include <glm/gtc/matrix_transform.hpp>
#include <glm/gtc/type_ptr.hpp>
#include <imgui.h>
#include <ImGuizmo.h>
#include <stb_image.h>

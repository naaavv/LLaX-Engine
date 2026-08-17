# LLaX-Engine

A modern, lightweight 2D/3D Game Engine built in C++20 with OpenGL, GLFW, Dear ImGui, GLM, stb_image, and spdlog.

## 🚀 Features

- **Modern C++20 Architecture**: Modular layer system, clean separation of core engine and client sandboxes.
- **Windowing & Input**: Full GLFW 3.4 integration supporting mouse, keyboard, and resize events.
- **Real-Time Input Polling**: Static `Input::IsKeyPressed()`, `Input::GetMousePosition()` polling anywhere in code.
- **OpenGL Graphics Pipeline**: Glad loader supporting Modern OpenGL with clear & viewport management.
- **Dear ImGui Integration**: Built-in docking and multi-viewport support for developer tooling and UI overlays.
- **Math Library**: GLM (OpenGL Mathematics) vector, matrix, and quaternion math.
- **Texture Support**: stb_image integrated for fast 2D texture and image file decoding.
- **Fast Logging**: spdlog colorized logging macros for both Core engine and Client apps.

---

## 📁 Project Structure

```
LLaX Engine/
├── CMakeLists.txt              # Root CMake configuration (FetchContent dependencies)
├── CMakePresets.json           # Visual Studio & Ninja build presets
├── README.md
├── .gitignore
├── Assets/                     # Shaders, textures, and assets
├── Engine/                     # LLaX Engine Core Library
│   ├── CMakeLists.txt
│   ├── vendor/
│   │   ├── stb_image/          # stb_image.h & stb_image.cpp
│   │   └── glad/               # GLAD khrplatform.h
│   └── src/
│       ├── LLaX.h             # Master Engine include header
│       ├── Core/
│       │   ├── Base.h         # Core definitions, macros & smart pointer aliases
│       │   ├── Log.h / .cpp   # Spdlog wrapper
│       │   ├── Timestep.h     # Delta time utility
│       │   ├── KeyCodes.h     # GLFW Key codes
│       │   ├── MouseButtonCodes.h
│       │   ├── Input.h / .cpp # Input polling
│       │   ├── Window.h / .cpp# GLFW Window & event dispatching
│       │   └── Application.h / .cpp # Main engine loop & layer lifecycle
│       ├── Events/
│       │   ├── Event.h        # Base event system & dispatcher
│       │   ├── ApplicationEvent.h
│       │   ├── KeyEvent.h
│       │   └── MouseEvent.h
│       ├── ImGui/
│       │   └── ImGuiLayer.h / .cpp # ImGui Docking & Viewport layer
│       └── Renderer/
│           └── Renderer.h / .cpp   # OpenGL rendering & state commands
└── Sandbox/                    # Client Application Demo
    ├── CMakeLists.txt
    └── src/
        └── SandboxApp.cpp      # Interactive demo with ImGui, WASD & Controls
```

---

## 🛠️ Integrated Third-Party Libraries

| Library | Version | Description |
| :--- | :--- | :--- |
| **GLFW** | 3.4 | Window creation & OS input handling |
| **GLAD** | 4.6 Core | Modern OpenGL function loader |
| **GLM** | 1.0.1 | Header-only mathematics library |
| **Dear ImGui** | Docking | Immediate mode graphical user interface |
| **stb_image** | Latest | Fast 2D texture & image file decoder |
| **spdlog** | 1.14.1 | Fast C++ logging library |

---

## 🏗️ Building and Running

### Prerequisites
- **CMake** (v3.20 or newer)
- **C++20 Compiler** (MSVC 2019/2022/2026, GCC 11+, or Clang 13+)

### Option 1: Visual Studio
1. Open the folder `LLaX Engine` in Visual Studio.
2. Select `Sandbox.exe` as the startup item and press **F5** to build and run.

### Option 2: Command Line (CMake)
```bash
# Configure dependencies
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build
cmake --build build --config Debug

# Run
./build/bin/Sandbox
```

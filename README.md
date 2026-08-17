# LLaX-Engine

A modern, lightweight 2D/3D Game Engine built in C++20 with OpenGL, GLFW, Dear ImGui, GLM, EnTT, Assimp, and spdlog.

## 🚀 Features

- **Modern C++20 Architecture**: Modular layer system, clean separation of core engine and client sandboxes.
- **Windowing & Input**: Full GLFW 3.4 integration supporting mouse, keyboard, and resize events.
- **OpenGL Graphics Pipeline**: Glad loader supporting Modern OpenGL with easy clear & viewport management.
- **Dear ImGui Integration**: Built-in docking and multi-viewport support for developer tooling and UI overlays.
- **Entity Component System (ECS)**: Powered by EnTT for high-performance entity management.
- **Math Library**: GLM (OpenGL Mathematics) vector, matrix, and quaternion math.
- **3D Asset Loading**: Assimp integrated for loading 3D models (OBJ, FBX, GLTF, etc.).
- **Fast Logging**: spdlog colorized logging macros for both Core engine and Client apps.

---

## 📁 Project Structure

```
LLaX Engine/
├── CMakeLists.txt              # Root CMake configuration (FetchContent dependencies)
├── CMakePresets.json           # Visual Studio & Ninja build presets
├── README.md
├── .gitignore
├── Assets/                     # Shaders, textures, and 3D models
├── Engine/                     # LLaX Engine Core Library
│   ├── CMakeLists.txt
│   └── src/
│       ├── LLaX.h             # Master Engine include header
│       ├── Core/
│       │   ├── Base.h         # Core definitions, macros & smart pointer aliases
│       │   ├── Log.h / .cpp   # Spdlog wrapper
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
        └── SandboxApp.cpp      # Interactive demo with EnTT, ImGui & Controls
```

---

## 🛠️ Integrated Third-Party Libraries

| Library | Version | Description |
| :--- | :--- | :--- |
| **GLFW** | 3.4 | Window creation & OS input handling |
| **GLAD** | 4.6 Core | Modern OpenGL function loader |
| **GLM** | 1.0.1 | Header-only mathematics library |
| **Dear ImGui** | Docking | Immediate mode graphical user interface |
| **EnTT** | 3.13.2 | Fast header-only Entity Component System (ECS) |
| **Assimp** | 5.4.3 | Open Asset Import Library for 3D model formats |
| **spdlog** | 1.14.1 | Fast C++ logging library |

---

## 🏗️ Building and Running

### Prerequisites
- **CMake** (v3.20 or newer)
- **C++20 Compiler** (MSVC 2019/2022, GCC 11+, or Clang 13+)
- **Git** (for FetchContent dependency downloads)

### Option 1: Visual Studio
1. Open the folder `LLaX Engine` in Visual Studio (Open Folder / CMake Project).
2. Visual Studio will automatically detect `CMakePresets.json` and configure dependencies.
3. Select `Sandbox.exe` as the startup item and press **F5** to build and run.

### Option 2: Command Line (CMake)
```bash
# Configure and download dependencies
cmake -B build -DCMAKE_BUILD_TYPE=Debug

# Build the Engine and Sandbox executable
cmake --build build --config Debug

# Run the Sandbox demo
./build/bin/Sandbox
```

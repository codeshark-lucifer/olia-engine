# olia-engine

**A lightweight, ECS-driven Vulkan 3D game engine in C++**

**olia-engine** is a modern, modular game engine built with Vulkan 1.3+, an Entity-Component-System (ECS) architecture, and a Unity-like animation and scene hierarchy workflow. It provides real-time rendering, skeletal animation, hierarchical transform graphs, texture management, and flexible resource management.

---

## ✨ Features

- **Vulkan 1.3+ Renderer**:
  - Swapchain recreation on window resize.
  - Depth buffering and dynamic viewport/scissor state.
  - Descriptor-driven shader resource manager supporting Uniform Buffers (UBO), Storage Buffers (SSBO), and Samplers.
  - Push Constants for matrix updates (`model`, `view`, `proj`).
- **Unity-like Skeletal Animation System**:
  - Powered by [`ufbx`](https://github.com/ufbx/ufbx) for fast, robust FBX loading.
  - Separate loading of 3D meshes/skeletons and animation clips (e.g., `character.fbx` + `character@walk_f.fbx`).
  - Automatic bone-node matching by name across separate mesh and animation files.
  - **`Animator` component**: Supports multiple `AnimationClip`s (`AddClip`, `Play`, `Pause`, `isLooping`, `speed`).
  - GPU-accelerated vertex skinning (up to 4 bone influences per vertex via SSBO).
- **Hierarchical Transform System**:
  - Full parent-child scene graph hierarchy via `parent` entity linkage.
  - Local space transforms (`position`, `rotation`, `scale`).
  - Global / World space transformations (`GetWorldPosition()`, `GetWorldRotation()`, `GetWorldScale()`, `GetWorldMatrix()`).
  - World-space direction vectors (`GetForward()`, `GetRight()`, `GetUp()`).
- **PBR Material & Texture System**:
  - `Material` component with color, roughness, metallic, and texture bindings.
  - `TextureManager` for automatic asset resolution, image loading (JPEG, PNG via `stb_image`), texture caching, and fallback default textures.
- **Lighting System**:
  - Real-time point light support passed to GPU shader storage buffers.
- **Entity-Component-System (ECS)**:
  - Cache-friendly, type-safe pool-based ECS.
- **Input & Time Handling**:
  - Frame-rate independent delta time and full keyboard/mouse query support via SDL3.

---

## 📦 Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) | 1.3+ | Graphics API |
| [GLM](https://github.com/g-truc/glm) | 0.9.9+ | Mathematics |
| [SDL3](https://github.com/libsdl-org/SDL) | 3.x | Windowing & Input |
| [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) | 3.4.0 | Vulkan Memory Management |
| [ufbx](https://github.com/ufbx/ufbx) | 0.23.x | FBX Mesh & Animation Import (included) |
| [stb_image](https://github.com/nothings/stb) | 2.x | Texture Image Loading (included) |

---

## 🛠️ Build Instructions

### Prerequisites

- **C++17** compiler (GCC 9+, Clang 10+, MSVC 2019+)
- **CMake** 3.12+
- **Vulkan SDK** (1.3+) installed with `VULKAN_SDK` environment variable set.

### Build Steps

```bash
git clone https://github.com/codeshark-lucifer/olia-engine.git
cd olia-engine
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

The compiled binary will be generated under `bin/` (or `bin/Release/` on Windows).

### Shader Compilation

Shaders are located in `assets/shaders/`. Compile GLSL shaders to SPIR-V before running the engine:

```bash
# From assets/shaders/
glslc main.vert -o compiled/main.vert.spv
glslc main.frag -o compiled/main.frag.spv
```

---

## 📁 Project Structure

```
olia-engine/
├── engine/                     # Core engine framework
│   ├── include/               
│   │   ├── components/         # Transform, Material, Light, MeshRenderer, Skeleton
│   │   ├── core/               # ECS, Animation, Input, Models
│   │   ├── vulkan/             # Device, Swapchain, Pipeline, TextureManager, Resources
│   │   └── engine/             # Engine entry point & globals
│   └── src/                    # Engine implementation
├── app/                        # Application / Game code
│   ├── include/app/            # Game headers
│   └── src/                    # setup(), loop(), main()
├── assets/                     # Models, textures, shaders
│   ├── models/                 # FBX files (e.g. character.fbx, character@walk_f.fbx)
│   ├── textures/               # Texture maps (.jpg, .png)
│   └── shaders/                # GLSL source & compiled SPIR-V
└── CMakeLists.txt
```

---

## 🧩 Built-in Components

| Component | Description |
|-----------|-------------|
| `Transform` | Local & global position, rotation (Euler radians), scale, and parent entity reference (`SetParent`). |
| `Camera` | Perspective or Orthographic projections. Automatically calculates View matrices from parented world transforms. |
| `MeshRenderer` | Handles vertex/index Vulkan buffer allocation and draw calls. |
| `Material` | Color (RGBA), roughness, metallic, texture path, and `Engine::Texture` reference. |
| `Light` | Point light color and intensity. |
| `Skeleton` | List of bones, parent bone indices, inverse bind matrices, and final GPU matrices. |
| `Animator` | Map of `AnimationClip`s, playback time, playback speed, looping flag, and `Play()`/`Pause()` functions. |

---

## 🚀 Quick Start Guide

### 1. Hierarchical Transforms (Parent-Child)

```cpp
// Create a parent vehicle entity
Entity car = ecs->Create();
auto &carTrans = ecs->Get<Transform>(car);
carTrans.position = glm::vec3(0.0f, 0.0f, 0.0f);

// Create a child entity (e.g., wheel) attached to the car
Entity wheel = ecs->Create();
auto &wheelTrans = ecs->Get<Transform>(wheel);
wheelTrans.SetParent(car);                         // Attach hierarchy
wheelTrans.position = glm::vec3(1.5f, 0.5f, 0.0f); // Local offset relative to car

// Translate the car -> wheel automatically follows in World Space!
carTrans.position.x += 10.0f;

// Get world coordinates
glm::vec3 wheelWorldPos = wheelTrans.GetWorldPosition(ecs); // (11.5, 0.5, 0.0)
```

### 2. Loading Character Models & Playing Animations

```cpp
#include <app/app.h>

Entity character;

void setup() {
    // 1. Load Character Mesh + Skeleton
    character = LoadModel("models/character.fbx")[0];
    
    // Rotate character if needed
    auto &trans = ecs->Get<Transform>(character);
    trans.rotation.x = glm::radians(-90.0f);

    // 2. Load separate animation file
    AnimationClip walkClip = Engine::LoadAnimation("models/character@walk_f.fbx");

    // 3. Add to Animator & Play
    if (ecs->Has<Animator>(character)) {
        auto &animator = ecs->Get<Animator>(character);
        animator.AddClip("walk", walkClip);
        animator.Play("walk");
        animator.isLooping = true;
    }
}

void loop() {
    float deltaTime = (float)input->GetTime().deltaTime;

    // Advance and evaluate skeletal skinning
    Engine::UpdateSkeletalAnimation(character, deltaTime);
}
```

### 3. Setting Up Camera and Lighting

```cpp
void setup() {
    // Main Camera
    Entity camera = ecs->Create();
    auto &cam = ecs->Add<Camera>(camera);
    cam.fov = 60.0f;
    
    auto &camTrans = ecs->Get<Transform>(camera);
    camTrans.position = glm::vec3(0.0f, 2.0f, 5.0f);
    camTrans.rotation = glm::vec3(glm::radians(-15.0f), 0.0f, 0.0f);

    // Point Light
    Entity light = ecs->Create();
    auto &l = ecs->Add<Light>(light);
    l.color = glm::vec3(1.0f, 0.9f, 0.8f);
    l.intensity = 2.0f;
    
    auto &ltTrans = ecs->Get<Transform>(light);
    ltTrans.position = glm::vec3(2.0f, 4.0f, 3.0f);
}
```

### 4. Engine Entry Point

```cpp
int main() {
    Engine::Run(setup, loop);
    return 0;
}
```

---

## 📐 Vertex & Shader Layout

### Vertex Attribute Layout (`Engine::Vertex`)

| Location | Attribute | GLSL Type | Format |
|----------|-----------|-----------|--------|
| `0` | `position` | `vec3` | `VK_FORMAT_R32G32B32_SFLOAT` |
| `1` | `normal` | `vec3` | `VK_FORMAT_R32G32B32_SFLOAT` |
| `2` | `uv` | `vec2` | `VK_FORMAT_R32G32_SFLOAT` |
| `3` | `boneIndices` | `ivec4` | `VK_FORMAT_R32G32B32A32_SINT` |
| `4` | `boneWeights` | `vec4` | `VK_FORMAT_R32G32B32A32_SFLOAT` |

### Descriptor Set Layout (Set 0)

| Binding | Type | Shader Stage | Resource |
|---------|------|--------------|----------|
| `0` | Storage Buffer | Fragment | `LightBuffer` |
| `1` | Uniform Buffer | Fragment | `CameraUBO` |
| `2` | Uniform Buffer | Fragment | `MaterialUBO` |
| `3` | Combined Image Sampler | Fragment | `AlbedoMap` |
| `4` | Storage Buffer | Vertex | `BoneBuffer` (Skinning Matrices) |

---

## 📄 License

This project is licensed under the **MIT License** – see the [LICENSE](LICENSE) file for details.

Third-party libraries used in this project:
- **ufbx**: MIT License
- **Vulkan Memory Allocator**: MIT License
- **GLM**: MIT License
- **stb_image**: MIT / Public Domain
- **SDL3**: zlib License

---

**Happy Coding!** 🎮
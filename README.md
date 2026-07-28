# olia-engine

**A lightweight ECS-driven Vulkan game engine in C++**

olia-engine is a modern, modular game engine built on Vulkan and an entity‑component‑system architecture. It provides a clean API for creating 3D applications with real‑time rendering, input handling, and a flexible component system.

---

## ✨ Features

- **Vulkan 1.0+ renderer** with swapchain, depth buffering, and command buffer recording.
- **Entity‑Component‑System (ECS)** – simple, fast, and type‑safe.
- **Built‑in components**:
  - `Transform` – position, rotation, scale.
  - `Camera` – perspective/orthographic projection with Vulkan depth corrections.
  - `MeshRenderer` – vertex/index buffer management and drawing.
  - `GameObject` – lightweight handle to group components.
- **Input handling** – keyboard, mouse (SDL3 based).
- **Customisable render loop** – user‑defined `setup()` and `loop()` functions.
- **Push constant support** for per‑object uniform data (model, view, projection matrices).

---

## 📦 Dependencies

| Library | Version | Purpose |
|---------|---------|---------|
| [Vulkan SDK](https://www.lunarg.com/vulkan-sdk/) | 1.3+ | Graphics API |
| [GLM](https://github.com/g-truc/glm) | 0.9.9+ | Mathematics |
| [SDL3](https://github.com/libsdl-org/SDL) | 3.x | Window & input |
| [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator) | 3.4.0 | Memory management |

All dependencies are included in the repository or linked externally.

---

## 🛠️ Build Instructions

### Prerequisites

- **C++17** compiler (GCC 9+, Clang 10+, MSVC 2019+)
- **CMake** 3.12+
- **Vulkan SDK** installed and `VULKAN_SDK` environment variable set.

### Build steps

```bash
git clone https://github.com/codeshark-lucifer/olia-engine.git
cd olia-engine
mkdir build && cd build
cmake ..
cmake --build . --config Release
```

The executable will be placed in `bin/` (or `bin/Release/` on Windows).

### Shader compilation

GLSL shaders are located in `assets/shaders/`. They must be compiled to SPIR‑V before running:

```bash
glslc main.vert -o compiled/main.vert.spv
glslc main.frag -o compiled/main.frag.spv
```

The engine expects `assets/shaders/compiled/` to contain the `.spv` files.

---

## 🧩 Project Structure

```
olia-engine/
├── engine/                     # Core engine
│   ├── include/                # Public headers
│   │   ├── core/               # ECS, input
│   │   ├── components/         # Built‑in components
│   │   ├── vulkan/             # Vulkan wrappers
│   │   └── engine/             # Engine entry points
│   └── src/                    # Implementation
├── app/                        # Example application
│   ├── include/app/            # App‑specific headers
│   └── src/                    # main(), setup(), loop()
├── assets/                     # Shaders, models, textures
└── CMakeLists.txt
```

---

## 🚀 Usage (Quick Start)

### 1. Create an entity with a mesh

```cpp
std::vector<Vertex> vertices = { /* ... */ };
std::vector<uint16_t> indices = { /* ... */ };
Entity meshEntity = CreateMesh(vertices, indices);
```

### 2. Add a camera

```cpp
Entity camera = ecs->Create();
auto& cam = ecs->Add<Camera>(camera);
auto& trans = ecs->Get<Transform>(camera);
trans.position = glm::vec3(0.0f, 0.0f, 10.0f);
```

### 3. Define `setup()` and `loop()`

```cpp
void setup() {
    // create entities, load resources
}

void loop() {
    // update components (move camera, rotate objects)
    auto meshes = ecs->Query<MeshRenderer>();
    for (auto e : meshes) {
        auto& t = ecs->Get<Transform>(e);
        t.rotation.y += 0.5f * input->GetTime().deltaTime;
    }
}
```

### 4. Start the engine

```cpp
int main() {
    Engine::Run(setup, loop);
    return 0;
}
```

The engine will handle window creation, Vulkan initialisation, and frame rendering.

---

## 📐 Vulkan ↔ GLSL Type Mapping

When defining vertex attributes or push constants, use the following table for correct data layout:

| Vulkan Format | GLSL Type | C++ Type (GLM) | Alignment |
|---------------|-----------|----------------|-----------|
| `VK_FORMAT_R32_SFLOAT` | `float` | `float` | 4 |
| `VK_FORMAT_R32G32_SFLOAT` | `vec2` | `glm::vec2` | 8 |
| `VK_FORMAT_R32G32B32_SFLOAT` | `vec3` | `glm::vec3` | 16 |
| `VK_FORMAT_R32G32B32A32_SFLOAT` | `vec4` | `glm::vec4` | 16 |
| `VK_FORMAT_R32_SINT` | `int` | `int32_t` | 4 |
| `VK_FORMAT_R32G32_SINT` | `ivec2` | `glm::ivec2` | 8 |
| `VK_FORMAT_R32G32B32_SINT` | `ivec3` | `glm::ivec3` | 16 |
| `VK_FORMAT_R32G32B32A32_SINT` | `ivec4` | `glm::ivec4` | 16 |
| `VK_FORMAT_R8G8B8A8_UNORM` | `vec4` | `glm::vec4` | 4 |

### GLM type alignment (for push constants / uniform blocks)

| Type | Size | Alignment |
|------|------|-----------|
| `glm::vec2` | 8 | 8 |
| `glm::vec3` | 12 | 16 |
| `glm::vec4` | 16 | 16 |
| `glm::mat2` | 16 | 8 |
| `glm::mat3` | 36 | 16 |
| `glm::mat4` | 64 | 16 |

Use `alignas()` in C++ structs to match GLSL layout.

---

## 📄 License

This project is licensed under the MIT License – see the [LICENSE](LICENSE) file for details.

Third‑party libraries are used under their respective licenses:

- Vulkan Memory Allocator – MIT
- GLM – MIT
- SDL3 – zlib/libpng

---

## 🤝 Contributing

Contributions are welcome! Please open an issue or pull request on GitHub.

---

## 📚 Acknowledgements

- [Vulkan Memory Allocator](https://github.com/GPUOpen-LibrariesAndSDKs/VulkanMemoryAllocator)
- [GLM](https://github.com/g-truc/glm)
- [SDL](https://www.libsdl.org/)

---

**Happy coding!** 🎮
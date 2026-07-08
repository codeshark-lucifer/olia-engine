# Olia Engine Custom UI Canvas System

Welcome to the **Olia Engine Custom UI Canvas System**, a modern, lightweight, component-driven UI library built on top of a custom Entity Component System (ECS) in C++ (powered by OpenGL/GLFW).

---

## 🎨 UI Architecture

Olia Engine adopts a strictly modular, ECS-based layout. UI components are lightweight state-containers attached to Entities, managed by `UISystem` for input processing (`UISystem::Update`) and batch-rendering (`UISystem::Render`).

All UI entities typically consist of:
1. **`Transform`**: Defines the $X$, $Y$ positions and drawing depth ($Z$).
2. **`SpriteRenderer`**: Defines background size, color tint, and texture references.
3. **UI Component (ECS)**: Specifies layout state and custom behavior callbacks.

---

## 🚀 Key Features

* **High Performance Batching**: Utilizes a central `Batch` renderer (`s_UIBatch`) to draw elements with minimal draw calls.
* **Scissor-Clipped Scroll Views**: Scroll view items are rendered with hardware-level scissor testing (`glScissor`) to ensure pixel-perfect boundary clipping during scroll actions.
* **Pixel-Perfect Scrollbar/Slider Dragging**: Fully interactive dragging for scroll bars and sliders using direct mouse tracking.
* **Generic Interaction Events**: A modular event callback wrapper (`UIEventComponent`) that turns any entity (e.g., images, panels, panels) into an interactive draggable/hoverable element.

---

## 📦 UI Component Catalog

Here is the list of available ECS UI components (defined in [engine/include/olia/ui/ui.h](file:///D:/Program%20Education/c++/olia-engine/engine/include/olia/ui/ui.h)):

### 1. `UIImageComponent`
Draws static or animated textures on the UI canvas.
```cpp
struct UIImageComponent {
    glm::vec2 position{0.0f};
    glm::vec2 size{100.0f};
    glm::vec4 color{1.0f};
    Texture* texture = nullptr;
};
```

### 2. `UIButtonComponent`
Triggers actions upon mouse hover and click events.
```cpp
struct UIButtonComponent {
    glm::vec2 position{0.0f};
    glm::vec2 size{120.0f, 40.0f};
    std::string label;
    glm::vec4 color;
    glm::vec4 hoverColor;
    glm::vec4 clickColor;
    std::function<void()> onClick = nullptr;
};
```

### 3. `UIInputFieldComponent`
Enables raw text input with keyboard support, UTF-8 backspacing, text selection cursors, and placeholder fallbacks.
```cpp
struct UIInputFieldComponent {
    glm::vec2 position{0.0f};
    glm::vec2 size{200.0f, 40.0f};
    std::string text;
    std::string placeholder;
    std::function<void(const std::string&)> onTextChanged = nullptr;
};
```

### 4. `UIDropdownComponent`
Creates selectable drop-down menus with option hover tinting.
```cpp
struct UIDropdownComponent {
    glm::vec2 position{0.0f};
    glm::vec2 size{200.0f, 40.0f};
    std::vector<std::string> options;
    int selectedIndex = 0;
    std::function<void(int)> onSelectionChanged = nullptr;
};
```

### 5. `UIScrollViewComponent`
Scrollable panel containing child entities. Supports arrow keys, mouse wheel scrolling, and mouse-dragging the scrollbar thumb.
```cpp
struct UIScrollViewComponent {
    glm::vec2 position{0.0f};
    glm::vec2 size{300.0f, 200.0f};
    glm::vec2 scrollOffset{0.0f};
    glm::vec2 maxScroll{0.0f, 200.0f};
    std::vector<Entity> children;
};
```

### 6. `UIGridLayoutComponent`
Automatically positions child elements in a grid structure.
```cpp
struct UIGridLayoutComponent {
    glm::vec2 position{0.0f};
    glm::vec2 cellSize{80.0f, 80.0f};
    glm::vec2 spacing{10.0f, 10.0f};
    int columns = 4;
    std::vector<Entity> children;
};
```

### 7. `UISliderComponent` *(New)*
An interactive slider bar that can adjust numerical values in real-time.
```cpp
struct UISliderComponent {
    glm::vec2 position{0.0f};
    glm::vec2 size{200.0f, 20.0f};
    float value = 0.5f;
    float minValue = 0.0f;
    float maxValue = 1.0f;
    glm::vec4 trackColor;
    glm::vec4 thumbColor;
    glm::vec4 thumbHoverColor;
    std::function<void(float)> onValueChanged = nullptr;
};
```

---

## ⚡ UI Event & Interaction System (`UIEventComponent`)

To make it incredibly easy for developers to add hover effects, click event hooks, drag-and-drop systems, or custom sliders, we implemented the `UIEventComponent`. 

By adding this component to any entity, you instantly get the following callbacks:
* `onHoverEnter`: Triggers when mouse cursor enters bounds.
* `onHover`: Triggers every frame the mouse is within bounds.
* `onHoverExit`: Triggers when mouse cursor leaves bounds.
* `onPress`: Triggers when mouse button is clicked down over bounds.
* `onDrag`: Triggers every frame the mouse drags the element, delivering the current cursor position and the frame's `delta` motion vector.
* `onRelease`: Triggers when the mouse button is released.

### 💡 Example: Making a Drag-and-Drop Image
```cpp
// Create image entity with one line using factory helper
Entity imgEntity = Olia::CreateImage(&wall, {430.0f, 150.0f}, {160.0f, 160.0f});

// Attach generic event support for hover tinting and drag movement
Olia::AddEventSupport(imgEntity,
    [](Entity entity) { // Hover Enter
        Olia::context.ecs->Get<Olia::SpriteRenderer>(entity).color = glm::vec4(1.0f, 0.9f, 0.8f, 1.0f);
    },
    [](Entity entity) { // Hover Exit
        Olia::context.ecs->Get<Olia::SpriteRenderer>(entity).color = glm::vec4(1.0f);
    },
    [](Entity entity, glm::vec2 mousePos, glm::vec2 delta) { // Dragging
        auto& trans = Olia::context.ecs->Get<Olia::Transform>(entity);
        trans.position.x += delta.x;
        trans.position.y += delta.y;
        
        auto& img = Olia::context.ecs->Get<Olia::UIImageComponent>(entity);
        img.position.x += delta.x;
        img.position.y += delta.y;
    }
);
```

---

## 🛠️ Build and Run Instructions

This project uses **CMake** and **Ninja** as the build tools. 

### Prerequisites
Make sure you have CMake, Ninja, and a C++17 compiler (like MSVC, GCC, or Clang) installed on your system.

### Build Steps
1. **Configure CMake**:
   ```bash
   cmake -S . -B build -G Ninja
   ```
2. **Compile the engine and app**:
   ```bash
   cmake --build build
   ```
3. **Run the demo application**:
   ```bash
   ./build/app/application.exe
   ```

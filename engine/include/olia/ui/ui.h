#pragma once

#include <string>
#include <vector>
#include <functional>
#include <glm/glm.hpp>
#include <core/ecs.h>
#include <olia/olia.h>

namespace Olia
{
    // UI Element Components
    struct UIImageComponent
    {
        glm::vec2 position{0.0f};
        glm::vec2 size{100.0f};
        glm::vec4 color{1.0f};
        Texture* texture = nullptr;
    };

    struct UIButtonComponent
    {
        glm::vec2 position{0.0f};
        glm::vec2 size{120.0f, 40.0f};
        std::string label;
        glm::vec4 color{0.25f, 0.25f, 0.25f, 1.0f};
        glm::vec4 hoverColor{0.35f, 0.35f, 0.35f, 1.0f};
        glm::vec4 clickColor{0.15f, 0.15f, 0.15f, 1.0f};
        glm::vec4 textColor{1.0f};
        std::function<void()> onClick = nullptr;
        
        bool isHovered = false;
        bool isPressed = false;
    };

    struct UIInputFieldComponent
    {
        glm::vec2 position{0.0f};
        glm::vec2 size{200.0f, 40.0f};
        std::string text;
        std::string placeholder;
        glm::vec4 color{0.15f, 0.15f, 0.15f, 1.0f};
        glm::vec4 focusedColor{0.20f, 0.20f, 0.20f, 1.0f};
        glm::vec4 textColor{1.0f};
        glm::vec4 placeholderColor{0.5f, 0.5f, 0.5f, 1.0f};
        
        bool isFocused = false;
        std::function<void(const std::string&)> onTextChanged = nullptr;
    };

    struct UIDropdownComponent
    {
        glm::vec2 position{0.0f};
        glm::vec2 size{200.0f, 40.0f};
        std::vector<std::string> options;
        int selectedIndex = 0;
        bool isOpen = false;
        
        glm::vec4 color{0.2f, 0.2f, 0.2f, 1.0f};
        glm::vec4 hoverColor{0.3f, 0.3f, 0.3f, 1.0f};
        glm::vec4 textColor{1.0f};
        
        std::function<void(int)> onSelectionChanged = nullptr;
        
        int hoveredIndex = -1; // -1 if none, 0+ for option list
    };

    struct UIScrollViewComponent
    {
        glm::vec2 position{0.0f};
        glm::vec2 size{300.0f, 200.0f};
        glm::vec2 scrollOffset{0.0f};
        glm::vec2 maxScroll{0.0f, 200.0f};
        
        glm::vec4 backgroundColor{0.1f, 0.1f, 0.1f, 0.5f};
        std::vector<Entity> children;

        bool isDraggingScrollbar = false;
    };

    struct UISliderComponent
    {
        glm::vec2 position{0.0f};
        glm::vec2 size{200.0f, 20.0f};
        float value = 0.5f;
        float minValue = 0.0f;
        float maxValue = 1.0f;
        
        glm::vec4 trackColor{0.15f, 0.15f, 0.15f, 1.0f};
        glm::vec4 thumbColor{0.3f, 0.6f, 0.9f, 1.0f};
        glm::vec4 thumbHoverColor{0.4f, 0.7f, 1.0f, 1.0f};
        
        bool isDragging = false;
        std::function<void(float)> onValueChanged = nullptr;
    };

    struct UIEventComponent
    {
        glm::vec2 customBoundsPosition{0.0f};
        glm::vec2 customBoundsSize{0.0f};

        std::function<void(Entity)> onHoverEnter = nullptr;
        std::function<void(Entity)> onHover = nullptr;
        std::function<void(Entity)> onHoverExit = nullptr;
        std::function<void(Entity, glm::vec2)> onPress = nullptr;
        std::function<void(Entity, glm::vec2, glm::vec2)> onDrag = nullptr;
        std::function<void(Entity, glm::vec2)> onRelease = nullptr;

        bool isHovered = false;
        bool isPressed = false;
        glm::vec2 lastMousePos{0.0f};
    };

    struct UIGridLayoutComponent
    {
        glm::vec2 position{0.0f};
        glm::vec2 cellSize{80.0f, 80.0f};
        glm::vec2 spacing{10.0f, 10.0f};
        int columns = 4;
        
        std::vector<Entity> children;
    };

    class UISystem
    {
    public:
        static void Update();
        static void Render();
        
        // Character input callback helper for InputField
        static void CharCallback(unsigned int codepoint);
    };

    // ==========================================
    // UI Factory Helpers (Olia namespace)
    // ==========================================
    Entity CreateImage(Texture* texture, glm::vec2 position, glm::vec2 size, glm::vec4 color = glm::vec4(1.0f));
    
    Entity CreateButton(const std::string& label, glm::vec2 position, glm::vec2 size, std::function<void()> onClick,
                        glm::vec4 color = glm::vec4(0.25f, 0.25f, 0.25f, 1.0f),
                        glm::vec4 hoverColor = glm::vec4(0.35f, 0.35f, 0.35f, 1.0f),
                        glm::vec4 clickColor = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f),
                        glm::vec4 textColor = glm::vec4(1.0f));
                           
    Entity CreateInputField(const std::string& placeholder, glm::vec2 position, glm::vec2 size,
                                   std::function<void(const std::string&)> onTextChanged = nullptr);
                                   
    Entity CreateDropdown(const std::vector<std::string>& options, glm::vec2 position, glm::vec2 size,
                                 std::function<void(int)> onSelectionChanged = nullptr);
                                 
    Entity CreateSlider(glm::vec2 position, glm::vec2 size, float minValue, float maxValue, float initialValue,
                               std::function<void(float)> onValueChanged = nullptr);
                               
    Entity CreateScrollView(glm::vec2 position, glm::vec2 size, glm::vec2 maxScroll);
    
    Entity CreateGridLayout(glm::vec2 position, glm::vec2 cellSize, glm::vec2 spacing, int columns);

    Entity CreateText(const std::string& text, glm::vec2 position, float scale = 1.0f, glm::vec4 color = glm::vec4(1.0f));

    void AddEventSupport(Entity entity, 
                         std::function<void(Entity)> onHoverEnter = nullptr,
                         std::function<void(Entity)> onHoverExit = nullptr,
                         std::function<void(Entity, glm::vec2, glm::vec2)> onDrag = nullptr);
}

#include <olia/ui/ui.h>
#include <iostream>
#include <unordered_set>
#include <glad/gl.h>
#include <GLFW/glfw3.h>

namespace Olia
{
    static Batch s_UIBatch;

    void UISystem::Update()
    {
        if (!context.ecs || !context.input) return;

        double mouseX, mouseY;
        context.input->GetMousePosition(mouseX, mouseY);
        bool mousePressed = context.input->GetMouseButton(GLFW_MOUSE_BUTTON_LEFT);
        bool mouseClicked = context.input->GetMouseButtonDown(GLFW_MOUSE_BUTTON_LEFT);

        // Map child entity -> parent scroll view entity
        std::unordered_map<Entity, Entity> scrollChildToParent;
        auto scrollViews = context.ecs->Query<UIScrollViewComponent>();

        // 1. Update Scroll Views first to reposition children!
        for (Entity entity : scrollViews)
        {
            auto& sv = context.ecs->Get<UIScrollViewComponent>(entity);
            for (Entity child : sv.children)
            {
                scrollChildToParent[child] = entity;
            }

            bool hover = (mouseX >= sv.position.x && mouseX <= sv.position.x + sv.size.x &&
                          mouseY >= sv.position.y && mouseY <= sv.position.y + sv.size.y);

            float thumbHeight = glm::max(20.0f, (sv.size.y / (sv.size.y + sv.maxScroll.y)) * sv.size.y);
            float scrollbarX = sv.position.x + sv.size.x - 10.0f;

            if (mouseClicked)
            {
                // If clicked on scrollbar track or thumb
                if (mouseX >= scrollbarX - 2.0f && mouseX <= sv.position.x + sv.size.x &&
                    mouseY >= sv.position.y && mouseY <= sv.position.y + sv.size.y)
                {
                    sv.isDraggingScrollbar = true;
                }
            }

            if (sv.isDraggingScrollbar)
            {
                if (mousePressed)
                {
                    float clickYFactor = (mouseY - sv.position.y - thumbHeight * 0.5f) / (sv.size.y - thumbHeight);
                    sv.scrollOffset.y = glm::clamp(clickYFactor * sv.maxScroll.y, 0.0f, sv.maxScroll.y);
                }
                else
                {
                    sv.isDraggingScrollbar = false;
                }
            }

            if (hover && !sv.isDraggingScrollbar)
            {
                if (context.input->GetKey(GLFW_KEY_UP))
                {
                    sv.scrollOffset.y = glm::max(0.0f, sv.scrollOffset.y - 4.0f);
                }
                if (context.input->GetKey(GLFW_KEY_DOWN))
                {
                    sv.scrollOffset.y = glm::min(sv.maxScroll.y, sv.scrollOffset.y + 4.0f);
                }
                
                // Mouse wheel scrolling
                if (g_ScrollYDelta != 0.0)
                {
                    sv.scrollOffset.y = glm::clamp(sv.scrollOffset.y - static_cast<float>(g_ScrollYDelta) * 20.0f, 0.0f, sv.maxScroll.y);
                }
            }

            // Reposition scroll view children
            for (int i = 0; i < (int)sv.children.size(); ++i)
            {
                Entity child = sv.children[i];
                float offsetIndexY = i * 50.0f; // vertical layout inside scroll view
                glm::vec2 targetPos = sv.position + glm::vec2(10.0f, 10.0f + offsetIndexY - sv.scrollOffset.y);

                // Update UI Component position (for mouse clicking)
                if (context.ecs->Has<UIButtonComponent>(child))
                    context.ecs->Get<UIButtonComponent>(child).position = targetPos;
                else if (context.ecs->Has<UIImageComponent>(child))
                    context.ecs->Get<UIImageComponent>(child).position = targetPos;
                else if (context.ecs->Has<UIInputFieldComponent>(child))
                    context.ecs->Get<UIInputFieldComponent>(child).position = targetPos;
                else if (context.ecs->Has<UISliderComponent>(child))
                    context.ecs->Get<UISliderComponent>(child).position = targetPos;

                // Update Transform position (so the rendered quad actually moves!)
                if (context.ecs->Has<Transform>(child))
                {
                    auto& trans = context.ecs->Get<Transform>(child);
                    trans.position = glm::vec3(targetPos.x, targetPos.y, trans.position.z);
                }
            }
        }

        // 2. Update Buttons
        auto buttons = context.ecs->Query<UIButtonComponent>();
        for (Entity entity : buttons)
        {
            auto& btn = context.ecs->Get<UIButtonComponent>(entity);
            auto& sprite = context.ecs->Get<SpriteRenderer>(entity);

            bool hover = (mouseX >= btn.position.x && mouseX <= btn.position.x + btn.size.x &&
                          mouseY >= btn.position.y && mouseY <= btn.position.y + btn.size.y);

            // Scissor clipping check
            if (scrollChildToParent.find(entity) != scrollChildToParent.end())
            {
                Entity parentSVEntity = scrollChildToParent[entity];
                auto& sv = context.ecs->Get<UIScrollViewComponent>(parentSVEntity);
                bool mouseInSV = (mouseX >= sv.position.x && mouseX <= sv.position.x + sv.size.x &&
                                  mouseY >= sv.position.y && mouseY <= sv.position.y + sv.size.y);
                if (!mouseInSV) hover = false;
            }

            btn.isHovered = hover;

            if (hover)
            {
                if (mouseClicked)
                {
                    btn.isPressed = true;
                }
                else if (btn.isPressed && !mousePressed)
                {
                    btn.isPressed = false;
                    if (btn.onClick) btn.onClick();
                }
                
                sprite.color = btn.isPressed ? btn.clickColor : btn.hoverColor;
            }
            else
            {
                btn.isPressed = false;
                sprite.color = btn.color;
            }
        }

        // 3. Update Input Fields
        auto inputs = context.ecs->Query<UIInputFieldComponent>();
        for (Entity entity : inputs)
        {
            auto& field = context.ecs->Get<UIInputFieldComponent>(entity);
            auto& sprite = context.ecs->Get<SpriteRenderer>(entity);

            bool hover = (mouseX >= field.position.x && mouseX <= field.position.x + field.size.x &&
                          mouseY >= field.position.y && mouseY <= field.position.y + field.size.y);

            // Scissor clipping check
            if (scrollChildToParent.find(entity) != scrollChildToParent.end())
            {
                Entity parentSVEntity = scrollChildToParent[entity];
                auto& sv = context.ecs->Get<UIScrollViewComponent>(parentSVEntity);
                bool mouseInSV = (mouseX >= sv.position.x && mouseX <= sv.position.x + sv.size.x &&
                                  mouseY >= sv.position.y && mouseY <= sv.position.y + sv.size.y);
                if (!mouseInSV) hover = false;
            }

            if (mouseClicked)
            {
                field.isFocused = hover;
            }

            sprite.color = field.isFocused ? field.focusedColor : field.color;

            // Handle backspace when focused
            if (field.isFocused && context.input->GetKeyDown(GLFW_KEY_BACKSPACE))
            {
                if (!field.text.empty())
                {
                    // Delete last character (handle UTF-8 backspacing properly)
                    unsigned char back = field.text.back();
                    if ((back & 0x80) == 0x00) // ASCII
                    {
                        field.text.pop_back();
                    }
                    else // UTF-8 multibyte
                    {
                        while (!field.text.empty() && (static_cast<unsigned char>(field.text.back()) & 0xC0) == 0x80)
                        {
                            field.text.pop_back();
                        }
                        if (!field.text.empty())
                        {
                            field.text.pop_back();
                        }
                    }
                    if (field.onTextChanged) field.onTextChanged(field.text);
                }
            }
        }

        // 4. Update Dropdowns
        auto dropdowns = context.ecs->Query<UIDropdownComponent>();
        for (Entity entity : dropdowns)
        {
            auto& drop = context.ecs->Get<UIDropdownComponent>(entity);
            auto& sprite = context.ecs->Get<SpriteRenderer>(entity);

            bool mainHover = (mouseX >= drop.position.x && mouseX <= drop.position.x + drop.size.x &&
                              mouseY >= drop.position.y && mouseY <= drop.position.y + drop.size.y);

            // Scissor clipping check
            if (scrollChildToParent.find(entity) != scrollChildToParent.end())
            {
                Entity parentSVEntity = scrollChildToParent[entity];
                auto& sv = context.ecs->Get<UIScrollViewComponent>(parentSVEntity);
                bool mouseInSV = (mouseX >= sv.position.x && mouseX <= sv.position.x + sv.size.x &&
                                  mouseY >= sv.position.y && mouseY <= sv.position.y + sv.size.y);
                if (!mouseInSV) mainHover = false;
            }

            sprite.color = mainHover ? drop.hoverColor : drop.color;

            if (mainHover && mouseClicked)
            {
                drop.isOpen = !drop.isOpen;
            }
            else if (drop.isOpen)
            {
                drop.hoveredIndex = -1;
                for (int i = 0; i < (int)drop.options.size(); ++i)
                {
                    float optY = drop.position.y + drop.size.y + i * drop.size.y;
                    bool optHover = (mouseX >= drop.position.x && mouseX <= drop.position.x + drop.size.x &&
                                     mouseY >= optY && mouseY <= optY + drop.size.y);
                    if (optHover)
                    {
                        drop.hoveredIndex = i;
                        if (mouseClicked)
                        {
                            drop.selectedIndex = i;
                            drop.isOpen = false;
                            if (drop.onSelectionChanged) drop.onSelectionChanged(i);
                            break;
                        }
                    }
                }

                if (mouseClicked && !mainHover && drop.hoveredIndex == -1)
                {
                    drop.isOpen = false;
                }
            }
        }

        // 5. Update Sliders
        auto sliders = context.ecs->Query<UISliderComponent>();
        for (Entity entity : sliders)
        {
            auto& slider = context.ecs->Get<UISliderComponent>(entity);
            
            // Sync background sprite renderer if exists
            if (context.ecs->Has<SpriteRenderer>(entity))
            {
                auto& sprite = context.ecs->Get<SpriteRenderer>(entity);
                sprite.color = slider.trackColor;
                sprite.size = slider.size;
            }

            bool hover = (mouseX >= slider.position.x && mouseX <= slider.position.x + slider.size.x &&
                          mouseY >= slider.position.y && mouseY <= slider.position.y + slider.size.y);

            // Scissor clipping check
            if (scrollChildToParent.find(entity) != scrollChildToParent.end())
            {
                Entity parentSVEntity = scrollChildToParent[entity];
                auto& sv = context.ecs->Get<UIScrollViewComponent>(parentSVEntity);
                bool mouseInSV = (mouseX >= sv.position.x && mouseX <= sv.position.x + sv.size.x &&
                                  mouseY >= sv.position.y && mouseY <= sv.position.y + sv.size.y);
                if (!mouseInSV) hover = false;
            }

            if (mouseClicked && hover)
            {
                slider.isDragging = true;
            }

            if (slider.isDragging)
            {
                if (mousePressed)
                {
                    float clickXFactor = (mouseX - slider.position.x) / slider.size.x;
                    clickXFactor = glm::clamp(clickXFactor, 0.0f, 1.0f);
                    slider.value = slider.minValue + clickXFactor * (slider.maxValue - slider.minValue);
                    if (slider.onValueChanged)
                    {
                        slider.onValueChanged(slider.value);
                    }
                }
                else
                {
                    slider.isDragging = false;
                }
            }
        }

        // 6. Update Grid Layouts
        auto grids = context.ecs->Query<UIGridLayoutComponent>();
        for (Entity entity : grids)
        {
            auto& grid = context.ecs->Get<UIGridLayoutComponent>(entity);
            for (int i = 0; i < (int)grid.children.size(); ++i)
            {
                Entity child = grid.children[i];
                int r = i / grid.columns;
                int c = i % grid.columns;

                glm::vec2 targetPos = grid.position + glm::vec2(
                    c * (grid.cellSize.x + grid.spacing.x),
                    r * (grid.cellSize.y + grid.spacing.y)
                );

                // Update UI Component position (for mouse clicking)
                if (context.ecs->Has<UIButtonComponent>(child))
                    context.ecs->Get<UIButtonComponent>(child).position = targetPos;
                else if (context.ecs->Has<UIImageComponent>(child))
                    context.ecs->Get<UIImageComponent>(child).position = targetPos;
                else if (context.ecs->Has<UIInputFieldComponent>(child))
                    context.ecs->Get<UIInputFieldComponent>(child).position = targetPos;
                else if (context.ecs->Has<UISliderComponent>(child))
                    context.ecs->Get<UISliderComponent>(child).position = targetPos;

                // Update Transform position (so the rendered quad actually moves!)
                if (context.ecs->Has<Transform>(child))
                {
                    auto& trans = context.ecs->Get<Transform>(child);
                    trans.position = glm::vec3(targetPos.x, targetPos.y, trans.position.z);
                }
            }
        }

        // 7. Update UIEventComponents (General drag and drop, hover, click callbacks)
        auto eventEntities = context.ecs->Query<UIEventComponent>();
        for (Entity entity : eventEntities)
        {
            auto& ev = context.ecs->Get<UIEventComponent>(entity);
            
            // Get position and size
            glm::vec2 pos{0.0f};
            glm::vec2 size{0.0f};

            if (ev.customBoundsSize.x > 0.0f && ev.customBoundsSize.y > 0.0f)
            {
                pos = ev.customBoundsPosition;
                size = ev.customBoundsSize;
            }
            else if (context.ecs->Has<UIButtonComponent>(entity))
            {
                auto& c = context.ecs->Get<UIButtonComponent>(entity);
                pos = c.position;
                size = c.size;
            }
            else if (context.ecs->Has<UISliderComponent>(entity))
            {
                auto& c = context.ecs->Get<UISliderComponent>(entity);
                pos = c.position;
                size = c.size;
            }
            else if (context.ecs->Has<UIInputFieldComponent>(entity))
            {
                auto& c = context.ecs->Get<UIInputFieldComponent>(entity);
                pos = c.position;
                size = c.size;
            }
            else if (context.ecs->Has<UIImageComponent>(entity))
            {
                auto& c = context.ecs->Get<UIImageComponent>(entity);
                pos = c.position;
                size = c.size;
            }
            else if (context.ecs->Has<Transform>(entity) && context.ecs->Has<SpriteRenderer>(entity))
            {
                auto& t = context.ecs->Get<Transform>(entity);
                auto& s = context.ecs->Get<SpriteRenderer>(entity);
                pos = glm::vec2(t.position.x, t.position.y);
                size = s.size;
            }

            bool hover = (mouseX >= pos.x && mouseX <= pos.x + size.x &&
                          mouseY >= pos.y && mouseY <= pos.y + size.y);

            // Scissor clipping check if child of a scroll view
            if (scrollChildToParent.find(entity) != scrollChildToParent.end())
            {
                Entity parentSVEntity = scrollChildToParent[entity];
                auto& sv = context.ecs->Get<UIScrollViewComponent>(parentSVEntity);
                bool mouseInSV = (mouseX >= sv.position.x && mouseX <= sv.position.x + sv.size.x &&
                                  mouseY >= sv.position.y && mouseY <= sv.position.y + sv.size.y);
                if (!mouseInSV) hover = false;
            }

            // Hover state callbacks
            if (hover && !ev.isHovered)
            {
                ev.isHovered = true;
                if (ev.onHoverEnter) ev.onHoverEnter(entity);
            }
            else if (!hover && ev.isHovered)
            {
                ev.isHovered = false;
                if (ev.onHoverExit) ev.onHoverExit(entity);
            }

            if (ev.isHovered && ev.onHover)
            {
                ev.onHover(entity);
            }

            // Press and drag callbacks
            if (hover && mouseClicked)
            {
                ev.isPressed = true;
                ev.lastMousePos = glm::vec2(mouseX, mouseY);
                if (ev.onPress) ev.onPress(entity, ev.lastMousePos);
            }

            if (ev.isPressed)
            {
                if (mousePressed)
                {
                    glm::vec2 currentMousePos(mouseX, mouseY);
                    glm::vec2 delta = currentMousePos - ev.lastMousePos;
                    ev.lastMousePos = currentMousePos;
                    if (ev.onDrag) ev.onDrag(entity, currentMousePos, delta);
                }
                else
                {
                    ev.isPressed = false;
                    if (ev.onRelease) ev.onRelease(entity, glm::vec2(mouseX, mouseY));
                }
            }
        }
    }

    void UISystem::Render()
    {
        if (!context.ecs || !context.shader) return;

        static bool batchInit = false;
        if (!batchInit)
        {
            s_UIBatch.Init();
            batchInit = true;
        }

        // Gather all child entities that belong to Scroll Views to skip drawing their labels in the general loops
        std::unordered_set<Entity> scrollChildren;
        auto scrollViews = context.ecs->Query<UIScrollViewComponent>();
        for (Entity entity : scrollViews)
        {
            auto& sv = context.ecs->Get<UIScrollViewComponent>(entity);
            for (Entity child : sv.children)
            {
                scrollChildren.insert(child);
            }
        }

        // 1. Render Buttons Labels (skipping scroll view children) - text scale reduced to 0.25f to look professional
        auto buttons = context.ecs->Query<UIButtonComponent>();
        for (Entity entity : buttons)
        {
            if (scrollChildren.find(entity) != scrollChildren.end()) continue;

            auto& btn = context.ecs->Get<UIButtonComponent>(entity);
            // Center the label text using GetTextWidth
            float textWidth = GetTextWidth(btn.label, 0.25f);
            float textX = btn.position.x + (btn.size.x - textWidth) / 2.0f;
            float textY = btn.position.y + btn.size.y * 0.6f;
            RenderText(btn.label, textX, textY, 0.25f, btn.textColor);
        }

        // 2. Render Input Field Text (skipping scroll view children)
        auto inputs = context.ecs->Query<UIInputFieldComponent>();
        for (Entity entity : inputs)
        {
            if (scrollChildren.find(entity) != scrollChildren.end()) continue;

            auto& field = context.ecs->Get<UIInputFieldComponent>(entity);
            float textX = field.position.x + 10.0f;
            float textY = field.position.y + field.size.y * 0.6f;

            if (field.text.empty())
            {
                RenderText(field.placeholder, textX, textY, 0.25f, field.placeholderColor);
            }
            else
            {
                // Draw text with a cursor if focused
                std::string displayText = field.text + (field.isFocused && (int(glfwGetTime() * 2) % 2 == 0) ? "|" : "");
                RenderText(displayText, textX, textY, 0.25f, field.textColor);
            }
        }

        // 3. Render Dropdowns and their open option lists
        auto dropdowns = context.ecs->Query<UIDropdownComponent>();
        for (Entity entity : dropdowns)
        {
            auto& drop = context.ecs->Get<UIDropdownComponent>(entity);
            
            // Draw current selection text on top of main box
            std::string labelText = drop.options.empty() ? "" : drop.options[drop.selectedIndex];
            float textX = drop.position.x + 10.0f;
            float textY = drop.position.y + drop.size.y * 0.6f;
            RenderText(labelText, textX, textY, 0.25f, drop.textColor);

            if (drop.isOpen)
            {
                // Draw dropdown options menu using direct batching
                s_UIBatch.Begin();
                for (int i = 0; i < (int)drop.options.size(); ++i)
                {
                    float optY = drop.position.y + drop.size.y + i * drop.size.y;
                    glm::vec4 color = (i == drop.hoveredIndex) ? drop.hoverColor : drop.color;
                    s_UIBatch.DrawQuad({ drop.position.x, optY, 0.1f }, drop.size, color, 0);
                }
                s_UIBatch.End();
                s_UIBatch.Flush(*context.shader);

                // Draw option text labels
                for (int i = 0; i < (int)drop.options.size(); ++i)
                {
                    float optY = drop.position.y + drop.size.y + i * drop.size.y;
                    float optTextX = drop.position.x + 10.0f;
                    float optTextY = optY + drop.size.y * 0.6f;
                    RenderText(drop.options[i], optTextX, optTextY, 0.25f, drop.textColor);
                }
            }
        }

        // 4. Render General Sliders (skipping scroll view children)
        auto sliders = context.ecs->Query<UISliderComponent>();
        for (Entity entity : sliders)
        {
            if (scrollChildren.find(entity) != scrollChildren.end()) continue;

            auto& slider = context.ecs->Get<UISliderComponent>(entity);
            
            glm::vec2 thumbSize(12.0f, slider.size.y + 6.0f);
            float thumbX = slider.position.x + ((slider.value - slider.minValue) / (slider.maxValue - slider.minValue)) * slider.size.x - thumbSize.x * 0.5f;
            float thumbY = slider.position.y - 3.0f;
            
            glm::vec4 thumbColor = slider.isDragging ? slider.thumbHoverColor : slider.thumbColor;
            
            s_UIBatch.Begin();
            s_UIBatch.DrawQuad({ thumbX, thumbY, 0.2f }, thumbSize, thumbColor, 0);
            s_UIBatch.End();
            s_UIBatch.Flush(*context.shader);
            
            // Value text next to it
            char valStr[32];
            snprintf(valStr, sizeof(valStr), "%.2f", slider.value);
            RenderText(valStr, slider.position.x + slider.size.x + 10.0f, slider.position.y + slider.size.y * 0.6f, 0.25f, glm::vec4(1.0f));
        }

        // 5. Render Scroll View Scissor region (applying scissoring over children)
        for (Entity entity : scrollViews)
        {
            auto& sv = context.ecs->Get<UIScrollViewComponent>(entity);

            // Draw scroll view background
            s_UIBatch.Begin();
            s_UIBatch.DrawQuad({ sv.position.x, sv.position.y, -0.1f }, sv.size, sv.backgroundColor, 0);
            
            // Draw a small scrollbar thumb
            float thumbHeight = glm::max(20.0f, (sv.size.y / (sv.size.y + sv.maxScroll.y)) * sv.size.y);
            float thumbY = sv.position.y + (sv.scrollOffset.y / sv.maxScroll.y) * (sv.size.y - thumbHeight);
            s_UIBatch.DrawQuad({ sv.position.x + sv.size.x - 8.0f, thumbY, 0.2f }, { 6.0f, thumbHeight }, { 0.4f, 0.4f, 0.4f, 0.8f }, 0);
            
            s_UIBatch.End();
            s_UIBatch.Flush(*context.shader);

            // Enable scissor testing for clipping children UI elements
            int winWidth, winHeight;
            glfwGetWindowSize(context.window, &winWidth, &winHeight);

            float scaleX = (float)winWidth / context.virtualWidth;
            float scaleY = (float)winHeight / context.virtualHeight;

            int scissorX = static_cast<int>(sv.position.x * scaleX);
            int scissorY = static_cast<int>((context.virtualHeight - (sv.position.y + sv.size.y)) * scaleY);
            int scissorW = static_cast<int>(sv.size.x * scaleX);
            int scissorH = static_cast<int>(sv.size.y * scaleY);

            glEnable(GL_SCISSOR_TEST);
            glScissor(scissorX, scissorY, scissorW, scissorH);

            // Draw child background SpriteRenderers first in scissor region
            s_UIBatch.Begin();
            for (Entity child : sv.children)
            {
                if (context.ecs->Has<SpriteRenderer>(child) && context.ecs->Has<Transform>(child))
                {
                    auto& sprite = context.ecs->Get<SpriteRenderer>(child);
                    auto& trans = context.ecs->Get<Transform>(child);
                    uint32_t texID = sprite.texture ? sprite.texture->id : 0;
                    s_UIBatch.DrawQuad(trans.position, sprite.size, sprite.color, texID);
                }
            }
            s_UIBatch.End();
            s_UIBatch.Flush(*context.shader);

            // Draw children's text labels/thumbs inside scissored area
            for (Entity child : sv.children)
            {
                if (context.ecs->Has<UIButtonComponent>(child))
                {
                    auto& btn = context.ecs->Get<UIButtonComponent>(child);
                    float textWidth = GetTextWidth(btn.label, 0.25f);
                    float textX = btn.position.x + (btn.size.x - textWidth) / 2.0f;
                    float textY = btn.position.y + btn.size.y * 0.6f;
                    RenderText(btn.label, textX, textY, 0.25f, btn.textColor);
                }
                else if (context.ecs->Has<UIInputFieldComponent>(child))
                {
                    auto& field = context.ecs->Get<UIInputFieldComponent>(child);
                    float textX = field.position.x + 10.0f;
                    float textY = field.position.y + field.size.y * 0.6f;
                    if (field.text.empty())
                        RenderText(field.placeholder, textX, textY, 0.25f, field.placeholderColor);
                    else
                        RenderText(field.text, textX, textY, 0.25f, field.textColor);
                }
                else if (context.ecs->Has<UISliderComponent>(child))
                {
                    auto& slider = context.ecs->Get<UISliderComponent>(child);
                    
                    glm::vec2 thumbSize(12.0f, slider.size.y + 6.0f);
                    float thumbX = slider.position.x + ((slider.value - slider.minValue) / (slider.maxValue - slider.minValue)) * slider.size.x - thumbSize.x * 0.5f;
                    float thumbY = slider.position.y - 3.0f;
                    
                    glm::vec4 thumbColor = slider.isDragging ? slider.thumbHoverColor : slider.thumbColor;
                    
                    s_UIBatch.Begin();
                    s_UIBatch.DrawQuad({ thumbX, thumbY, 0.2f }, thumbSize, thumbColor, 0);
                    s_UIBatch.End();
                    s_UIBatch.Flush(*context.shader);
                    
                    char valStr[32];
                    snprintf(valStr, sizeof(valStr), "%.2f", slider.value);
                    RenderText(valStr, slider.position.x + slider.size.x + 10.0f, slider.position.y + slider.size.y * 0.6f, 0.25f, glm::vec4(1.0f));
                }
            }

            glDisable(GL_SCISSOR_TEST);
        }
    }

    void UISystem::CharCallback(unsigned int codepoint)
    {
        if (!context.ecs) return;
        auto inputs = context.ecs->Query<UIInputFieldComponent>();
        for (Entity entity : inputs)
        {
            auto& field = context.ecs->Get<UIInputFieldComponent>(entity);
            if (field.isFocused)
            {
                // Convert codepoint to UTF-8
                if (codepoint < 128)
                {
                    field.text += static_cast<char>(codepoint);
                }
                else
                {
                    if (codepoint <= 0x7FF)
                    {
                        field.text += static_cast<char>(0xC0 | ((codepoint >> 6) & 0x1F));
                        field.text += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                    else if (codepoint <= 0xFFFF)
                    {
                        field.text += static_cast<char>(0xE0 | ((codepoint >> 12) & 0x0F));
                        field.text += static_cast<char>(0x80 | ((codepoint >> 6) & 0x3F));
                        field.text += static_cast<char>(0x80 | (codepoint & 0x3F));
                    }
                }
                if (field.onTextChanged) field.onTextChanged(field.text);
            }
        }
    }

    // ==========================================
    // UI Factory Helpers (Olia namespace)
    // ==========================================

    Entity CreateImage(Texture* texture, glm::vec2 position, glm::vec2 size, glm::vec4 color)
    {
        Entity entity = context.ecs->Create();
        
        Transform trans;
        trans.position = glm::vec3(position.x, position.y, 0.0f);
        
        SpriteRenderer sprite;
        sprite.size = size;
        sprite.color = color;
        sprite.texture = texture;
        
        UIImageComponent imgComp{ position, size, color, texture };
        
        context.ecs->Add(entity, trans);
        context.ecs->Add(entity, sprite);
        context.ecs->Add(entity, imgComp);
        
        return entity;
    }

    Entity CreateButton(const std::string& label, glm::vec2 position, glm::vec2 size, std::function<void()> onClick,
                        glm::vec4 color, glm::vec4 hoverColor, glm::vec4 clickColor, glm::vec4 textColor)
    {
        Entity entity = context.ecs->Create();
        
        Transform trans;
        trans.position = glm::vec3(position.x, position.y, 0.0f);
        
        SpriteRenderer sprite;
        sprite.size = size;
        sprite.color = color;
        
        UIButtonComponent btnComp;
        btnComp.position = position;
        btnComp.size = size;
        btnComp.label = label;
        btnComp.color = color;
        btnComp.hoverColor = hoverColor;
        btnComp.clickColor = clickColor;
        btnComp.textColor = textColor;
        btnComp.onClick = onClick;
        
        context.ecs->Add(entity, trans);
        context.ecs->Add(entity, sprite);
        context.ecs->Add(entity, btnComp);
        
        return entity;
    }

    Entity CreateInputField(const std::string& placeholder, glm::vec2 position, glm::vec2 size,
                            std::function<void(const std::string&)> onTextChanged)
    {
        Entity entity = context.ecs->Create();
        
        Transform trans;
        trans.position = glm::vec3(position.x, position.y, 0.0f);
        
        SpriteRenderer sprite;
        sprite.size = size;
        sprite.color = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
        
        UIInputFieldComponent inputComp;
        inputComp.position = position;
        inputComp.size = size;
        inputComp.placeholder = placeholder;
        inputComp.onTextChanged = onTextChanged;
        
        context.ecs->Add(entity, trans);
        context.ecs->Add(entity, sprite);
        context.ecs->Add(entity, inputComp);
        
        return entity;
    }

    Entity CreateDropdown(const std::vector<std::string>& options, glm::vec2 position, glm::vec2 size,
                          std::function<void(int)> onSelectionChanged)
    {
        Entity entity = context.ecs->Create();
        
        Transform trans;
        trans.position = glm::vec3(position.x, position.y, 0.0f);
        
        SpriteRenderer sprite;
        sprite.size = size;
        sprite.color = glm::vec4(0.2f, 0.2f, 0.2f, 1.0f);
        
        UIDropdownComponent dropdownComp;
        dropdownComp.position = position;
        dropdownComp.size = size;
        dropdownComp.options = options;
        dropdownComp.onSelectionChanged = onSelectionChanged;
        
        context.ecs->Add(entity, trans);
        context.ecs->Add(entity, sprite);
        context.ecs->Add(entity, dropdownComp);
        
        return entity;
    }

    Entity CreateSlider(glm::vec2 position, glm::vec2 size, float minValue, float maxValue, float initialValue,
                        std::function<void(float)> onValueChanged)
    {
        Entity entity = context.ecs->Create();
        
        Transform trans;
        trans.position = glm::vec3(position.x, position.y, 0.0f);
        
        SpriteRenderer sprite;
        sprite.size = size;
        sprite.color = glm::vec4(0.15f, 0.15f, 0.15f, 1.0f);
        
        UISliderComponent sliderComp;
        sliderComp.position = position;
        sliderComp.size = size;
        sliderComp.value = initialValue;
        sliderComp.minValue = minValue;
        sliderComp.maxValue = maxValue;
        sliderComp.onValueChanged = onValueChanged;
        
        context.ecs->Add(entity, trans);
        context.ecs->Add(entity, sprite);
        context.ecs->Add(entity, sliderComp);
        
        return entity;
    }

    Entity CreateScrollView(glm::vec2 position, glm::vec2 size, glm::vec2 maxScroll)
    {
        Entity entity = context.ecs->Create();
        
        Transform trans;
        trans.position = glm::vec3(position.x, position.y, 0.0f);
        
        SpriteRenderer sprite;
        sprite.size = size;
        sprite.color = glm::vec4(0.1f, 0.1f, 0.1f, 0.5f);
        
        UIScrollViewComponent svComp;
        svComp.position = position;
        svComp.size = size;
        svComp.maxScroll = maxScroll;
        
        context.ecs->Add(entity, trans);
        context.ecs->Add(entity, sprite);
        context.ecs->Add(entity, svComp);
        
        return entity;
    }

    Entity CreateGridLayout(glm::vec2 position, glm::vec2 cellSize, glm::vec2 spacing, int columns)
    {
        Entity entity = context.ecs->Create();
        
        UIGridLayoutComponent gridComp;
        gridComp.position = position;
        gridComp.cellSize = cellSize;
        gridComp.spacing = spacing;
        gridComp.columns = columns;
        
        context.ecs->Add(entity, gridComp);
        
        return entity;
    }

    Entity CreateText(const std::string& text, glm::vec2 position, float scale, glm::vec4 color)
    {
        Entity entity = context.ecs->Create();
        
        TextComponent textComp;
        textComp.text = text;
        textComp.position = position;
        textComp.scale = scale;
        textComp.color = color;
        
        context.ecs->Add(entity, textComp);
        
        return entity;
    }

    void AddEventSupport(Entity entity, 
                         std::function<void(Entity)> onHoverEnter,
                         std::function<void(Entity)> onHoverExit,
                         std::function<void(Entity, glm::vec2, glm::vec2)> onDrag)
    {
        UIEventComponent eventComp;
        eventComp.onHoverEnter = onHoverEnter;
        eventComp.onHoverExit = onHoverExit;
        eventComp.onDrag = onDrag;
        
        context.ecs->Add(entity, eventComp);
    }
}

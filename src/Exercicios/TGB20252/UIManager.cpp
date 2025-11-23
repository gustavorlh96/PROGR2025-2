#include "UIManager.h"

UIManager::UIManager(int width, int height) 
    : windowWidth(width), windowHeight(height), currentMode(UIMode::VIDEO), showDescription(false) {}

void UIManager::setMode(UIMode mode) {
    currentMode = mode;
}

UIMode UIManager::getMode() const {
    return currentMode;
}

void UIManager::addButton(const std::string& label, float x, float y, float width, float height,
                          std::function<void()> onClick, cv::Scalar color) {
    Button btn;
    btn.label = label;
    btn.x = x;
    btn.y = y;
    btn.width = width;
    btn.height = height;
    btn.onClick = onClick;
    btn.visible = true;
    btn.color = color;
    btn.hoverColor = cv::Scalar(color[0] + 30, color[1] + 30, color[2] + 30);
    btn.isHovered = false;
    buttons.push_back(btn);
}

void UIManager::addSlider(const std::string& label, float x, float y, float width, float height,
                          float minVal, float maxVal, float defaultVal, std::function<void(float)> onChange) {
    Slider slider;
    slider.label = label;
    slider.x = x;
    slider.y = y;
    slider.width = width;
    slider.height = height;
    slider.minValue = minVal;
    slider.maxValue = maxVal;
    slider.currentValue = defaultVal;
    slider.onChange = onChange;
    slider.visible = true;
    slider.isDragging = false;
    sliders.push_back(slider);
}

void UIManager::addDropdown(const std::string& label, float x, float y, float width, float height,
                           const std::vector<std::string>& options, std::function<void(int)> onSelect) {
    DropdownMenu dropdown;
    dropdown.label = label;
    dropdown.x = x;
    dropdown.y = y;
    dropdown.width = width;
    dropdown.height = height;
    dropdown.options = options;
    dropdown.selectedIndex = 0;
    dropdown.isOpen = false;
    dropdown.onSelect = onSelect;
    dropdown.visible = true;
    dropdowns.push_back(dropdown);
}

void UIManager::clearButtons() {
    buttons.clear();
}

void UIManager::clearSliders() {
    sliders.clear();
}

void UIManager::clearDropdowns() {
    dropdowns.clear();
}

void UIManager::showFilterDescription(const std::string& description) {
    filterDescription = description;
    showDescription = true;
}

void UIManager::hideFilterDescription() {
    showDescription = false;
}

void UIManager::render(cv::Mat& image) {
    if (image.empty()) return;
    
    for (auto& button : buttons) {
        if (button.visible) {
            drawButton(image, button);
        }
    }
    
    for (auto& slider : sliders) {
        if (slider.visible) {
            drawSlider(image, slider);
        }
    }
    
    for (auto& dropdown : dropdowns) {
        if (dropdown.visible) {
            drawDropdown(image, dropdown);
        }
    }
    
    if (showDescription && !filterDescription.empty()) {
        int margin = 20;
        int padding = 15;
        int lineHeight = 25;
        
        std::vector<std::string> lines;
        std::string currentLine;
        for (char c : filterDescription) {
            if (c == '\n' || currentLine.length() > 60) {
                lines.push_back(currentLine);
                currentLine.clear();
            }
            if (c != '\n') {
                currentLine += c;
            }
        }
        if (!currentLine.empty()) {
            lines.push_back(currentLine);
        }
        
        int boxHeight = padding * 2 + lineHeight * lines.size();
        int boxWidth = windowWidth - 2 * margin;
        int boxY = windowHeight - margin - boxHeight;
        
        cv::Rect descBox(margin, boxY, boxWidth, boxHeight);
        drawRect(image, descBox, cv::Scalar(30, 30, 30, 200));
        
        cv::rectangle(image, descBox, cv::Scalar(100, 100, 100), 2);
        
        for (size_t i = 0; i < lines.size(); i++) {
            cv::Point textPos(margin + padding, boxY + padding + 20 + i * lineHeight);
            drawText(image, lines[i], textPos, 0.5, cv::Scalar(255, 255, 255), 1);
        }
    }
}

void UIManager::handleMouseClick(double x, double y) {
    for (auto& dropdown : dropdowns) {
        if (!dropdown.visible) continue;
        
        if (isPointInRect(x, y, dropdown.x, dropdown.y, dropdown.width, dropdown.height)) {
            dropdown.isOpen = !dropdown.isOpen;
            return;
        }
        
        if (dropdown.isOpen) {
            float optionHeight = 35;
            for (size_t i = 0; i < dropdown.options.size(); i++) {
                float optY = dropdown.y + dropdown.height + i * optionHeight;
                if (isPointInRect(x, y, dropdown.x, optY, dropdown.width, optionHeight)) {
                    dropdown.selectedIndex = i;
                    dropdown.isOpen = false;
                    if (dropdown.onSelect) {
                        dropdown.onSelect(i);
                    }
                    return;
                }
            }
        }
    }
    
    for (auto& button : buttons) {
        if (button.visible && isPointInRect(x, y, button.x, button.y, button.width, button.height)) {
            if (button.onClick) {
                button.onClick();
            }
            return;
        }
    }
    
    for (auto& slider : sliders) {
        if (slider.visible && isPointInRect(x, y, slider.x, slider.y, slider.width, slider.height)) {
            slider.isDragging = true;
            
            float ratio = (x - slider.x) / slider.width;
            ratio = std::max(0.0f, std::min(1.0f, ratio));
            slider.currentValue = slider.minValue + ratio * (slider.maxValue - slider.minValue);
            
            if (slider.onChange) {
                slider.onChange(slider.currentValue);
            }
            return;
        }
    }
}

void UIManager::handleMouseMove(double x, double y) {
    for (auto& button : buttons) {
        button.isHovered = button.visible && isPointInRect(x, y, button.x, button.y, button.width, button.height);
    }
    
    for (auto& slider : sliders) {
        if (slider.isDragging) {
            float ratio = (x - slider.x) / slider.width;
            ratio = std::max(0.0f, std::min(1.0f, ratio));
            slider.currentValue = slider.minValue + ratio * (slider.maxValue - slider.minValue);
            
            if (slider.onChange) {
                slider.onChange(slider.currentValue);
            }
        }
    }
}

void UIManager::handleMouseRelease() {
    for (auto& slider : sliders) {
        slider.isDragging = false;
    }
}

void UIManager::drawButton(cv::Mat& image, const Button& button) {
    cv::Scalar color = button.isHovered ? button.hoverColor : button.color;
    cv::Rect rect(button.x, button.y, button.width, button.height);
    
    drawRect(image, rect, color);
    
    cv::Scalar borderColor = button.isHovered ? cv::Scalar(255, 255, 255) : cv::Scalar(180, 180, 180);
    cv::rectangle(image, rect, borderColor, 3);
    
    int baseline = 0;
    double fontScale = 0.4;
    int thickness = 1;
    
    if (button.width >= 60) {
        fontScale = 0.5;
        thickness = 2;
    }
    
    cv::Size textSize = cv::getTextSize(button.label, cv::FONT_HERSHEY_SIMPLEX, fontScale, thickness, &baseline);
    
    cv::Point textPos(
        button.x + (button.width - textSize.width) / 2,
        button.y + (button.height + textSize.height) / 2
    );
    
    cv::Scalar textColor = cv::Scalar(255, 255, 255);
    if (button.color[0] > 200 && button.color[1] > 200 && button.color[2] > 200) {
        textColor = cv::Scalar(50, 50, 50);
    }
    
    drawText(image, button.label, textPos, fontScale, textColor, thickness);
}

void UIManager::drawSlider(cv::Mat& image, const Slider& slider) {
    cv::Rect track(slider.x, slider.y + slider.height / 2 - 3, slider.width, 6);
    drawRect(image, track, cv::Scalar(100, 100, 100));
    
    float ratio = (slider.currentValue - slider.minValue) / (slider.maxValue - slider.minValue);
    int thumbX = slider.x + ratio * slider.width;
    
    cv::circle(image, cv::Point(thumbX, slider.y + slider.height / 2), 12, cv::Scalar(200, 200, 200), -1);
    cv::circle(image, cv::Point(thumbX, slider.y + slider.height / 2), 12, cv::Scalar(255, 255, 255), 2);
    
    std::string valueText = slider.label + ": " + std::to_string((int)slider.currentValue);
    drawText(image, valueText, cv::Point(slider.x, slider.y - 5), 0.4, cv::Scalar(255, 255, 255), 1);
}

void UIManager::drawText(cv::Mat& image, const std::string& text, cv::Point position,
                         double scale, cv::Scalar color, int thickness) {
    cv::putText(image, text, position, cv::FONT_HERSHEY_SIMPLEX, scale, color, thickness);
}

void UIManager::drawRect(cv::Mat& image, cv::Rect rect, cv::Scalar color, int thickness) {
    if (thickness < 0) {
        cv::rectangle(image, rect, color, cv::FILLED);
    } else {
        cv::rectangle(image, rect, color, thickness);
    }
}

bool UIManager::isPointInRect(double x, double y, float rx, float ry, float rw, float rh) {
    return x >= rx && x <= rx + rw && y >= ry && y <= ry + rh;
}

void UIManager::drawDropdown(cv::Mat& image, const DropdownMenu& dropdown) {
    cv::Rect mainRect(dropdown.x, dropdown.y, dropdown.width, dropdown.height);
    drawRect(image, mainRect, cv::Scalar(40, 40, 40, 230));
    cv::rectangle(image, mainRect, cv::Scalar(200, 200, 200), 2);
    
    std::string displayText = dropdown.selectedIndex >= 0 && dropdown.selectedIndex < (int)dropdown.options.size()
                              ? dropdown.options[dropdown.selectedIndex]
                              : dropdown.label;
    
    int baseline = 0;
    cv::Size textSize = cv::getTextSize(displayText, cv::FONT_HERSHEY_SIMPLEX, 0.45, 1, &baseline);
    cv::Point textPos(dropdown.x + 10, dropdown.y + (dropdown.height + textSize.height) / 2);
    drawText(image, displayText, textPos, 0.45, cv::Scalar(255, 255, 255), 1);
    
    cv::Point arrow1(dropdown.x + dropdown.width - 20, dropdown.y + dropdown.height / 2 - 3);
    cv::Point arrow2(dropdown.x + dropdown.width - 10, dropdown.y + dropdown.height / 2 + 5);
    cv::Point arrow3(dropdown.x + dropdown.width - 30, dropdown.y + dropdown.height / 2 + 5);
    std::vector<cv::Point> arrowPts = {arrow1, arrow2, arrow3};
    cv::fillConvexPoly(image, arrowPts, cv::Scalar(200, 200, 200));
    
    if (dropdown.isOpen) {
        float optionHeight = 35;
        for (size_t i = 0; i < dropdown.options.size(); i++) {
            float optY = dropdown.y + dropdown.height + i * optionHeight;
            cv::Rect optRect(dropdown.x, optY, dropdown.width, optionHeight);
            
            cv::Scalar optColor = (i == dropdown.selectedIndex) 
                                  ? cv::Scalar(60, 60, 60, 240) 
                                  : cv::Scalar(35, 35, 35, 240);
            drawRect(image, optRect, optColor);
            cv::rectangle(image, optRect, cv::Scalar(150, 150, 150), 1);
            
            cv::Size optTextSize = cv::getTextSize(dropdown.options[i], cv::FONT_HERSHEY_SIMPLEX, 0.4, 1, &baseline);
            cv::Point optTextPos(dropdown.x + 10, optY + (optionHeight + optTextSize.height) / 2);
            drawText(image, dropdown.options[i], optTextPos, 0.4, cv::Scalar(255, 255, 255), 1);
        }
    }
}

#ifndef UI_MANAGER_H
#define UI_MANAGER_H

#include <glad/glad.h>
#include <GLFW/glfw3.h>
#include <opencv2/opencv.hpp>
#include <vector>
#include <string>
#include <functional>

struct Button {
    float x, y, width, height;
    std::string label;
    std::function<void()> onClick;
    bool visible;
    cv::Scalar color;
    cv::Scalar hoverColor;
    bool isHovered;
};

struct Slider {
    float x, y, width, height;
    std::string label;
    float minValue, maxValue, currentValue;
    std::function<void(float)> onChange;
    bool visible;
    bool isDragging;
};

enum class UIMode {
    PHOTO,
    VIDEO
};

struct DropdownMenu {
    float x, y, width, height;
    std::string label;
    std::vector<std::string> options;
    int selectedIndex;
    bool isOpen;
    std::function<void(int)> onSelect;
    bool visible;
};

class UIManager {
public:
    UIManager(int windowWidth, int windowHeight);
    
    void render(cv::Mat& image);
    void handleMouseClick(double x, double y);
    void handleMouseMove(double x, double y);
    void handleMouseRelease();
    
    void addButton(const std::string& label, float x, float y, float width, float height, 
                   std::function<void()> onClick, cv::Scalar color = cv::Scalar(70, 70, 70));
    void addSlider(const std::string& label, float x, float y, float width, float height,
                   float minVal, float maxVal, float defaultVal, std::function<void(float)> onChange);
    void addDropdown(const std::string& label, float x, float y, float width, float height,
                    const std::vector<std::string>& options, std::function<void(int)> onSelect);
    
    void clearButtons();
    void clearSliders();
    void clearDropdowns();
    void setMode(UIMode mode);
    UIMode getMode() const;
    
    void showFilterDescription(const std::string& description);
    void hideFilterDescription();
    
private:
    int windowWidth;
    int windowHeight;
    UIMode currentMode;
    
    std::vector<Button> buttons;
    std::vector<Slider> sliders;
    std::vector<DropdownMenu> dropdowns;
    
    std::string filterDescription;
    bool showDescription;
    
    void drawButton(cv::Mat& image, const Button& button);
    void drawSlider(cv::Mat& image, const Slider& slider);
    void drawDropdown(cv::Mat& image, const DropdownMenu& dropdown);
    void drawText(cv::Mat& image, const std::string& text, cv::Point position, 
                  double scale, cv::Scalar color, int thickness);
    void drawRect(cv::Mat& image, cv::Rect rect, cv::Scalar color, int thickness = -1);
    
    bool isPointInRect(double x, double y, float rx, float ry, float rw, float rh);
};

#endif

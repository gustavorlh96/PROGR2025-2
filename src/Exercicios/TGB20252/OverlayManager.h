#ifndef OVERLAY_MANAGER_H
#define OVERLAY_MANAGER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <unordered_map>
#include <vector>

enum class OverlayType {
    NONE = 0,
    HLA_GLYPH,
    HIPSTER,
    SUMMER
};

struct OverlayOption {
    OverlayType type;
    std::string label;
    std::string asset;
};

class OverlayManager {
public:
    void load(int width, int height);
    cv::Mat apply(const cv::Mat& base, OverlayType type) const;
    const std::vector<OverlayOption>& options() const;

private:
    std::vector<OverlayOption> entries;
    std::unordered_map<OverlayType, cv::Mat> textures;
    cv::Size targetSize{0, 0};
};

#endif

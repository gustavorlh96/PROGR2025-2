#ifndef FILTER_MANAGER_H
#define FILTER_MANAGER_H

#include <opencv2/opencv.hpp>
#include <string>
#include <vector>

enum class FilterType {
    NONE,
    BILATERAL_FILTERING,
    BOX_BLUR,
    MEDIAN_BLUR,
    PORTRAIT_BLUR,
    SHARPEN,
    LAPLACIAN,
    SOBEL,
    CANNY,
    GRAYSCALE,
    SEPIA,
    INVERT,
    BRIGHTNESS,
    CONTRAST,
    EMBOSS,
    RGB_CHANNELS,
    VHS
};

enum class ChannelMode {
    RGB,
    RED,
    GREEN,
    BLUE,
    GRAYSCALE
};

struct FilterInfo {
    FilterType type;
    std::string name;
    std::string description;
};

class FilterManager {
public:
    FilterManager();
    
    cv::Mat applyFilter(const cv::Mat& input, FilterType filter, ChannelMode channel = ChannelMode::RGB);
    std::vector<FilterInfo> getAvailableFilters() const;
    std::string getFilterDescription(FilterType filter) const;
    
    void setKernelSize(int size);
    void setBrightnessValue(int value);
    void setContrastValue(double value);
    void setFaceMask(const cv::Mat& mask);
    
    void setRGBChannels(bool r, bool g, bool b);
    void getRGBChannels(bool& r, bool& g, bool& b) const;
    
private:
    int kernelSize;
    int brightnessValue;
    double contrastValue;
    cv::Mat faceMask;
    
    bool enableR, enableG, enableB;
    
    cv::Mat applyChannelMode(const cv::Mat& input, ChannelMode channel);
    cv::Mat gaussianBlur(const cv::Mat& input);
    cv::Mat boxBlur(const cv::Mat& input);
    cv::Mat medianBlur(const cv::Mat& input);
    cv::Mat portraitBlur(const cv::Mat& input);
    cv::Mat sharpen(const cv::Mat& input);
    cv::Mat laplacian(const cv::Mat& input);
    cv::Mat sobel(const cv::Mat& input);
    cv::Mat canny(const cv::Mat& input);
    cv::Mat grayscale(const cv::Mat& input);
    cv::Mat sepia(const cv::Mat& input);
    cv::Mat invert(const cv::Mat& input);
    cv::Mat brightness(const cv::Mat& input);
    cv::Mat contrast(const cv::Mat& input);
    cv::Mat emboss(const cv::Mat& input);
    cv::Mat rgbChannels(const cv::Mat& input);
    cv::Mat vhs(const cv::Mat& input);
};

#endif

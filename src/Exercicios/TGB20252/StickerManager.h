#ifndef STICKER_MANAGER_H
#define STICKER_MANAGER_H

#include <opencv2/opencv.hpp>
#include <vector>
#include <string>

struct Sticker {
    cv::Mat image;
    cv::Point position;
    float scale;
    int id;
    bool active;
};

class StickerManager {
public:
    StickerManager();
    
    bool loadStickers();
    void addSticker(int stickerIndex, cv::Point position);
    void removeSticker(int index);
    void clearStickers();
    cv::Mat applyStickers(const cv::Mat& baseImage);
    int getStickerCount() const;
    const std::vector<cv::Mat>& getAvailableStickers() const;
    void setScale(float scale);
    
    void updateStickerPosition(int stickerId, cv::Point newPosition);
    int findStickerAtPosition(cv::Point pos) const;
    cv::Mat renderPreview(const cv::Mat& baseImage, int stickerIndex, cv::Point position, float alpha = 0.5f) const;
    
private:
    std::vector<cv::Mat> availableStickers;
    std::vector<Sticker> activeStickers;
    float defaultScale;
    int nextId;
    
    cv::Mat overlayImage(const cv::Mat& background, const cv::Mat& foreground, cv::Point position, float alpha = 1.0f) const;
};

#endif

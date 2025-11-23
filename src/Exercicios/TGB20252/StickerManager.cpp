#include "StickerManager.h"

StickerManager::StickerManager() : defaultScale(0.15f), nextId(0) {}

bool StickerManager::loadStickers() {
    availableStickers.clear();
    
    std::vector<std::string> stickerFiles = {
        "../assets/stickers/aperture.png",
        "../assets/stickers/cat.png",
        "../assets/stickers/chicken_jockey.png",
        "../assets/stickers/companion_cube.png",
        "../assets/stickers/cpp_logo.png",
        "../assets/stickers/hl3.png",
        "../assets/stickers/hollow_knight.png",
        "../assets/stickers/nyan_cat.png",
        "../assets/stickers/opencv_logo.png"
    };
    
    const int STANDARD_HEIGHT = 256;
    
    for (const auto& path : stickerFiles) {
        cv::Mat sticker = cv::imread(path, cv::IMREAD_UNCHANGED);
        
        if (!sticker.empty()) {
            if (sticker.rows != STANDARD_HEIGHT) {
                float scale = (float)STANDARD_HEIGHT / sticker.rows;
                int newWidth = (int)(sticker.cols * scale);
                cv::Mat resized;
                cv::resize(sticker, resized, cv::Size(newWidth, STANDARD_HEIGHT));
                availableStickers.push_back(resized);
            } else {
                availableStickers.push_back(sticker);
            }
        } else {
            cv::Mat placeholder(100, 100, CV_8UC4, cv::Scalar(255, 0, 255, 200));
            cv::putText(placeholder, "?", cv::Point(35, 60), 
                       cv::FONT_HERSHEY_SIMPLEX, 1.5, cv::Scalar(255, 255, 255, 255), 2);
            availableStickers.push_back(placeholder);
        }
    }
    
    return !availableStickers.empty();
}

void StickerManager::addSticker(int stickerIndex, cv::Point position) {
    if (stickerIndex < 0 || stickerIndex >= (int)availableStickers.size()) {
        return;
    }
    
    Sticker sticker;
    sticker.image = availableStickers[stickerIndex].clone();
    sticker.position = position;
    sticker.scale = 1.0f;
    sticker.id = nextId++;
    sticker.active = true;
    
    activeStickers.push_back(sticker);
}

void StickerManager::removeSticker(int index) {
    if (index >= 0 && index < (int)activeStickers.size()) {
        activeStickers.erase(activeStickers.begin() + index);
    }
}

void StickerManager::clearStickers() {
    activeStickers.clear();
}

cv::Mat StickerManager::applyStickers(const cv::Mat& baseImage) {
    if (baseImage.empty()) return baseImage;
    
    cv::Mat result = baseImage.clone();
    
    for (const auto& sticker : activeStickers) {
        if (sticker.active && !sticker.image.empty()) {
            result = overlayImage(result, sticker.image, sticker.position);
        }
    }
    
    return result;
}

int StickerManager::getStickerCount() const {
    return activeStickers.size();
}

const std::vector<cv::Mat>& StickerManager::getAvailableStickers() const {
    return availableStickers;
}

void StickerManager::setScale(float scale) {
    defaultScale = scale;
}

cv::Mat StickerManager::overlayImage(const cv::Mat& background, const cv::Mat& foreground, cv::Point position, float alpha) const {
    cv::Mat result = background.clone();
    
    if (foreground.channels() != 4) {
        return result;
    }
    
    int x = position.x - foreground.cols / 2;
    int y = position.y - foreground.rows / 2;
    
    for (int i = 0; i < foreground.rows; i++) {
        int bgY = y + i;
        if (bgY < 0 || bgY >= background.rows) continue;
        
        for (int j = 0; j < foreground.cols; j++) {
            int bgX = x + j;
            if (bgX < 0 || bgX >= background.cols) continue;
            
            cv::Vec4b fgPixel = foreground.at<cv::Vec4b>(i, j);
            float fgAlpha = (fgPixel[3] / 255.0f) * alpha;
            
            if (fgAlpha > 0) {
                cv::Vec3b& bgPixel = result.at<cv::Vec3b>(bgY, bgX);
                
                bgPixel[0] = cv::saturate_cast<uchar>((1.0f - fgAlpha) * bgPixel[0] + fgAlpha * fgPixel[0]);
                bgPixel[1] = cv::saturate_cast<uchar>((1.0f - fgAlpha) * bgPixel[1] + fgAlpha * fgPixel[1]);
                bgPixel[2] = cv::saturate_cast<uchar>((1.0f - fgAlpha) * bgPixel[2] + fgAlpha * fgPixel[2]);
            }
        }
    }
    
    return result;
}

void StickerManager::updateStickerPosition(int stickerId, cv::Point newPosition) {
    for (auto& sticker : activeStickers) {
        if (sticker.id == stickerId) {
            sticker.position = newPosition;
            break;
        }
    }
}

int StickerManager::findStickerAtPosition(cv::Point pos) const {
    for (int i = activeStickers.size() - 1; i >= 0; i--) {
        const auto& sticker = activeStickers[i];
        if (!sticker.active) continue;
        
        int halfW = sticker.image.cols / 2;
        int halfH = sticker.image.rows / 2;
        
        cv::Rect bounds(
            sticker.position.x - halfW,
            sticker.position.y - halfH,
            sticker.image.cols,
            sticker.image.rows
        );
        
        if (bounds.contains(pos)) {
            return sticker.id;
        }
    }
    return -1;
}

cv::Mat StickerManager::renderPreview(const cv::Mat& baseImage, int stickerIndex, cv::Point position, float alpha) const {
    if (stickerIndex < 0 || stickerIndex >= (int)availableStickers.size()) {
        return baseImage;
    }
    
    const cv::Mat& stickerTemplate = availableStickers[stickerIndex];
    if (stickerTemplate.empty()) return baseImage;
    
    return overlayImage(baseImage, stickerTemplate, position, alpha);
}

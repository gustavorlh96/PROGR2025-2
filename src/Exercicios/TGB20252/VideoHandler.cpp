#include "VideoHandler.h"
#include <algorithm>

VideoHandler::VideoHandler() : playing(false), videoFPS(30.0), accumulator(0.0), frameInterval(1.0/30.0) {}

VideoHandler::~VideoHandler() {
    if (capture.isOpened()) {
        capture.release();
    }
}

bool VideoHandler::loadVideo(const std::string& path) {
    videoPath = path;
    capture.open(path);
    
    if (!capture.isOpened()) {
        return false;
    }
    
    videoFPS = capture.get(cv::CAP_PROP_FPS);
    if (videoFPS <= 0 || videoFPS > 120) {
        videoFPS = 30.0;
    }
    frameInterval = 1.0 / videoFPS;
    accumulator = 0.0;
    
    capture >> currentFrame;
    
    if (!currentFrame.empty()) {
        cv::rotate(currentFrame, currentFrame, cv::ROTATE_90_COUNTERCLOCKWISE);
        cv::resize(currentFrame, currentFrame, cv::Size(540, 960));
    }
    
    return !currentFrame.empty();
}

cv::Mat VideoHandler::getNextFrame(double deltaTime) {
    if (!playing || !capture.isOpened()) {
        return currentFrame;
    }

    accumulator += std::max(0.0, deltaTime);
    
    // Process multiple frames if needed to keep up with real-time playback
    while (accumulator >= frameInterval) {
        accumulator -= frameInterval;
        
        cv::Mat frame;
        capture >> frame;

        if (frame.empty()) {
            capture.set(cv::CAP_PROP_POS_FRAMES, 0);
            capture >> frame;
        }

        if (!frame.empty()) {
            cv::rotate(frame, frame, cv::ROTATE_90_COUNTERCLOCKWISE);
            cv::resize(frame, currentFrame, cv::Size(540, 960));
        }
    }

    return currentFrame;
}

cv::Mat VideoHandler::getCurrentFrame() const {
    return currentFrame.clone();
}

void VideoHandler::reset() {
    if (capture.isOpened()) {
        capture.set(cv::CAP_PROP_POS_FRAMES, 0);
        cv::Mat frame;
        capture >> frame;
        if (!frame.empty()) {
            cv::rotate(frame, frame, cv::ROTATE_90_COUNTERCLOCKWISE);
            cv::resize(frame, currentFrame, cv::Size(540, 960));
        }
    }
}

bool VideoHandler::isPlaying() const {
    return playing;
}

void VideoHandler::setPlaying(bool p) {
    playing = p;
}

int VideoHandler::getWidth() const {
    return currentFrame.cols;
}

int VideoHandler::getHeight() const {
    return currentFrame.rows;
}

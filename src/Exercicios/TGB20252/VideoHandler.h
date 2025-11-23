#ifndef VIDEO_HANDLER_H
#define VIDEO_HANDLER_H

#include <opencv2/opencv.hpp>
#include <string>

class VideoHandler {
public:
    VideoHandler();
    ~VideoHandler();
    
    bool loadVideo(const std::string& path);
    cv::Mat getNextFrame(double deltaTime);
    cv::Mat getCurrentFrame() const;
    void reset();
    bool isPlaying() const;
    void setPlaying(bool playing);
    int getWidth() const;
    int getHeight() const;
    
private:
    cv::VideoCapture capture;
    cv::Mat currentFrame;
    std::string videoPath;
    bool playing;
    double videoFPS;
    double accumulator;
    double frameInterval;
};

#endif

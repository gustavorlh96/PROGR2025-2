#ifndef FACE_DETECTOR_H
#define FACE_DETECTOR_H

#include <opencv2/opencv.hpp>
#include <opencv2/objdetect.hpp>
#include <vector>

struct FaceData {
    cv::Rect boundingBox;
    std::vector<cv::Point> landmarks;
};

class FaceDetector {
public:
    FaceDetector();
    ~FaceDetector();
    
    bool initialize();
    std::vector<FaceData> detectFaces(const cv::Mat& image);
    void drawFaces(cv::Mat& image, const std::vector<FaceData>& faces);
    cv::Mat createFaceMask(const cv::Mat& image, const std::vector<FaceData>& faces);
    bool isInitialized() const;
    
private:
    bool loadCascade(cv::CascadeClassifier& cascade, const std::vector<std::string>& paths);

    cv::CascadeClassifier faceCascade;
    cv::CascadeClassifier profileCascade;
    bool frontalLoaded;
    bool profileLoaded;
    bool initialized;
};

#endif

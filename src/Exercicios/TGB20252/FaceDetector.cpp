#include "FaceDetector.h"
#include <opencv2/core/utils/filesystem.hpp>

namespace {
std::vector<std::string> buildCandidatePaths(const std::vector<std::string>& preferred, const std::string& sampleRelative) {
    std::vector<std::string> candidates = preferred;
    try {
        std::string samplePath = cv::samples::findFile(sampleRelative, false, true);
        if (!samplePath.empty()) {
            candidates.push_back(samplePath);
        }
    } catch (...) {
    }
    return candidates;
}
}

FaceDetector::FaceDetector() : frontalLoaded(false), profileLoaded(false), initialized(false) {}

FaceDetector::~FaceDetector() {}

bool FaceDetector::initialize() {
    std::vector<std::string> frontalPaths = buildCandidatePaths({
        "../assets/models/haarcascade_frontalface_default.xml",
        "assets/models/haarcascade_frontalface_default.xml",
        "haarcascade_frontalface_default.xml",
        "../data/haarcascades/haarcascade_frontalface_default.xml",
        "/usr/share/opencv4/haarcascades/haarcascade_frontalface_default.xml",
        "C:/opencv/data/haarcascades/haarcascade_frontalface_default.xml"
    }, "haarcascades/haarcascade_frontalface_default.xml");
    std::vector<std::string> profilePaths = buildCandidatePaths({
        "../assets/models/haarcascade_profileface.xml",
        "assets/models/haarcascade_profileface.xml",
        "haarcascade_profileface.xml",
        "../data/haarcascades/haarcascade_profileface.xml",
        "/usr/share/opencv4/haarcascades/haarcascade_profileface.xml",
        "C:/opencv/data/haarcascades/haarcascade_profileface.xml"
    }, "haarcascades/haarcascade_profileface.xml");
    
    frontalLoaded = loadCascade(faceCascade, frontalPaths);
    profileLoaded = loadCascade(profileCascade, profilePaths);
    initialized = frontalLoaded || profileLoaded;
    return initialized;
}

std::vector<FaceData> FaceDetector::detectFaces(const cv::Mat& image) {
    std::vector<FaceData> faces;
    
    if (!initialized || image.empty()) {
        return faces;
    }
    
    cv::Mat gray;
    if (image.channels() == 3) {
        cv::cvtColor(image, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = image.clone();
    }
    
    cv::equalizeHist(gray, gray);
    
    std::vector<cv::Rect> detectedFaces;
    if (frontalLoaded) {
        std::vector<cv::Rect> frontal;
        faceCascade.detectMultiScale(gray, frontal, 1.08, 4, 0, cv::Size(30, 30));
        detectedFaces.insert(detectedFaces.end(), frontal.begin(), frontal.end());
    }
    if (profileLoaded) {
        std::vector<cv::Rect> profiles;
        profileCascade.detectMultiScale(gray, profiles, 1.12, 4, 0, cv::Size(24, 24));
        detectedFaces.insert(detectedFaces.end(), profiles.begin(), profiles.end());
    }

    if (detectedFaces.size() > 1) {
        cv::groupRectangles(detectedFaces, 1, 0.2);
    }

    for (const auto& rect : detectedFaces) {
        FaceData faceData;
        faceData.boundingBox = rect;
        faces.push_back(faceData);
    }
    
    return faces;
}

void FaceDetector::drawFaces(cv::Mat& image, const std::vector<FaceData>& faces) {
    for (const auto& face : faces) {
        cv::rectangle(image, face.boundingBox, cv::Scalar(0, 255, 0), 2);
        
        cv::Point center(
            face.boundingBox.x + face.boundingBox.width / 2,
            face.boundingBox.y + face.boundingBox.height / 2
        );
        
        int radius = (face.boundingBox.width + face.boundingBox.height) / 4;
        cv::circle(image, center, radius, cv::Scalar(255, 0, 0), 2);
    }
}

bool FaceDetector::isInitialized() const {
    return initialized;
}

bool FaceDetector::loadCascade(cv::CascadeClassifier& cascade, const std::vector<std::string>& paths) {
    for (const auto& path : paths) {
        if (path.empty()) {
            continue;
        }
        if (cascade.load(path)) {
            return true;
        }
    }
    return false;
}

cv::Mat FaceDetector::createFaceMask(const cv::Mat& image, const std::vector<FaceData>& faces) {
    cv::Mat mask = cv::Mat::zeros(image.size(), CV_8UC1);
    
    for (const auto& face : faces) {
        cv::Point center(
            face.boundingBox.x + face.boundingBox.width / 2,
            face.boundingBox.y + face.boundingBox.height / 2
        );
        
        int radiusW = static_cast<int>(face.boundingBox.width * 0.95);
        int radiusH = static_cast<int>(face.boundingBox.height * 1.15);
        
        cv::ellipse(mask, center, cv::Size(radiusW, radiusH), 0, 0, 360, cv::Scalar(255), -1);
    }
    
    cv::GaussianBlur(mask, mask, cv::Size(31, 31), 15);
    
    return mask;
}

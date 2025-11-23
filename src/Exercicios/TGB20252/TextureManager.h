#ifndef TEXTURE_MANAGER_H
#define TEXTURE_MANAGER_H

#include <glad/glad.h>
#include <opencv2/opencv.hpp>

class TextureManager {
public:
    TextureManager();
    ~TextureManager();
    
    void createTexture(int width, int height);
    void updateTexture(const cv::Mat& image);
    void bind();
    void unbind();
    GLuint getTextureID() const;
    void cleanup();
    
private:
    GLuint textureID;
    int width;
    int height;
};

#endif

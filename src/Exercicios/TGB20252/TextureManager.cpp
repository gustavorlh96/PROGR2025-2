#include "TextureManager.h"

TextureManager::TextureManager() : textureID(0), width(0), height(0) {}

TextureManager::~TextureManager() {
    cleanup();
}

void TextureManager::createTexture(int w, int h) {
    width = w;
    height = h;
    
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
    }
    
    glGenTextures(1, &textureID);
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_S, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_WRAP_T, GL_CLAMP_TO_EDGE);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MIN_FILTER, GL_LINEAR);
    glTexParameteri(GL_TEXTURE_2D, GL_TEXTURE_MAG_FILTER, GL_LINEAR);
    
    glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, width, height, 0, GL_BGR, GL_UNSIGNED_BYTE, nullptr);
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureManager::updateTexture(const cv::Mat& image) {
    if (textureID == 0 || image.empty()) return;
    
    glBindTexture(GL_TEXTURE_2D, textureID);
    
    cv::Mat flipped;
    cv::flip(image, flipped, 0);
    
    if (flipped.channels() == 3) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGB, flipped.cols, flipped.rows, 0, GL_BGR, GL_UNSIGNED_BYTE, flipped.data);
    } else if (flipped.channels() == 4) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RGBA, flipped.cols, flipped.rows, 0, GL_BGRA, GL_UNSIGNED_BYTE, flipped.data);
    } else if (flipped.channels() == 1) {
        glTexImage2D(GL_TEXTURE_2D, 0, GL_RED, flipped.cols, flipped.rows, 0, GL_RED, GL_UNSIGNED_BYTE, flipped.data);
    }
    
    glBindTexture(GL_TEXTURE_2D, 0);
}

void TextureManager::bind() {
    glBindTexture(GL_TEXTURE_2D, textureID);
}

void TextureManager::unbind() {
    glBindTexture(GL_TEXTURE_2D, 0);
}

GLuint TextureManager::getTextureID() const {
    return textureID;
}

void TextureManager::cleanup() {
    if (textureID != 0) {
        glDeleteTextures(1, &textureID);
        textureID = 0;
    }
}

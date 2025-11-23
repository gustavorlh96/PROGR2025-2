#include "FilterManager.h"

FilterManager::FilterManager() : kernelSize(15), brightnessValue(50), contrastValue(1.5), enableR(true), enableG(true), enableB(true) {}

std::vector<FilterInfo> FilterManager::getAvailableFilters() const {
    return {
        {FilterType::PORTRAIT_BLUR, "Portrait", "Simula modo retrato com fundo desfocado e rosto nitido."},
        {FilterType::BILATERAL_FILTERING, "Bilateral Filtering", "Suaviza pele e fundo mantendo contornos definidos."},
        {FilterType::BOX_BLUR, "Box Blur", "Desfoca uniformemente a imagem."},
        {FilterType::MEDIAN_BLUR, "Median Blur", "Reduz ruído preservando bordas."},
        {FilterType::SHARPEN, "Sharpen", "Destaca bordas e realca detalhes finos."},
        {FilterType::LAPLACIAN, "Laplacian", "Realça áreas de transição rápida de intensidade."},
        {FilterType::SOBEL, "Sobel", "Detecta bordas horizontais e verticais."},
        {FilterType::CANNY, "Canny Edge", "Extrai bordas com alta precisão."},
        {FilterType::GRAYSCALE, "B&W", "Converte para tons de cinza equilibrados."},
        {FilterType::SEPIA, "Vintage", "Aplica tonalidade quente inspirada em filme antigo."},
        {FilterType::INVERT, "Negative", "Inverte as cores para um efeito experimental."},
        {FilterType::BRIGHTNESS, "Bright", "Eleva o brilho geral de maneira suave."},
        {FilterType::CONTRAST, "Contrast", "Amplifica contraste e profundidade."},
        {FilterType::EMBOSS, "Emboss", "Cria relevo simulando iluminação lateral."},
        {FilterType::RGB_CHANNELS, "RGB", "Liga ou desliga rapidamente cada canal de cor."},
        {FilterType::VHS, "VHS", "Simula fita analogica com bleed, scanlines e ruido."}
    };
}

std::string FilterManager::getFilterDescription(FilterType filter) const {
    for (const auto& info : getAvailableFilters()) {
        if (info.type == filter) {
            return info.description;
        }
    }
    return "No description available.";
}

void FilterManager::setKernelSize(int size) {
    kernelSize = (size % 2 == 0) ? size + 1 : size;
    if (kernelSize < 3) kernelSize = 3;
}

void FilterManager::setBrightnessValue(int value) {
    brightnessValue = value;
}

void FilterManager::setContrastValue(double value) {
    contrastValue = value;
}

void FilterManager::setFaceMask(const cv::Mat& mask) {
    faceMask = mask.clone();
}

cv::Mat FilterManager::applyFilter(const cv::Mat& input, FilterType filter, ChannelMode channel) {
    if (input.empty()) return input;
    
    cv::Mat result;
    
    switch (filter) {
        case FilterType::NONE:
            result = input.clone();
            break;
        case FilterType::BILATERAL_FILTERING:
            result = gaussianBlur(input);
            break;
        case FilterType::BOX_BLUR:
            result = boxBlur(input);
            break;
        case FilterType::MEDIAN_BLUR:
            result = medianBlur(input);
            break;
        case FilterType::PORTRAIT_BLUR:
            result = portraitBlur(input);
            break;
        case FilterType::SHARPEN:
            result = sharpen(input);
            break;
        case FilterType::LAPLACIAN:
            result = laplacian(input);
            break;
        case FilterType::SOBEL:
            result = sobel(input);
            break;
        case FilterType::CANNY:
            result = canny(input);
            break;
        case FilterType::GRAYSCALE:
            result = grayscale(input);
            break;
        case FilterType::SEPIA:
            result = sepia(input);
            break;
        case FilterType::INVERT:
            result = invert(input);
            break;
        case FilterType::BRIGHTNESS:
            result = brightness(input);
            break;
        case FilterType::CONTRAST:
            result = contrast(input);
            break;
        case FilterType::EMBOSS:
            result = emboss(input);
            break;
        case FilterType::RGB_CHANNELS:
            result = rgbChannels(input);
            break;
        case FilterType::VHS:
            result = vhs(input);
            break;
        default:
            result = input.clone();
    }
    
    return applyChannelMode(result, channel);
}

cv::Mat FilterManager::applyChannelMode(const cv::Mat& input, ChannelMode channel) {
    if (channel == ChannelMode::RGB || input.channels() == 1) {
        return input;
    }
    
    if (channel == ChannelMode::GRAYSCALE) {
        cv::Mat gray;
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
        return gray;
    }
    
    std::vector<cv::Mat> channels(3);
    cv::split(input, channels);
    
    cv::Mat result = input.clone();
    
    switch (channel) {
        case ChannelMode::RED:
            channels[1] = cv::Mat::zeros(input.size(), CV_8UC1);
            channels[2] = cv::Mat::zeros(input.size(), CV_8UC1);
            break;
        case ChannelMode::GREEN:
            channels[0] = cv::Mat::zeros(input.size(), CV_8UC1);
            channels[2] = cv::Mat::zeros(input.size(), CV_8UC1);
            break;
        case ChannelMode::BLUE:
            channels[0] = cv::Mat::zeros(input.size(), CV_8UC1);
            channels[1] = cv::Mat::zeros(input.size(), CV_8UC1);
            break;
        default:
            break;
    }
    
    cv::merge(channels, result);
    return result;
}

cv::Mat FilterManager::gaussianBlur(const cv::Mat& input) {
    cv::Mat result;
    cv::bilateralFilter(input, result, 15, 75.0, 15.0);
    return result;
}

cv::Mat FilterManager::boxBlur(const cv::Mat& input) {
    cv::Mat result;
    cv::blur(input, result, cv::Size(kernelSize, kernelSize));
    return result;
}

cv::Mat FilterManager::medianBlur(const cv::Mat& input) {
    cv::Mat result;
    cv::medianBlur(input, result, kernelSize);
    return result;
}

cv::Mat FilterManager::portraitBlur(const cv::Mat& input) {
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(71, 71), 25.0);
    cv::GaussianBlur(blurred, blurred, cv::Size(31, 31), 12.0);
    
    if (faceMask.empty() || faceMask.size() != input.size()) {
        return blurred;
    }
    
    cv::Mat mask3channel;
    if (faceMask.channels() == 1) {
        cv::cvtColor(faceMask, mask3channel, cv::COLOR_GRAY2BGR);
    } else {
        mask3channel = faceMask.clone();
    }
    
    cv::Mat maskFloat, inputFloat, blurredFloat;
    mask3channel.convertTo(maskFloat, CV_32F, 1.0/255.0);
    input.convertTo(inputFloat, CV_32F);
    blurred.convertTo(blurredFloat, CV_32F);
    
    cv::Mat result = inputFloat.mul(maskFloat) + blurredFloat.mul(cv::Scalar(1.0, 1.0, 1.0) - maskFloat);
    result.convertTo(result, CV_8U);
    
    return result;
}

cv::Mat FilterManager::sharpen(const cv::Mat& input) {
    cv::Mat blurred;
    cv::GaussianBlur(input, blurred, cv::Size(0, 0), 3);
    cv::Mat result;
    cv::addWeighted(input, 1.5, blurred, -0.5, 0, result);
    return result;
}

cv::Mat FilterManager::laplacian(const cv::Mat& input) {
    cv::Mat gray, result;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input;
    }
    cv::Laplacian(gray, result, CV_16S, 3);
    cv::convertScaleAbs(result, result);
    if (input.channels() == 3) {
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    }
    return result;
}

cv::Mat FilterManager::sobel(const cv::Mat& input) {
    cv::Mat gray, gradX, gradY, result;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input;
    }
    cv::Sobel(gray, gradX, CV_16S, 1, 0);
    cv::Sobel(gray, gradY, CV_16S, 0, 1);
    cv::convertScaleAbs(gradX, gradX);
    cv::convertScaleAbs(gradY, gradY);
    cv::addWeighted(gradX, 0.5, gradY, 0.5, 0, result);
    if (input.channels() == 3) {
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    }
    return result;
}

cv::Mat FilterManager::canny(const cv::Mat& input) {
    cv::Mat gray, result;
    if (input.channels() == 3) {
        cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    } else {
        gray = input;
    }
    cv::Canny(gray, result, 50, 150);
    if (input.channels() == 3) {
        cv::cvtColor(result, result, cv::COLOR_GRAY2BGR);
    }
    return result;
}

cv::Mat FilterManager::grayscale(const cv::Mat& input) {
    if (input.channels() == 1) {
        cv::Mat result;
        cv::cvtColor(input, result, cv::COLOR_GRAY2BGR);
        return result;
    }
    cv::Mat gray, result;
    cv::cvtColor(input, gray, cv::COLOR_BGR2GRAY);
    cv::cvtColor(gray, result, cv::COLOR_GRAY2BGR);
    return result;
}

cv::Mat FilterManager::sepia(const cv::Mat& input) {
    cv::Mat result = input.clone();
    cv::Mat kernel = (cv::Mat_<float>(3, 3) << 
        0.272, 0.534, 0.131,
        0.349, 0.686, 0.168,
        0.393, 0.769, 0.189);
    cv::transform(input, result, kernel);
    return result;
}

cv::Mat FilterManager::invert(const cv::Mat& input) {
    cv::Mat result;
    cv::bitwise_not(input, result);
    return result;
}

cv::Mat FilterManager::brightness(const cv::Mat& input) {
    cv::Mat result;
    input.convertTo(result, -1, 1, brightnessValue);
    return result;
}

cv::Mat FilterManager::contrast(const cv::Mat& input) {
    cv::Mat result;
    input.convertTo(result, -1, contrastValue, 0);
    return result;
}

cv::Mat FilterManager::emboss(const cv::Mat& input) {
    cv::Mat kernel = (cv::Mat_<float>(3, 3) << 
        -2, -1, 0,
        -1,  1, 1,
         0,  1, 2);
    cv::Mat result;
    cv::filter2D(input, result, input.depth(), kernel);
    result = result + 128;
    return result;
}

cv::Mat FilterManager::vhs(const cv::Mat& input) {
    cv::Mat floatInput;
    input.convertTo(floatInput, CV_32FC3, 1.0f / 255.0f);

    cv::Mat luma;
    cv::cvtColor(floatInput, luma, cv::COLOR_BGR2GRAY);
    cv::Mat smear;
    cv::boxFilter(luma, smear, -1, cv::Size(25, 1));
    cv::normalize(smear, smear, 0.0f, 1.0f, cv::NORM_MINMAX);
    cv::Mat smearChannels[] = {smear, smear, smear};
    cv::Mat smear3;
    cv::merge(smearChannels, 3, smear3);

    cv::Mat grad;
    cv::Sobel(luma, grad, CV_32F, 1, 0, 3);
    grad = cv::abs(grad);
    cv::GaussianBlur(grad, grad, cv::Size(5, 1), 0.0);
    cv::normalize(grad, grad, 0.0f, 1.0f, cv::NORM_MINMAX);

    cv::Mat shiftMat = (cv::Mat_<float>(2, 3) << 1, 0, 4, 0, 1, 0);
    cv::Mat gradShifted;
    cv::warpAffine(grad, gradShifted, shiftMat, grad.size(), cv::INTER_LINEAR, cv::BORDER_REFLECT);

    cv::Mat diffCombined = 0.65f * grad + 0.35f * gradShifted;
    cv::Mat diffChannels[] = {diffCombined, diffCombined, diffCombined};
    cv::Mat diff3;
    cv::merge(diffChannels, 3, diff3);

    cv::Mat smearBoost = smear3.mul(cv::Scalar(1.0f, 1.0f, 1.0f) + 0.45f * diff3);
    cv::Mat processed = 0.55f * floatInput + 0.45f * smearBoost;

    cv::Mat bleed;
    cv::GaussianBlur(processed, bleed, cv::Size(13, 1), 0.0);
    processed = 0.7f * processed + 0.3f * bleed;

    std::vector<cv::Mat> colorCh;
    cv::split(processed, colorCh);
    cv::Mat shiftRight = (cv::Mat_<float>(2, 3) << 1, 0, 2, 0, 1, 0);
    cv::Mat shiftLeft  = (cv::Mat_<float>(2, 3) << 1, 0, -2, 0, 1, 0);
    cv::Mat redShift, blueShift;
    cv::warpAffine(colorCh[2], redShift, shiftRight, colorCh[2].size(), cv::INTER_LINEAR, cv::BORDER_REFLECT);
    cv::warpAffine(colorCh[0], blueShift, shiftLeft,  colorCh[0].size(), cv::INTER_LINEAR, cv::BORDER_REFLECT);
    colorCh[2] = 0.85f * colorCh[2] + 0.15f * redShift;
    colorCh[0] = 0.85f * colorCh[0] + 0.15f * blueShift;
    cv::merge(colorCh, processed);

    cv::Mat ghostTransform = (cv::Mat_<float>(2, 3) << 1, 0, 6, 0, 1, 0);
    cv::Mat ghost;
    cv::warpAffine(floatInput, ghost, ghostTransform, floatInput.size(),
                   cv::INTER_LINEAR, cv::BORDER_REFLECT);
    processed = 0.85f * processed + 0.15f * ghost;

    cv::Mat noise(floatInput.size(), CV_32FC3);
    cv::randn(noise, 0.0, 0.02);
    processed += noise;

    std::vector<cv::Mat> aberrationChannels;
    cv::split(processed, aberrationChannels);
    int rows = processed.rows;
    int cols = processed.cols;
    float cx = cols * 0.5f;
    float cy = rows * 0.5f;
    cv::Mat mapXRed(rows, cols, CV_32FC1);
    cv::Mat mapYRed(rows, cols, CV_32FC1);
    cv::Mat mapXBlue(rows, cols, CV_32FC1);
    cv::Mat mapYBlue(rows, cols, CV_32FC1);
    const float aberrStrength = 4.0f;
    for (int y = 0; y < rows; ++y) {
        float normY = (y - cy) / rows;
        for (int x = 0; x < cols; ++x) {
            float normX = (x - cx) / cols;
            float offsetX = aberrStrength * normX;
            float offsetY = aberrStrength * normY;
            mapXRed.at<float>(y, x) = static_cast<float>(x) + offsetX;
            mapYRed.at<float>(y, x) = static_cast<float>(y) + offsetY;
            mapXBlue.at<float>(y, x) = static_cast<float>(x) - offsetX;
            mapYBlue.at<float>(y, x) = static_cast<float>(y) - offsetY;
        }
    }
    cv::Mat remappedRed, remappedBlue;
    cv::remap(aberrationChannels[2], remappedRed, mapXRed, mapYRed, cv::INTER_LINEAR, cv::BORDER_REFLECT);
    cv::remap(aberrationChannels[0], remappedBlue, mapXBlue, mapYBlue, cv::INTER_LINEAR, cv::BORDER_REFLECT);
    aberrationChannels[2] = 0.7f * aberrationChannels[2] + 0.3f * remappedRed;
    aberrationChannels[0] = 0.7f * aberrationChannels[0] + 0.3f * remappedBlue;
    cv::merge(aberrationChannels, processed);

    cv::Mat scanPattern(floatInput.rows, 1, CV_32FC1);
    for (int y = 0; y < floatInput.rows; ++y) {
        scanPattern.at<float>(y, 0) = (y % 2 == 0) ? 0.8f : 1.0f;
    }
    cv::Mat scanMask;
    cv::repeat(scanPattern, 1, floatInput.cols, scanMask);
    cv::Mat scanChannels[] = {scanMask, scanMask, scanMask};
    cv::Mat scanMask3;
    cv::merge(scanChannels, 3, scanMask3);
    processed = processed.mul(scanMask3);

    cv::Mat lab;
    cv::cvtColor(processed, lab, cv::COLOR_BGR2Lab);
    std::vector<cv::Mat> labChannels;
    cv::split(lab, labChannels);
    labChannels[1] *= 0.85f;
    labChannels[2] *= 0.85f;
    cv::merge(labChannels, lab);
    cv::cvtColor(lab, processed, cv::COLOR_Lab2BGR);

    cv::Mat clipped;
    cv::min(cv::max(processed, 0.0f), 1.0f, clipped);
    cv::Mat result8U;
    clipped.convertTo(result8U, CV_8UC3, 255.0f);
    return result8U;
}

void FilterManager::setRGBChannels(bool r, bool g, bool b) {
    enableR = r;
    enableG = g;
    enableB = b;
}

void FilterManager::getRGBChannels(bool& r, bool& g, bool& b) const {
    r = enableR;
    g = enableG;
    b = enableB;
}

cv::Mat FilterManager::rgbChannels(const cv::Mat& input) {
    if (input.channels() != 3) return input;
    
    std::vector<cv::Mat> channels(3);
    cv::split(input, channels);
    
    if (!enableB) channels[0] = cv::Mat::zeros(input.size(), CV_8UC1);
    if (!enableG) channels[1] = cv::Mat::zeros(input.size(), CV_8UC1);
    if (!enableR) channels[2] = cv::Mat::zeros(input.size(), CV_8UC1);
    
    cv::Mat result;
    cv::merge(channels, result);
    return result;
}

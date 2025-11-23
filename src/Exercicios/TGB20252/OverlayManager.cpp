#include "OverlayManager.h"

namespace {
struct OverlayAsset {
    OverlayType type;
    const char* label;
    const char* path;
};

const OverlayAsset kOverlayAssets[] = {
    {OverlayType::HLA_GLYPH, "HL Alyx Glyph [Color Burn]", "../assets/overlays/hla_overlay.png"},
    {OverlayType::HIPSTER, "2010 Hipster [Exclude]", "../assets/overlays/hipster_overlay.png"},
    {OverlayType::SUMMER, "Summertime [Linear Light]", "../assets/overlays/summer_overlay.png"}
};
}

void OverlayManager::load(int width, int height) {
    targetSize = cv::Size(width, height);
    entries.clear();
    textures.clear();

    for (const auto& asset : kOverlayAssets) {
        entries.push_back({asset.type, asset.label, asset.path});
        cv::Mat img = cv::imread(asset.path, cv::IMREAD_UNCHANGED);
        if (!img.empty() && (img.cols != width || img.rows != height)) {
            cv::resize(img, img, targetSize);
        }
        textures[asset.type] = img;
    }
}

const std::vector<OverlayOption>& OverlayManager::options() const {
    return entries;
}

cv::Mat OverlayManager::apply(const cv::Mat& base, OverlayType type) const {
    if (type == OverlayType::NONE || base.empty()) {
        return base;
    }

    auto it = textures.find(type);
    if (it == textures.end() || it->second.empty()) {
        return base;
    }

    cv::Mat overlay = it->second;
    cv::Mat overlayColor = overlay;
    cv::Mat alphaChannel;
    if (overlay.channels() == 4) {
        std::vector<cv::Mat> channels;
        cv::split(overlay, channels);
        std::vector<cv::Mat> bgr(channels.begin(), channels.begin() + 3);
        cv::merge(bgr, overlayColor);
        alphaChannel = channels[3];
    } else {
        alphaChannel = cv::Mat(overlay.rows, overlay.cols, CV_8UC1, cv::Scalar(255));
    }

    cv::Mat baseFloat;
    cv::Mat overlayFloat;
    cv::Mat alphaFloat;
    base.convertTo(baseFloat, CV_32FC3, 1.0f / 255.0f);
    overlayColor.convertTo(overlayFloat, CV_32FC3, 1.0f / 255.0f);
    alphaChannel.convertTo(alphaFloat, CV_32FC1, 1.0f / 255.0f);
    cv::Mat alphaFloat3;
    cv::Mat alphaChannels[] = {alphaFloat, alphaFloat, alphaFloat};
    cv::merge(alphaChannels, 3, alphaFloat3);

    cv::Mat blended;
    switch (type) {
        case OverlayType::HLA_GLYPH: {
            std::vector<cv::Mat> baseCh(3), overlayCh(3), burnCh(3);
            cv::split(baseFloat, baseCh);
            cv::split(overlayFloat, overlayCh);
            for (int i = 0; i < 3; ++i) {
                cv::Mat denom = overlayCh[i].clone();
                cv::Mat zeroMask;
                cv::compare(denom, 1e-4f, zeroMask, cv::CMP_LT);
                denom.setTo(1e-4f, zeroMask);
                cv::Mat invDenom;
                cv::divide(1.0f, denom, invDenom);
                cv::Mat channel = 1.0f - (1.0f - baseCh[i]).mul(invDenom);
                channel.setTo(0.0f, zeroMask);
                burnCh[i] = channel;
            }
            cv::merge(burnCh, blended);
            break;
        }
        case OverlayType::HIPSTER: {
            blended = baseFloat + overlayFloat - 2.0f * baseFloat.mul(overlayFloat);
            break;
        }
        case OverlayType::SUMMER: {
            std::vector<cv::Mat> baseCh(3), overlayCh(3), blendedCh(3);
            cv::split(baseFloat, baseCh);
            cv::split(overlayFloat, overlayCh);
            for (int i = 0; i < 3; ++i) {
                cv::Mat dodge = baseCh[i] + 2.0f * (overlayCh[i] - 0.5f);
                cv::min(dodge, 1.0f, dodge);
                cv::Mat burn = baseCh[i] + 2.0f * overlayCh[i] - 1.0f;
                cv::max(burn, 0.0f, burn);
                cv::Mat hiMask;
                cv::Mat loMask;
                cv::compare(overlayCh[i], 0.5f, hiMask, cv::CMP_GE);
                cv::compare(overlayCh[i], 0.5f, loMask, cv::CMP_LT);
                cv::Mat channelResult = cv::Mat::zeros(baseCh[i].size(), baseCh[i].type());
                dodge.copyTo(channelResult, hiMask);
                burn.copyTo(channelResult, loMask);
                blendedCh[i] = channelResult;
            }
            cv::merge(blendedCh, blended);
            break;
        }
        default:
            return base;
    }

    cv::max(blended, 0.0f, blended);
    cv::min(blended, 1.0f, blended);

    cv::Mat resultFloat = baseFloat.mul(1.0f - alphaFloat3) + blended.mul(alphaFloat3);
    cv::Mat result8U;
    resultFloat.convertTo(result8U, CV_8UC3, 255.0f);
    return result8U;
}

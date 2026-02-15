#include "99_Default/pch.h"
#include "SpriteSheet.h"

SpriteSheet::SpriteSheet(std::unique_ptr<Gdiplus::Bitmap> sheet, UINT fw, UINT fh, UINT fpr, UINT tf, bool flipHorizontal)
    : m_pSheetBitmap(std::move(sheet)),
    m_frameWidth(fw), m_frameHeight(fh), m_framesPerRow(fpr), m_totalFrames(tf), m_flipHorizontal(flipHorizontal) {}

std::unique_ptr<SpriteSheet> SpriteSheet::CreateFromFile(
    const std::wstring& imagePath,
    UINT frameWidth, UINT frameHeight,
    UINT framesPerRow, UINT totalFrames,
    bool flipHorizontal) {
    
    auto bitmap = std::make_unique<Gdiplus::Bitmap>(imagePath.c_str());
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        OutputDebugStringW((L"SpriteSheet: 파일 로드 실패 - " + imagePath + L"\n").c_str());
        return nullptr;
    }

    if (flipHorizontal) {
        UINT w = bitmap->GetWidth();
        UINT h = bitmap->GetHeight();
        auto flipped = std::make_unique<Gdiplus::Bitmap>(static_cast<INT>(w), static_cast<INT>(h));
        if (flipped && flipped->GetLastStatus() == Gdiplus::Ok) {
            Gdiplus::Graphics g(flipped.get());
            Gdiplus::RectF destRect(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
            g.DrawImage(bitmap.get(), destRect, static_cast<float>(w), 0.0f, -static_cast<float>(w), static_cast<float>(h), Gdiplus::UnitPixel);
            bitmap = std::move(flipped);
        }
    }
    
    return std::make_unique<SpriteSheet>(std::move(bitmap), frameWidth, frameHeight, framesPerRow, totalFrames, flipHorizontal);
}

SpriteSheet::~SpriteSheet() {} 

std::vector<AnimationFrame> SpriteSheet::ExtractFrames(float frameDuration, float pivotX, float pivotY) const {
    std::vector<AnimationFrame> frames;
    if (!m_pSheetBitmap.get()) {
        return frames;
    }

    for (UINT i = 0; i < m_totalFrames; ++i) {
        UINT row = i / m_framesPerRow;
        UINT col = i % m_framesPerRow;

        // flipHorizontal: 열 역순으로 소스 X 보정하여 프레임 순서 유지
        UINT colForX = m_flipHorizontal ? (m_framesPerRow - 1 - col) : col;
        float x = (float)(colForX * m_frameWidth);
        float y = (float)(row * m_frameHeight);

        Gdiplus::RectF sourceRect(x, y, (float)m_frameWidth, (float)m_frameHeight);

        AnimationFrame frame(
            sourceRect,
            frameDuration,
            m_frameWidth,
            m_frameHeight,
            pivotX,
            pivotY
        );
        frames.push_back(frame);
    }
    return frames;
}

#include "99_Default/pch.h"
#include "SpriteSheet.h"

SpriteSheet::SpriteSheet(Gdiplus::Bitmap* sheet, UINT fw, UINT fh, UINT fpr, UINT tf, bool flipHorizontal)
    : m_pSheetBitmap(sheet),
    m_frameWidth(fw), m_frameHeight(fh), m_framesPerRow(fpr), m_totalFrames(tf), m_flipHorizontal(flipHorizontal) {
    // nullptr 체크는 호출자가 해야 하지만, 안전을 위해
    if (!m_pSheetBitmap) {
        OutputDebugStringW(L"SpriteSheet: nullptr 비트맵 전달됨\n");
    }
}

std::unique_ptr<SpriteSheet> SpriteSheet::CreateFromFile(
    const std::wstring& imagePath,
    UINT frameWidth, UINT frameHeight,
    UINT framesPerRow, UINT totalFrames,
    bool flipHorizontal) {
    
    Gdiplus::Bitmap* bitmap = new Gdiplus::Bitmap(imagePath.c_str());
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        if (bitmap) delete bitmap;
        OutputDebugStringW((L"SpriteSheet: 파일 로드 실패 - " + imagePath + L"\n").c_str());
        return nullptr;
    }

    if (flipHorizontal) {
        UINT w = bitmap->GetWidth();
        UINT h = bitmap->GetHeight();
        Gdiplus::Bitmap* flipped = new Gdiplus::Bitmap(static_cast<INT>(w), static_cast<INT>(h));
        if (flipped && flipped->GetLastStatus() == Gdiplus::Ok) {
            Gdiplus::Graphics g(flipped);
            Gdiplus::RectF destRect(0.0f, 0.0f, static_cast<float>(w), static_cast<float>(h));
            g.DrawImage(bitmap, destRect, static_cast<float>(w), 0.0f, -static_cast<float>(w), static_cast<float>(h), Gdiplus::UnitPixel);
            delete bitmap;
            bitmap = flipped;
        } else {
            if (flipped) {
                delete flipped;
            }
            // flip 실패 시 원본 bitmap은 그대로 사용
        }
    }
    
    return std::make_unique<SpriteSheet>(bitmap, frameWidth, frameHeight, framesPerRow, totalFrames, flipHorizontal);
}

SpriteSheet::~SpriteSheet() {
    if (m_pSheetBitmap) {
        delete m_pSheetBitmap;
        m_pSheetBitmap = nullptr;
    }
}

Gdiplus::Bitmap* SpriteSheet::GetBitmap() const {
    return m_pSheetBitmap;
}

std::vector<AnimationFrame> SpriteSheet::ExtractFrames(float frameDuration, float pivotX, float pivotY) const {
    std::vector<AnimationFrame> frames;
    if (!m_pSheetBitmap) {
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

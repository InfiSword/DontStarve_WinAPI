#include "../99_Default/pch.h"
#include "SpriteSheet.h"

// 생성자 - 비트맵으로부터 SpriteSheet 생성
SpriteSheet::SpriteSheet(std::unique_ptr<Gdiplus::Bitmap> sheet, UINT fw, UINT fh, UINT fpr, UINT tf)
    : m_pSheetBitmap(std::move(sheet)),
    m_frameWidth(fw), m_frameHeight(fh), m_framesPerRow(fpr), m_totalFrames(tf) {}

// 생성자 - 파일로부터 SpriteSheet 생성
std::unique_ptr<SpriteSheet> SpriteSheet::CreateFromFile(
    const std::wstring& imagePath,
    UINT frameWidth, UINT frameHeight,
    UINT framesPerRow, UINT totalFrames) {
    
    auto bitmap = std::make_unique<Gdiplus::Bitmap>(imagePath.c_str());
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        OutputDebugStringW((L"SpriteSheet: 비트맵 로드 실패 - " + imagePath + L"\n").c_str());
        return nullptr;
    }
    
    return std::make_unique<SpriteSheet>(std::move(bitmap), frameWidth, frameHeight, framesPerRow, totalFrames);
}

SpriteSheet::~SpriteSheet() {} 

std::vector<AnimationFrame> SpriteSheet::ExtractFrames(float frameDuration, float pivotX, float pivotY) const {
    std::vector<AnimationFrame> frames;
    if (!m_pSheetBitmap.get()) {
        return frames;
    }

        for (UINT i = 0; i < m_totalFrames; ++i) {
            UINT row = i / m_framesPerRow; // 행 인덱스
            UINT col = i % m_framesPerRow; // 열 인덱스

            // 스프라이트 시트 프레임 좌표 계산
            float x = (float)(col * m_frameWidth);
            float y = (float)(row * m_frameHeight);

            // 소스 영역 계산
            Gdiplus::RectF sourceRect(x, y, (float)m_frameWidth, (float)m_frameHeight);

            // AnimationFrame 객체 생성, 프레임 너비와 높이 전달
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

// Animator/RenderManager에 전달
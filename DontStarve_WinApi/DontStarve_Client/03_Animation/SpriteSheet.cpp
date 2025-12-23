#include "../99_Default/pch.h"
#include "SpriteSheet.h"

// 생성자 (unique_ptr 버전)
SpriteSheet::SpriteSheet(std::unique_ptr<Gdiplus::Bitmap> sheet, UINT fw, UINT fh, UINT fpr, UINT tf)
    : m_pSheetBitmap(std::move(sheet)),
    m_frameWidth(fw), m_frameHeight(fh), m_framesPerRow(fpr), m_totalFrames(tf) {}

// 정적 팩토리 메소드 - 파일 경로로 직접 SpriteSheet 생성
std::unique_ptr<SpriteSheet> SpriteSheet::CreateFromFile(
    const std::wstring& imagePath,
    UINT frameWidth, UINT frameHeight,
    UINT framesPerRow, UINT totalFrames) {
    
    auto bitmap = std::make_unique<Gdiplus::Bitmap>(imagePath.c_str());
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        OutputDebugStringW((L"SpriteSheet: 이미지 로드 실패 - " + imagePath + L"\n").c_str());
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
        UINT row = i / m_framesPerRow; // 현재 프레임이 몇 번째 줄인지
        UINT col = i % m_framesPerRow; // 현재 프레임이 몇 번째 열인지

        // 스프라이트 시트 내에서 해당 프레임의 좌상단 픽셀 좌표
        float x = (float)(col * m_frameWidth);
        float y = (float)(row * m_frameHeight);

        // 스프라이트 시트 내의 소스 사각형 정보 생성
        Gdiplus::RectF sourceRect(x, y, (float)m_frameWidth, (float)m_frameHeight);

        // AnimationFrame 객체 생성 시, 씬에서 렌더링할 크기 의미로 해석
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

// 렌더링은 Animator/RenderManager에서 처리
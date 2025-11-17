#include "../99_Default/pch.h"
#include "SpriteSheet.h"

// 기존 생성자 (raw pointer 버전) - 하위 호환성을 위해 유지
SpriteSheet::SpriteSheet(Gdiplus::Bitmap* sheet, UINT fw, UINT fh, UINT fpr, UINT tf)
    : m_pSheetBitmap(sheet), // raw pointer를 unique_ptr로 래핑
    m_frameWidth(fw), m_frameHeight(fh), m_framesPerRow(fpr), m_totalFrames(tf) {}

// 새로운 생성자 (unique_ptr 버전)
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

// 이동 생성자 및 이동 대입 연산자
SpriteSheet::SpriteSheet(SpriteSheet&& other) noexcept = default;
SpriteSheet& SpriteSheet::operator=(SpriteSheet&& other) noexcept = default;

SpriteSheet::~SpriteSheet() {} 

Gdiplus::Bitmap* SpriteSheet::GetBitmap() const { return m_pSheetBitmap.get(); }
UINT SpriteSheet::GetFrameWidth() const { return m_frameWidth; }
UINT SpriteSheet::GetFrameHeight() const { return m_frameHeight; }
UINT SpriteSheet::GetFramesPerRow() const { return m_framesPerRow; }
UINT SpriteSheet::GetTotalFrames() const { return m_totalFrames; }

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

void SpriteSheet::DrawFrame(Gdiplus::Graphics* pGraphics, const AnimationFrame& frame, const Gdiplus::PointF& finalRenderTopLeftScreenPos, float zoomFactor) const {
    if (!pGraphics || !m_pSheetBitmap.get()) return;

    // 현재 프레임의 이미지를 최종적 크기 (줌 팩터 반영)
    float scaledWidth = frame.width * zoomFactor;
    float scaledHeight = frame.height * zoomFactor;

    // destRect는 finalRenderTopLeftScreenPos를 기준으로, scaledWidth/Height 크기를 가진다.
    Gdiplus::RectF destRect(finalRenderTopLeftScreenPos.X, finalRenderTopLeftScreenPos.Y, scaledWidth, scaledHeight);

    pGraphics->DrawImage(m_pSheetBitmap.get(), // 스프라이트 시트 자체 바이트맵
        destRect, // 화면에 그릴 영역
        frame.sourceRect.X, frame.sourceRect.Y, // 시트 내 소스 사각형 좌상단 X, Y
        frame.sourceRect.Width, frame.sourceRect.Height, // 시트 내 소스 사각형 너비, 높이
        Gdiplus::UnitPixel);
}
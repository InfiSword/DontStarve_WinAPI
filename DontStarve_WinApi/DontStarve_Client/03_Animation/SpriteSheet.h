#pragma once

class SpriteSheet {
public:
    // 기존 생성자 (raw pointer 받는 버전)
    SpriteSheet(Gdiplus::Bitmap* sheet, UINT fw, UINT fh, UINT fpr, UINT tf);
    
    // 새로운 생성자 (unique_ptr 받는 버전)
    SpriteSheet(std::unique_ptr<Gdiplus::Bitmap> sheet, UINT fw, UINT fh, UINT fpr, UINT tf);
    
    // 파일 경로로 직접 생성하는 정적 팩토리 메소드
    static std::unique_ptr<SpriteSheet> CreateFromFile(
        const std::wstring& imagePath,
        UINT frameWidth, UINT frameHeight,
        UINT framesPerRow, UINT totalFrames
    );
    
    // 이동 생성자 및 이동 대입 연산자
    SpriteSheet(SpriteSheet&& other) noexcept;
    SpriteSheet& operator=(SpriteSheet&& other) noexcept;
    
    // 복사 생성자 및 복사 대입 연산자 삭제
    SpriteSheet(const SpriteSheet&) = delete;
    SpriteSheet& operator=(const SpriteSheet&) = delete;
    
    ~SpriteSheet();

    // Getter 메소드들
    Gdiplus::Bitmap* GetBitmap() const;
    UINT GetFrameWidth() const;
    UINT GetFrameHeight() const;
    UINT GetFramesPerRow() const;
    UINT GetTotalFrames() const;

    // 프레임 추출 및 렌더링
    std::vector<AnimationFrame> ExtractFrames(float frameDuration = 0.1f, float pivotX = 0.5f, float pivotY = 1.0f) const;
    void DrawFrame(Gdiplus::Graphics* pGraphics, const AnimationFrame& frame, const Gdiplus::PointF& worldPos, float zoomFactor = 1.0f) const;

private:
    std::unique_ptr<Gdiplus::Bitmap> m_pSheetBitmap;
    UINT m_frameWidth;
    UINT m_frameHeight;
    UINT m_framesPerRow;
    UINT m_totalFrames;
}; 
#pragma once

class SpriteSheet {
public:
    // 파일 경로로 직접 생성하는 정적 팩토리 메소드에서 사용하는 생성자 (unique_ptr 버전)
    SpriteSheet(std::unique_ptr<Gdiplus::Bitmap> sheet, UINT fw, UINT fh, UINT fpr, UINT tf);
    
    // 파일 경로로 직접 생성하는 정적 팩토리 메소드
    static std::unique_ptr<SpriteSheet> CreateFromFile(
        const std::wstring& imagePath,
        UINT frameWidth, UINT frameHeight,
        UINT framesPerRow, UINT totalFrames
    );
    
    ~SpriteSheet();
    
    // 원본 비트맵 접근자 (렌더링용)
    Gdiplus::Bitmap* GetBitmap() const { return m_pSheetBitmap.get(); }
    
    // 프레임 추출
    std::vector<AnimationFrame> ExtractFrames(float frameDuration = 0.1f, float pivotX = 0.5f, float pivotY = 1.0f) const;

private:
    std::unique_ptr<Gdiplus::Bitmap> m_pSheetBitmap;
    UINT m_frameWidth;
    UINT m_frameHeight;
    UINT m_framesPerRow;
    UINT m_totalFrames;
}; 
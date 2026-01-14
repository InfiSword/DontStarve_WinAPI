#pragma once

class SpriteSheet {
public:
    // 생성자 - 비트맵으로부터 SpriteSheet 생성
    SpriteSheet(std::unique_ptr<Gdiplus::Bitmap> sheet, UINT fw, UINT fh, UINT fpr, UINT tf);
    
    // 생성자 - 파일로부터 SpriteSheet 생성
    static std::unique_ptr<SpriteSheet> CreateFromFile(
        const std::wstring& imagePath,
        UINT frameWidth, UINT frameHeight,
        UINT framesPerRow, UINT totalFrames
    );
    
    ~SpriteSheet();
    
	// 비트맵 접근자
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
#pragma once
#include <memory>
#include <vector>
#include <string>
#include <gdiplus.h>
#include <Struct.h>

// SpriteSheet 클래스: 스프라이트 시트 이미지를 관리하고 프레임 추출
class SpriteSheet {
public:
    // unique_ptr을 받는 생성자 (flipHorizontal: 비트맵이 좌우 반전된 경우 true → ExtractFrames에서 프레임 순서 보정)
    SpriteSheet(std::unique_ptr<Gdiplus::Bitmap> sheet, UINT fw, UINT fh, UINT fpr, UINT tf, bool flipHorizontal = false);
    
    // 파일로부터 SpriteSheet 생성하는 팩토리 메서드
    // flipHorizontal: true면 로드 시 비트맵을 좌우 반전해 저장 (렌더 시 Transform 불필요)
    static std::unique_ptr<SpriteSheet> CreateFromFile(
        const std::wstring& imagePath,
        UINT frameWidth, UINT frameHeight,
        UINT framesPerRow, UINT totalFrames,
        bool flipHorizontal = false
    );
    
    ~SpriteSheet();

    // 비트맵 포인터 반환 (읽기 전용)
    Gdiplus::Bitmap* GetBitmap() const { return m_pSheetBitmap.get(); }
    
    // 프레임 추출
    std::vector<AnimationFrame> ExtractFrames(float frameDuration = 0.1f, float pivotX = 0.5f, float pivotY = 1.0f) const;

private:
    std::unique_ptr<Gdiplus::Bitmap> m_pSheetBitmap;
    UINT m_frameWidth;
    UINT m_frameHeight;
    UINT m_framesPerRow;
    UINT m_totalFrames;
    bool m_flipHorizontal;  // true면 비트맵이 좌우 반전됨 → 소스 X를 열 역순으로 보정
};

#pragma once
#include <memory>
#include <vector>
#include <string>
#include <gdiplus.h>
#include <Struct.h>

// SpriteSheet 클래스: 스프라이트 시트 이미지를 관리하고 프레임 추출
class SpriteSheet {
public:
    // 비트맵 소유권을 넘겨받는 생성자 (호출 후 호출자가 delete 하면 안 됨. SpriteSheet 소멸 시 delete)
    // flipHorizontal: 비트맵이 좌우 반전된 경우 true → ExtractFrames에서 프레임 순서 보정
    SpriteSheet(Gdiplus::Bitmap* sheet, UINT fw, UINT fh, UINT fpr, UINT tf, bool flipHorizontal = false);
    
    // 파일로부터 SpriteSheet 생성하는 팩토리 메서드
    // flipHorizontal: true면 로드 시 비트맵을 좌우 반전해 저장 (렌더 시 Transform 불필요)
    static std::unique_ptr<SpriteSheet> CreateFromFile(
        const std::wstring& imagePath,
        UINT frameWidth, UINT frameHeight,
        UINT framesPerRow, UINT totalFrames,
        bool flipHorizontal = false
    );
    
    ~SpriteSheet();

    // 복사/이동 방지 
    SpriteSheet(const SpriteSheet&) = delete;
    SpriteSheet& operator=(const SpriteSheet&) = delete;
    SpriteSheet(SpriteSheet&&) = delete;
    SpriteSheet& operator=(SpriteSheet&&) = delete;

    // 비트맵 포인터 반환 
    Gdiplus::Bitmap* GetBitmap() const;
    
    // 프레임 추출
    std::vector<AnimationFrame> ExtractFrames(float frameDuration = 0.1f, float pivotX = 0.5f, float pivotY = 1.0f) const;

private:
    Gdiplus::Bitmap* m_pSheetBitmap;  // SpriteSheet가 소유. 소멸자에서 delete
    UINT m_frameWidth;
    UINT m_frameHeight;
    UINT m_framesPerRow;
    UINT m_totalFrames;
    bool m_flipHorizontal;  // true면 비트맵이 좌우 반전됨 → 소스 X를 열 역순으로 보정
};

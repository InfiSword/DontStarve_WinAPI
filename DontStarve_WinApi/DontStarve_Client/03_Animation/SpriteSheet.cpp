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

    // 이미지 실제 크기에 맞게 프레임 크기 자동 계산
    // - 기본적으로 전달받은 frameWidth/frameHeight를 사용하되
    // - 0이 넘어오면 이미지 크기와 framesPerRow/totalFrames를 기반으로 계산
    UINT sheetWidth = bitmap->GetWidth();
    UINT sheetHeight = bitmap->GetHeight();
    UINT rows = (framesPerRow == 0) ? 0 : (totalFrames + framesPerRow - 1) / framesPerRow;

    UINT finalFrameWidth = frameWidth;
    UINT finalFrameHeight = frameHeight;

    if (framesPerRow == 0 || totalFrames == 0 || sheetWidth == 0 || sheetHeight == 0) {
        // 잘못된 설정인 경우, 기존 파라미터를 그대로 사용
        finalFrameWidth = frameWidth;
        finalFrameHeight = frameHeight;
    }
    else {
        if (finalFrameWidth == 0) {
            finalFrameWidth = (rows > 0) ? sheetWidth / framesPerRow : sheetWidth;
        }
        if (finalFrameHeight == 0) {
            finalFrameHeight = (rows > 0) ? sheetHeight / rows : sheetHeight;
        }

        // 전달된 frameWidth/Height가 이미지 계산 값과 크게 다를 경우 디버그 로그 남김
        /*UINT expectedWidth = (rows > 0) ? sheetWidth / framesPerRow : 0;
        UINT expectedHeight = (rows > 0) ? sheetHeight / rows : 0;
        if (expectedWidth != 0 && expectedHeight != 0 &&
            (frameWidth != 0 || frameHeight != 0) &&
            (frameWidth != expectedWidth || frameHeight != expectedHeight)) 
		{
            OutputDebugStringW((L"SpriteSheet: 프레임 크기와 이미지가 일치하지 않습니다 - " + imagePath +
                L" (입력: " + std::to_wstring(frameWidth) + L"x" + std::to_wstring(frameHeight) +
                L", 자동계산: " + std::to_wstring(expectedWidth) + L"x" + std::to_wstring(expectedHeight) + L")\n").c_str());
        }*/
    }

    return std::make_unique<SpriteSheet>(bitmap, finalFrameWidth, finalFrameHeight, framesPerRow, totalFrames, flipHorizontal);
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

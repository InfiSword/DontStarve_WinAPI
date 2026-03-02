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

    // 이미지 실제 크기 정보
    UINT sheetWidth = bitmap->GetWidth();
    UINT sheetHeight = bitmap->GetHeight();
    
    // 최종 프레임 크기 (자동 계산 또는 전달받은 값 사용)
    UINT finalFrameWidth = frameWidth;
    UINT finalFrameHeight = frameHeight;

    // 자동 계산이 필요한 경우 (frameWidth 또는 frameHeight가 0)
    if ((finalFrameWidth == 0 || finalFrameHeight == 0) && 
        framesPerRow > 0 && totalFrames > 0 && 
        sheetWidth > 0 && sheetHeight > 0) 
    {
        // 행 개수 계산: 올림 나눗셈 (totalFrames를 framesPerRow로 나눈 값을 올림)
        // 예시: 33프레임 ÷ 4열 = (33 + 4 - 1) / 4 = 9행
        //       36프레임 ÷ 6열 = (36 + 6 - 1) / 6 = 6행
        UINT totalRows = (totalFrames + framesPerRow - 1) / framesPerRow;
        
        // 자동 프레임 크기 계산
        if (finalFrameWidth == 0) {
            finalFrameWidth = sheetWidth / framesPerRow;
        }
        if (finalFrameHeight == 0) {
            finalFrameHeight = sheetHeight / totalRows;
        }

        // 디버그: 계산된 프레임 크기 출력
        #ifdef _DEBUG
        OutputDebugStringW((L"SpriteSheet 자동 계산 - " + imagePath + 
            L"\n  이미지 크기: " + std::to_wstring(sheetWidth) + L"x" + std::to_wstring(sheetHeight) +
            L"\n  프레임 배치: " + std::to_wstring(framesPerRow) + L"열 x " + std::to_wstring(totalRows) + L"행 (총 " + std::to_wstring(totalFrames) + L"개)" +
            L"\n  프레임 크기: " + std::to_wstring(finalFrameWidth) + L"x" + std::to_wstring(finalFrameHeight) + L"\n").c_str());
        #endif
    }

    // 잘못된 설정 검증
    if (finalFrameWidth == 0 || finalFrameHeight == 0) {
        OutputDebugStringW((L"SpriteSheet: 프레임 크기를 계산할 수 없습니다 - " + imagePath + 
            L"\n  framesPerRow: " + std::to_wstring(framesPerRow) + 
            L", totalFrames: " + std::to_wstring(totalFrames) +
            L", sheetSize: " + std::to_wstring(sheetWidth) + L"x" + std::to_wstring(sheetHeight) + L"\n").c_str());
        delete bitmap;
        return nullptr;
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

#include "99_Default/pch.h"
#include "SpriteSheet.h"

// ������ (unique_ptr ����)
SpriteSheet::SpriteSheet(std::unique_ptr<Gdiplus::Bitmap> sheet, UINT fw, UINT fh, UINT fpr, UINT tf)
    : m_pSheetBitmap(std::move(sheet)),
    m_frameWidth(fw), m_frameHeight(fh), m_framesPerRow(fpr), m_totalFrames(tf) {}

// ���� ���丮 �޼ҵ� - ���� ��η� ���� SpriteSheet ����
std::unique_ptr<SpriteSheet> SpriteSheet::CreateFromFile(
    const std::wstring& imagePath,
    UINT frameWidth, UINT frameHeight,
    UINT framesPerRow, UINT totalFrames) {
    
    auto bitmap = std::make_unique<Gdiplus::Bitmap>(imagePath.c_str());
    if (!bitmap || bitmap->GetLastStatus() != Gdiplus::Ok) {
        OutputDebugStringW((L"SpriteSheet: 파일 로드 실패 - " + imagePath + L"\n").c_str());
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
        UINT row = i / m_framesPerRow; // ���� �������� �� ��° ������
        UINT col = i % m_framesPerRow; // ���� �������� �� ��° ������

        // ��������Ʈ ��Ʈ ������ �ش� �������� �»�� �ȼ� ��ǥ
        float x = (float)(col * m_frameWidth);
        float y = (float)(row * m_frameHeight);

        // ��������Ʈ ��Ʈ ���� �ҽ� �簢�� ���� ����
        Gdiplus::RectF sourceRect(x, y, (float)m_frameWidth, (float)m_frameHeight);

        // AnimationFrame ��ü ���� ��, ������ �������� ũ�� �ǹ̷� �ؼ�
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

// �������� Animator/RenderManager���� ó��

#include "../99_Default/pch.h"
#include "AnimationClip.h"

// ============== AnimationBuilder 구현 ==============

AnimationBuilder& AnimationBuilder::SetName(const std::wstring& name) {
    m_name = name;
    return *this;
}

AnimationBuilder& AnimationBuilder::SetImagePath(const std::wstring& imagePath) {
    m_imagePath = imagePath;
    return *this;
}

AnimationBuilder& AnimationBuilder::SetFrameSize(UINT width, UINT height) {
    m_frameWidth = width;
    m_frameHeight = height;
    return *this;
}

AnimationBuilder& AnimationBuilder::SetFrameCount(UINT framesPerRow, UINT totalFrames) {
    m_framesPerRow = framesPerRow;
    m_totalFrames = totalFrames;
    return *this;
}

AnimationBuilder& AnimationBuilder::SetFrameDuration(float duration) {
    m_frameDuration = duration;
    return *this;
}

AnimationBuilder& AnimationBuilder::SetPivot(float pivotX, float pivotY) {
    m_pivotX = pivotX;
    m_pivotY = pivotY;
    return *this;
}

AnimationBuilder& AnimationBuilder::SetLooping(bool loop) {
    m_isLooping = loop;
    return *this;
}

AnimationBuilder& AnimationBuilder::AddEvent(int frameIndex, const std::wstring& eventName) {
    m_events[frameIndex] = eventName;
    return *this;
}

std::unique_ptr<AnimationClip> AnimationBuilder::Build() {
    if (m_imagePath.empty() || m_frameWidth == 0 || m_frameHeight == 0) {
        OutputDebugStringW(L"AnimationBuilder: 이미지 경로나 프레임 크기가 설정되지 않았습니다.\n");
        return nullptr;
    }

    auto clip = AnimationClip::CreateFromFile(
        m_name, m_imagePath, 
        m_frameWidth, m_frameHeight,
        m_framesPerRow, m_totalFrames,
        m_frameDuration, m_pivotX, m_pivotY, m_isLooping
    );

    if (clip) {
        // 이벤트 추가
        for (const auto& eventPair : m_events) {
            clip->AddEventFrame(eventPair.first, eventPair.second);
        }
    }

    return clip;
}

// ============== AnimationClip 구현 ==============

AnimationClip::AnimationClip() : m_isLooping(true), m_totalDuration(0.0f) {}

AnimationClip::AnimationClip(const std::wstring& name, bool loop) 
    : m_name(name), m_isLooping(loop), m_totalDuration(0.0f) {}

AnimationClip::~AnimationClip() {}

std::unique_ptr<AnimationClip> AnimationClip::CreateFromFile(
    const std::wstring& name,
    const std::wstring& imagePath,
    UINT frameWidth, UINT frameHeight,
    UINT framesPerRow, UINT totalFrames,
    float frameDuration,
    float pivotX, float pivotY,
    bool loop) {
    
    // Bitmap 로드
    OutputDebugStringW((L"AnimationClip: 이미지 로드 시도 - " + imagePath + L"\n").c_str());
    std::unique_ptr<Gdiplus::Bitmap> bitmap = std::make_unique<Gdiplus::Bitmap>(imagePath.c_str());
    if (!bitmap) {
        OutputDebugStringW((L"AnimationClip: Bitmap 객체 생성 실패 - " + imagePath + L"\n").c_str());
        return nullptr;
    }
    
    Gdiplus::Status status = bitmap->GetLastStatus();
    if (status != Gdiplus::Ok) {
        std::wstring statusStr;
        switch (status) {
            case Gdiplus::Ok: statusStr = L"Ok"; break;
            case Gdiplus::GenericError: statusStr = L"GenericError"; break;
            case Gdiplus::InvalidParameter: statusStr = L"InvalidParameter"; break;
            case Gdiplus::OutOfMemory: statusStr = L"OutOfMemory"; break;
            case Gdiplus::ObjectBusy: statusStr = L"ObjectBusy"; break;
            case Gdiplus::InsufficientBuffer: statusStr = L"InsufficientBuffer"; break;
            case Gdiplus::NotImplemented: statusStr = L"NotImplemented"; break;
            case Gdiplus::Win32Error: statusStr = L"Win32Error"; break;
            case Gdiplus::WrongState: statusStr = L"WrongState"; break;
            case Gdiplus::Aborted: statusStr = L"Aborted"; break;
            case Gdiplus::FileNotFound: statusStr = L"FileNotFound"; break;
            case Gdiplus::ValueOverflow: statusStr = L"ValueOverflow"; break;
            case Gdiplus::AccessDenied: statusStr = L"AccessDenied"; break;
            case Gdiplus::UnknownImageFormat: statusStr = L"UnknownImageFormat"; break;
            case Gdiplus::FontFamilyNotFound: statusStr = L"FontFamilyNotFound"; break;
            case Gdiplus::FontStyleNotFound: statusStr = L"FontStyleNotFound"; break;
            case Gdiplus::NotTrueTypeFont: statusStr = L"NotTrueTypeFont"; break;
            case Gdiplus::UnsupportedGdiplusVersion: statusStr = L"UnsupportedGdiplusVersion"; break;
            case Gdiplus::GdiplusNotInitialized: statusStr = L"GdiplusNotInitialized"; break;
            case Gdiplus::PropertyNotFound: statusStr = L"PropertyNotFound"; break;
            case Gdiplus::PropertyNotSupported: statusStr = L"PropertyNotSupported"; break;
            default: statusStr = L"Unknown(" + std::to_wstring(status) + L")"; break;
        }
        OutputDebugStringW((L"AnimationClip: 이미지 로드 실패 - " + imagePath + L", 오류: " + statusStr + L"\n").c_str());
        return nullptr;
    }
    OutputDebugStringW((L"AnimationClip: 이미지 로드 성공 - " + imagePath + L"\n").c_str());

    // SpriteSheet 생성
    auto spriteSheet = std::make_unique<SpriteSheet>(std::move(bitmap), frameWidth, frameHeight, framesPerRow, totalFrames);
    
    // AnimationClip 생성
    auto clip = std::make_unique<AnimationClip>(name, loop);
    clip->SetSpriteSheet(std::move(spriteSheet));
    clip->InitializeFromSpriteSheet(frameDuration, pivotX, pivotY);
    
    return clip;
}

AnimationBuilder AnimationClip::Builder() {
    return AnimationBuilder();
}

void AnimationClip::SetSpriteSheet(std::unique_ptr<SpriteSheet> pSheet) {
    m_pSpriteSheet = std::move(pSheet);
}

const SpriteSheet* AnimationClip::GetSpriteSheet() const {
    return m_pSpriteSheet.get();
}

void AnimationClip::InitializeFromSpriteSheet(float frameDuration, float pivotX, float pivotY) {
    if (!m_pSpriteSheet) {
        return;
    }

    m_frames = m_pSpriteSheet->ExtractFrames(frameDuration, pivotX, pivotY);
    m_totalDuration = 0.0f;
    for (const auto& frame : m_frames) {
        m_totalDuration += frame.duration;
    }
}

void AnimationClip::AddFrame(const AnimationFrame& frame) {
    m_frames.push_back(frame);
    m_totalDuration += frame.duration;
}

void AnimationClip::SetLooping(bool loop) {
    m_isLooping = loop;
}

void AnimationClip::SetName(const std::wstring& name) {
    m_name = name;
}

void AnimationClip::SetTotalDuration(float duration) {
    m_totalDuration = duration;
}

const std::wstring& AnimationClip::GetName() const {
    return m_name;
}

bool AnimationClip::IsLooping() const {
    return m_isLooping;
}

float AnimationClip::GetTotalDuration() const {
    return m_totalDuration;
}

const std::vector<AnimationFrame>& AnimationClip::GetFrames() const {
    return m_frames;
}

const AnimationFrame& AnimationClip::GetCurrentFrame(float elapsed) const {
    if (m_frames.empty()) {
        static AnimationFrame dummyFrame;
        return dummyFrame;
    }

    float t;
    if (m_totalDuration <= 0.0f) {
        t = 0.0f;  // m_totalDuration이 0이면 t를 0으로 설정
    } else {
        t = m_isLooping ? fmod(elapsed, m_totalDuration) : min(elapsed, m_totalDuration);
    }
    
    float acc = 0.0f;
    for (const auto& frame : m_frames) {
        acc += frame.duration;
        if (t < acc) return frame;
    }
    return m_frames.back(); 
}

void AnimationClip::AddEventFrame(int frameIndex, const std::wstring& eventName) {
    if (frameIndex >= 0 && frameIndex < m_frames.size()) {
        m_eventFrames[frameIndex] = eventName;
    }
}

void AnimationClip::SetEventCallback(AnimationEventCallback callback) {
    m_eventCallback = callback;
}


#include "../99_Default/pch.h"
#include "AnimationClip.h"
#include "SpriteSheet.h"

// ============== AnimationClip 구현 ==============

AnimationClip::AnimationClip(
    const std::wstring& name,
    std::unique_ptr<SpriteSheet> spriteSheet,
    float frameDuration,
    float pivotX,
    float pivotY,
    bool loop) 
    : m_pSpriteSheet(std::move(spriteSheet)),
      m_isLooping(loop), 
      m_totalDuration(0.0f) {
    // 생성 시점에 SpriteSheet -> Frames 초기화까지 완료
    if (m_pSpriteSheet) {
        m_frames = m_pSpriteSheet->ExtractFrames(frameDuration, pivotX, pivotY);
        m_totalDuration = 0.0f;
        for (const auto& frame : m_frames) {
            m_totalDuration += frame.duration;
        }
    }
}

AnimationClip::~AnimationClip() {}

bool AnimationClip::IsLooping() const {
    return m_isLooping;
}

float AnimationClip::GetTotalDuration() const {
    return m_totalDuration;
}

const SpriteSheet* AnimationClip::GetSpriteSheet() const {
    return m_pSpriteSheet.get();
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
        t = 0.0f;  
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

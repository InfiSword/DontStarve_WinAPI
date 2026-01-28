#pragma once

#include "SpriteSheet.h" 

// 애니메이션 이벤트 콜백 타입 정의
typedef std::function<void(int, const std::wstring&)> AnimationEventCallback;

class AnimationClip {
public:
    // SpriteSheet를 받는 생성자
    AnimationClip(
        const std::wstring& name,
        std::unique_ptr<SpriteSheet> spriteSheet,
        float frameDuration = 0.1f,
        float pivotX = 0.5f,
        float pivotY = 1.0f,
        bool loop = true
    );

    ~AnimationClip();
    
    // 프레임 / 상태 조회 메서드
    bool IsLooping() const;
    float GetTotalDuration() const;
    const SpriteSheet* GetSpriteSheet() const;
    const std::vector<AnimationFrame>& GetFrames() const;
    const AnimationFrame& GetCurrentFrame(float elapsed) const;

    // 이벤트 관련 메서드
    void AddEventFrame(int frameIndex, const std::wstring& eventName);
    void SetEventCallback(AnimationEventCallback callback);
    const std::map<int, std::wstring>& GetEventFrames() const { return m_eventFrames; }
    const AnimationEventCallback& GetEventCallback() const { return m_eventCallback; }

private:    
    std::unique_ptr<SpriteSheet> m_pSpriteSheet;
    std::vector<AnimationFrame> m_frames;
    bool m_isLooping;
    float m_totalDuration;

    // 이벤트 관련 멤버
    std::map<int, std::wstring> m_eventFrames;
    AnimationEventCallback m_eventCallback;
};

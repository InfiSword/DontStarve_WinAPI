#pragma once
#include "SpriteSheet.h" 
#include <functional>
#include <memory>

// 애니메이션 이벤트 콜백 타입 정의
using AnimationEventCallback = std::function<void(int, const std::wstring&)>;

// 애니메이션 빌더 클래스 (더 편리한 생성을 위해)
class AnimationBuilder {
public:
    AnimationBuilder& SetName(const std::wstring& name);
    AnimationBuilder& SetImagePath(const std::wstring& imagePath);
    AnimationBuilder& SetFrameSize(UINT width, UINT height);
    AnimationBuilder& SetFrameCount(UINT framesPerRow, UINT totalFrames);
    AnimationBuilder& SetFrameDuration(float duration);
    AnimationBuilder& SetPivot(float pivotX, float pivotY);
    AnimationBuilder& SetLooping(bool loop);
    AnimationBuilder& AddEvent(int frameIndex, const std::wstring& eventName);
    
    // 편의 메소드들
    AnimationBuilder& AsIdle() { return SetLooping(true).SetFrameDuration(0.1f); }
    AnimationBuilder& AsWalk() { return SetLooping(true).SetFrameDuration(0.03f); }
    AnimationBuilder& AsAction() { return SetLooping(false).SetFrameDuration(0.05f); }
    
    std::unique_ptr<class AnimationClip> Build();

private:
    std::wstring m_name;
    std::wstring m_imagePath;
    UINT m_frameWidth = 0;
    UINT m_frameHeight = 0;
    UINT m_framesPerRow = 1;
    UINT m_totalFrames = 1;
    float m_frameDuration = 0.1f;
    float m_pivotX = 0.5f;
    float m_pivotY = 1.0f;
    bool m_isLooping = true;
    std::map<int, std::wstring> m_events;
};

class AnimationClip {
public:
    // 기본 생성자들
    AnimationClip();
    AnimationClip(const std::wstring& name, bool loop = true);
    
    // 직접 파일 경로로 애니메이션 생성하는 static factory methods
    static std::unique_ptr<AnimationClip> CreateFromFile(
        const std::wstring& name,
        const std::wstring& imagePath,
        UINT frameWidth, UINT frameHeight,
        UINT framesPerRow, UINT totalFrames,
        float frameDuration = 0.1f,
        float pivotX = 0.5f, float pivotY = 1.0f,
        bool loop = true
    );
    
    // 간단한 스프라이트 시트 애니메이션을 한 줄로 생성
    static std::unique_ptr<AnimationClip> CreateSimple(
        const std::wstring& imagePath,
        UINT frameWidth, UINT frameHeight,
        UINT totalFrames,
        bool loop = true
    ) {
        return CreateFromFile(L"", imagePath, frameWidth, frameHeight, totalFrames, totalFrames, 0.1f, 0.5f, 1.0f, loop);
    }
    
    // Builder 패턴을 위한 static method
    static AnimationBuilder Builder();

    ~AnimationClip();

    // 기존 메소드들
    void SetSpriteSheet(std::unique_ptr<SpriteSheet> pSheet);
    void AddFrame(const AnimationFrame& frame);
    void SetLooping(bool loop);
    void SetName(const std::wstring& name);
    void SetTotalDuration(float duration);
    const std::wstring& GetName() const;
    bool IsLooping() const;
    float GetTotalDuration() const;
    const SpriteSheet* GetSpriteSheet() const;
    const std::vector<AnimationFrame>& GetFrames() const;
    const AnimationFrame& GetCurrentFrame(float elapsed) const;

    // 이벤트 관련 메소드들
    void AddEventFrame(int frameIndex, const std::wstring& eventName);
    void SetEventCallback(AnimationEventCallback callback);
    const std::map<int, std::wstring>& GetEventFrames() const { return m_eventFrames; }
    const AnimationEventCallback& GetEventCallback() const { return m_eventCallback; }

private:    
    std::unique_ptr<SpriteSheet> m_pSpriteSheet;
    std::wstring m_name;
    std::vector<AnimationFrame> m_frames;
    bool m_isLooping;
    float m_totalDuration;

    // 이벤트 관련 멤버들
    std::map<int, std::wstring> m_eventFrames;
    AnimationEventCallback m_eventCallback;
    
    // 내부 초기화 메소드
    void InitializeFromSpriteSheet(float frameDuration, float pivotX, float pivotY);
};
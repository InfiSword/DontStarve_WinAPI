#pragma once
#include <map>
#include <memory>
#include "../02_GameObject/Component/Component.h"
#include "AnimationDefinition.h"

class GameObject;

// 애니메이션 파라미터 구조체
struct AnimationParameters {
    int state = 0;
    int direction = 0;
    bool trigger = false;
    
    AnimationParameters() = default;
    AnimationParameters(int s, int d) : state(s), direction(d) {}
};

// Unity Animator를 모방한 Animator 클래스 (Component 상속)
class Animator : public Component {
public:
    Animator(GameObject* owner);
    virtual ~Animator();

    // Unity Animator 스타일의 애니메이션 등록
    void RegisterAnimation(int state, Direction dir, 
                          const std::wstring& imagePath,
                          UINT frameWidth, UINT frameHeight,
                          UINT framesPerRow, UINT totalFrames,
                          float frameDuration = 0.1f,
                          float pivotX = 0.5f, float pivotY = 1.0f,
                          bool loop = true,
                          const std::map<int, std::wstring>& events = {});

    // 상태 설정 (Unity Animator 스타일)
    void SetState(int state, Direction direction);

    // 이벤트 콜백 설정
    void SetEventCallback(AnimationEventCallback callback) { m_globalEventCallback = callback; }

    // 컴포넌트 초기화 (Component 오버라이드) - 애니메이션 자동 등록
    virtual void Init() override;

    // 시간 업데이트 및 상태 업데이트 (Component 오버라이드)
    virtual void Update(float deltaTime) override;

    // 현재 프레임을 주어진 위치에 그리기
    void Draw(Gdiplus::Graphics* pGraphics, const Gdiplus::PointF& characterFootCenterScreenPos, 
              float zoomFactor, Direction currentDir, RenderLayer layer, float sortKey);

    // 현재 상태 정보 반환
    const AnimationFrame& GetCurrentFrame() const;
    bool IsPlaying() const;
    const SpriteSheet* GetSpriteSheet() const;
    bool IsAnimationDone() const;
    AnimationClip* GetClip() const { return m_currentClip; }
    float GetCurrentClipTotalDuration() const;
    int GetCurrentFrameIndex() const;

    // 재생 제어
    void Play();
    void Pause();
    void Stop();

private:
    // 애니메이션 저장소 - Unity Animator처럼 동작하기 위해 저장
    std::map<int, std::unique_ptr<AnimationClip>> m_animations; // key = state * 1000 + direction

    // 현재 상태
    AnimationClip* m_currentClip;
    int m_currentState;
    int m_currentDirection;
    float m_elapsed;
    bool m_isPlaying;
    int m_lastTriggeredFrame;

    // 전역 이벤트 콜백
    AnimationEventCallback m_globalEventCallback;

    // 내부 메서드
    int GetAnimationKey(int state, int direction) const { return state * 1000 + direction; }
    void SelectAndPlayAnimation();
    std::wstring GenerateAnimationName(int state, int direction) const;
};

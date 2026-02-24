#pragma once

#include <Enum.h>
#include <functional>
#include "../02_GameObject/Component/Component.h"
#include "AnimationClip.h"

class GameObject;

class Animator : public Component {
public:
    Animator(GameObject* owner);
    virtual ~Animator();

    void RegisterAnimation(int state, Direction dir, 
                          const std::wstring& imagePath,
                          UINT frameWidth, UINT frameHeight,
                          UINT framesPerRow, UINT totalFrames,
                          float pivotX = 0.5f, float pivotY = 1.0f,
                          bool loop = true,
                          float frameDuration = 0.03f,
                          bool flipHorizontal = false);  
	// false(기본값): LEFT 방향일 때만 자동 반전, true: 강제 반전 
    
    AnimationClip* GetAnimationClip(int state, Direction dir);

    void SetState(int state, Direction direction);

    virtual void Init() override;
    virtual void Update(float deltaTime) override;

    void Draw(Gdiplus::Graphics* pGraphics, const Gdiplus::PointF& characterFootCenterScreenPos, 
              float zoomFactor, Direction currentDir, RenderLayer layer, float sortKey);

    const AnimationFrame& GetCurrentFrame() const;
    const SpriteSheet* GetSpriteSheet() const;
    bool IsAnimationDone() const;
    AnimationClip* GetClip() const { return m_currentClip; }
    float GetCurrentClipTotalDuration() const;
    int GetCurrentFrameIndex() const;

    void Play();
    void Pause();
    void Stop();

private:
    std::map<int, std::unique_ptr<AnimationClip>> m_animations;

    AnimationClip* m_currentClip;
    int m_currentState;
    int m_currentDirection;
    float m_elapsed;
    bool m_isPlaying;
    int m_lastTriggeredFrame;

    int GetAnimationKey(int state, int direction) const { return state * 1000 + direction; }
    void SelectAndPlayAnimation();
};

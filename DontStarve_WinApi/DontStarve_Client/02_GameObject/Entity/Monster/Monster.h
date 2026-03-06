#pragma once
#include "../Entity.h"

class Monster : public Entity
{
protected:
    // 공통 AI 관련 변수
    GameObject* m_aggroTarget;
    float m_attackCooldownTimer;
    float m_walkSpeed;
    float m_runSpeed;
    float m_targetX;
    float m_targetY;

    // 플레이어와의 관계 정보 (Update에서 자동 계산)
    float m_distToPlayerSq; // 제곱 거리 (sqrt 연산 방지)
    Gdiplus::PointF m_dirToPlayer;

    // AI Tick 시스템 (성능 최적화)
    float m_aiTickTimer;
    float m_aiTickInterval;

public:
    Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, 
            const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Monster();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;

    // 자식 클래스에서 구현할 로직들
    virtual void UpdateAI(float deltaTime) = 0;       // 상태 결정 (0.1~0.2초마다 실행)
    virtual void UpdateMovement(float deltaTime) {}   // 실제 이동 (매 프레임 실행)

    // 편의 기능 (제곱 거리 비교)
    bool IsInAggroRangeSq(float range) const { return m_distToPlayerSq <= (range * range); }
};

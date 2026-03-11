#pragma once
#include "../Combatant.h"

class Monster : public Combatant
{
public:
    // 어그로 타입 정의
    enum class AggroType
    {
        ON_RANGE,               // 일정 범위 내 진입 시 추격 (거미, 하운드)
        ALWAYS,                 // 항상 추격 (보스, 특수 몬스터)
        ON_HIT_THEN_RANGE       // 처음 공격받으면 이후부터 범위 체크 시작 (피그)
    };

protected:
    // 공통 AI 관련 변수
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

    // 배회 및 어그로 관련 (공통)
    float m_wanderRadius;
    float m_aggroRadius;
    float m_deaggroRadius;

    // IDLE 상태 관련 (공통)
    float m_idleTimer;
    float m_idleDuration;

    // 어그로 설정
    AggroType m_aggroType;
    bool m_hasBeenHit;  // ON_HIT_THEN_RANGE 타입에서 피격 여부 추적

public:
    Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
            const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
    virtual ~Monster();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Damaged(int damage) override;

    // 어그로 설정 함수
    void SetupAggro(AggroType type, float aggroRadius = 300.0f, float deaggroRadius = 500.0f);

    // SetupAttackBox, UpdateAttackBoxByDirection, ProcessAttackHit는 Combatant에서 상속

protected:
    // AI 및 이동 (각 몬스터가 override)
    virtual void UpdateAI(float deltaTime);
    virtual void UpdateMovement(float deltaTime);
    virtual void OnAttackHit();
};

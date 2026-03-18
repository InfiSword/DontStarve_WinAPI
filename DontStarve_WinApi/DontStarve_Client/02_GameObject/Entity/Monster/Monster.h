#pragma once
#include "../Combatant.h"

class Monster : public Combatant
{
public:
    enum class AggroType
    {
        ON_RANGE,
        ALWAYS,
        ON_HIT_THEN_RANGE
    };

protected:
    float m_attackCooldownTimer;
    float m_walkSpeed;
    float m_runSpeed;
    float m_targetX;
    float m_targetY;
	float m_distToPlayerSq;
    
    float m_aiTickTimer;
    float m_aiTickInterval;
    float m_wanderRadius;
    float m_aggroRadius;
    float m_deaggroRadius;
    float m_idleTimer;
    float m_idleDuration;
    
	Gdiplus::PointF m_dirToPlayer;
	AggroType m_aggroType;

    bool m_hasBeenHit;
    bool m_bCanChase; 

    public:
    Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
            const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
    virtual ~Monster();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual void Damaged(int damage) override;
    virtual void ChangeState(int newState) override; 

    void SetupAggro(AggroType type, float aggroRadius = 300.0f, float deaggroRadius = 500.0f);

    void SetCanChase(bool canChase) { m_bCanChase = canChase; }
    bool CanChase() const { return m_bCanChase; }

protected:
    virtual void UpdateAI(float deltaTime);
    virtual void UpdateMovement(float deltaTime);
    virtual void OnAttackHit();
    virtual void OnAttackEnd();
    virtual void OnHitEnd();
    virtual void OnDeathEnd();

    void MoveTowardPlayer(float deltaTime, float speed, int runAnimState, int idleState);
    void MoveTowardLocation(float deltaTime, float speed, int walkAnimState, int idleState);
    void CheckAttackTransition(float range, int attackState, int idleState);
    void UpdateAI_AlwaysChase(float deltaTime, int runState, int attackState, int idleState);
    void UpdateAI_RangeChase(float deltaTime, int idleState, int walkState, int chaseState, int attackState, int tauntState = -1);
    void UpdateAI_Wander(float deltaTime, int walkState, int idleState);
};

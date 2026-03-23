#pragma once
#include "Monster.h"

class BoxCollider;

enum class PigState {
	IDLE = (int)CombatantState::IDLE,
	WALK = (int)CombatantState::WALK,
	RUN = (int)CombatantState::RUN,
	CHASE = (int)CombatantState::CHASE,
	ATTACK = (int)CombatantState::ATTACK,
	HIT = (int)CombatantState::HIT,
	DEATH = (int)CombatantState::DEATH,
	COUNT
};

class Pig : public Monster
{
public:
    Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
        const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
    virtual ~Pig() override;

    virtual void Init() override;
    virtual void UpdateAI(float deltaTime) override;
    virtual void UpdateMovement(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;
    virtual void Damaged(int damage) override;

    PigState GetPigState() const { return (PigState)m_state; }

    // 디버그 레이아웃 시각화
    virtual void RenderDebugOverlay() override;
    float GetActionRadius() const { return m_wanderRadius; }

protected:
    virtual void ChangeState(int newState) override;

    virtual void OnAttackHit() override;
    virtual void OnAttackEnd() override;
    virtual void OnHitEnd() override;
    virtual void Die() override; // override to set DEATH state and play animation
};

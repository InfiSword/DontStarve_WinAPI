#pragma once
#include "Monster.h"

enum class BossRedHoundState {
	IDLE = (int)CombatantState::IDLE,
	WALK = (int)CombatantState::WALK,
	CHASE = (int)CombatantState::CHASE,
	ATTACK = (int)CombatantState::ATTACK,
	ATTACK_PRE = (int)CombatantState::ATTACK_PRE,
	HIT = (int)CombatantState::HIT,
	DEATH = (int)CombatantState::DEATH,

	HOWL = (int)CombatantState::MAX_COMMON,
	COUNT
};

class Boss_RedHound : public Monster
{
public:
	Boss_RedHound(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
			 const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
	virtual ~Boss_RedHound() override;

	virtual void Init() override;
	virtual void RenderDebugOverlay() override;
	virtual void Damaged(int damage) override;
	virtual bool OnInteraction(GameObject* obj) override;

	// 슈퍼아머 훅
	virtual bool IsInAttackState() const override { return m_state == (int)BossRedHoundState::ATTACK || m_state == (int)BossRedHoundState::ATTACK_PRE; }
	virtual int GetHitState() const override { return (int)BossRedHoundState::HIT; }
	virtual void TriggerAttackState() override { ChangeState((int)BossRedHoundState::ATTACK_PRE); }

protected:
	virtual void UpdateAI(float deltaTime) override;
	virtual void UpdateMovement(float deltaTime) override;

	virtual int UpdateIdle(float deltaTime) override;
	virtual int UpdateWalk(float deltaTime) override;
	virtual int UpdateChase(float deltaTime) override;

	virtual void OnAttackHit() override;
	virtual void OnAttackEnd() override;
	virtual void OnHitEnd() override;
	virtual void Die() override;

private:
	bool m_bHasHowled;
};

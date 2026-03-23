#pragma once
#include "Monster.h"

enum class SpiderQueenState
{
	IDLE = (int)CombatantState::IDLE,
	CHASE = (int)CombatantState::CHASE,
	ATTACK = (int)CombatantState::ATTACK,
	HIT = (int)CombatantState::HIT,
	DEATH = (int)CombatantState::DEATH,

	BIRTH = (int)CombatantState::MAX_COMMON,
	TAUNT,
	COCOON,
	COCOON_HIT,
	COCOON_PRE,
	COUNT
};

class BoxCollider;

class Boss_SpiderQueen : public Monster
{
public:
	Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
		const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
	virtual ~Boss_SpiderQueen() override;

	virtual void Init() override;
	virtual void UpdateAI(float deltaTime) override;
	virtual void UpdateMovement(float deltaTime) override;
	virtual bool OnInteraction(GameObject* obj) override;

	virtual void Damaged(int damage) override;

	// 디버그 레이아웃 시각화
	virtual void RenderDebugOverlay() override;

	// 슈퍼아머 훅
	virtual bool IsInAttackState() const override { return m_state == (int)SpiderQueenState::ATTACK; }
	virtual int GetHitState() const override { return (int)SpiderQueenState::HIT; }
	virtual void TriggerAttackState() override { ChangeState((int)SpiderQueenState::ATTACK); }

protected:
	virtual void OnAttackHit() override;
	virtual void OnAttackEnd() override;
	virtual void OnHitEnd() override;

	// 애니메이션 이벤트 콜백
	void OnCocoonPreEnd();
	void OnBirthEnd();

private:
	void StartCocoonPhase();
	void EndCocoonPhase();
	void SummonSpider();

private:
	int m_bossPhase;
	float m_specialAttackCooldown;

	float m_idleTimer;
	float m_idleDuration;

	BoxCollider* m_attackCollider;

	// 회복 및 고치 로직 관련
	bool m_hasTriggeredCocoon;
	float m_cocoonTimer;
	float m_healTickTimer;

	// FX 관련
	bool m_isHealing;
	Animator* m_healFxAnimator;      // 회복 중 루프 FX
	Animator* m_spawnOutFxAnimator;  // 고치 탈출 시 단발 FX

	float m_spawnOnHitCooldown;
};

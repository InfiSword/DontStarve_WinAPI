#pragma once
#include "Monster.h"

enum class SpiderQueenState
{
	IDLE,
	CHASE,
	ATTACK,
	HIT,
	DEATH,
	BIRTH,
	TAUNT,
	COCOON,
	COCOON_HIT,
	COUNT
};

class BoxCollider;

class Boss_SpiderQueen : public Monster
{
public:
	Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
		const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
	virtual ~Boss_SpiderQueen();

	virtual void Init() override;
	virtual void UpdateAI(float deltaTime) override;
	virtual void UpdateMovement(float deltaTime) override;
	virtual bool OnInteraction(GameObject* obj) override;

	virtual void Damaged(int damage) override;

	// 디버그 레이아웃 시각화
	virtual void RenderDebugOverlay() override;

protected:
	virtual void OnAttackHit() override;
	virtual void OnAttackEnd() override;
	virtual void OnHitEnd() override;

private:
	void StartCocoonPhase();
	void EndCocoonPhase();
	void SpawnSpider();

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

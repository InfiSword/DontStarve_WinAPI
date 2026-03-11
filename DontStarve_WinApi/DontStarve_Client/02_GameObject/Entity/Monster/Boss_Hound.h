#pragma once
#include "Monster.h"

enum class BossHoundState {
	IDLE,
	RUN,
	ATTACK,
	HIT,
	DEATH,
	COUNT
};

class Boss_Hound : public Monster
{
public:
	Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
			 const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
	virtual ~Boss_Hound();

	virtual void Init() override;
	virtual void UpdateAI(float deltaTime) override;
	virtual void UpdateMovement(float deltaTime) override;
	virtual void Damaged(int damage) override;
	virtual bool OnInteraction(GameObject* obj) override;

protected:
	virtual void OnAttackHit() override;
	virtual void OnAttackEnd() override;
	virtual void Die() override; // ensure death state set when HP<=0

	// No additional members needed - all are inherited from Monster and Combatant
};

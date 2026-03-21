#pragma once
#include "Monster.h"

enum class HoundState {
	IDLE,
	RUN,
	ATTACK_PRE,
	ATTACK,
	HIT,
	DEATH,
	HOWL,
	CHASE,
	COUNT
};

class Hound : public Monster
{
public:
    Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
          const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
    virtual ~Hound() override;

    virtual void Init() override;
    virtual void UpdateAI(float deltaTime) override;
    virtual void UpdateMovement(float deltaTime) override;
    virtual void Damaged(int damage) override;
    virtual bool OnInteraction(GameObject* obj) override;

    // 디버그 레이아웃 시각화
    virtual void RenderDebugOverlay() override;

protected:
    virtual void OnAttackHit() override;
    virtual void OnAttackEnd() override;
    virtual void OnHitEnd() override;
    virtual void Die() override;

private:
    bool m_bHasHowled;
};

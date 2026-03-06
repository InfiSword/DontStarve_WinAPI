#pragma once
#include "Monster.h"

class BoxCollider;

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
    Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
    virtual ~Hound();

    virtual void Init() override;
    virtual void UpdateAI(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;
    virtual void Damaged(int damage) override;

	HoundState GetHoundState() const { return (HoundState)m_state; }

protected:
    virtual void OnAttackHit() override;
    virtual void OnAttackEnd() override;

private:
    static const float ATTACK_RANGE;
    static const float ATTACK_COOLDOWN;
};

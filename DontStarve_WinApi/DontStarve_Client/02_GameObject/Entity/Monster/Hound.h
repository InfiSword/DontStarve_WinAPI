#pragma once
#include "../Entity.h"

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

class Hound : public Entity
{
public:
    Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Hound();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;
    virtual void Damaged(int damage) override;

	HoundState GetHoundState() const { return (HoundState)m_state; }

private:
    static const float ATTACK_RANGE;
    static const float ATTACK_COOLDOWN;

    float m_wanderRadius;
    float m_aggroRadius;
    float m_deaggroRadius;
    float m_walkSpeed;
    float m_runSpeed;

    float m_attackCooldownTimer;
    float m_idleTimer;
    float m_idleDuration;

    float m_targetX;
    float m_targetY;

    GameObject* m_aggroTarget;
    BoxCollider* m_attackCollider;
};

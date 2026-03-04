#pragma once
#include "../Entity.h"

class BoxCollider;

enum class PigState {
	IDLE,
	WALK,
	RUN,
	CHASE,
	ATTACK,
	HIT,
	DEATH,
	COUNT
};

class Pig : public Entity
{
public:
    Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Pig();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;
    virtual void Damaged(int damage) override;

	void OnAttackHit();
	void OnAttackEnd();	

	PigState GetPigState() const { return (PigState)m_state; }

	// 디버그 레이아웃 시각화
	virtual void RenderDebugOverlay() override;
	float GetActionRadius() const { return m_wanderRadius; }

private:
    static const float ATTACK_RANGE;
    static const float ATTACK_COOLDOWN;

    float m_wanderRadius;
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

#pragma once
#include "Monster.h"

class BoxCollider;

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
    Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Boss_Hound();

    virtual void Init() override;
    virtual void UpdateAI(float deltaTime) override;
    virtual void UpdateMovement(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;
    virtual void Damaged(int damage) override;

	BossHoundState GetBossHoundState() const { return (BossHoundState)m_state; }

private:
    static const float ATTACK_RANGE;
    static const float ATTACK_COOLDOWN;

    float m_wanderRadius;
    float m_aggroRadius;
    float m_deaggroRadius;

    float m_idleTimer;
    float m_idleDuration;

    BoxCollider* m_attackCollider;
};

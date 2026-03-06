#pragma once
#include "Monster.h"

class ResourceManager;

enum class SpiderQueenState
{
    IDLE,
	CHASE,
    ATTACK,
    HIT,
    DEATH,
    TAUNT,
    BIRTH, 
    COUNT
};

class BoxCollider;

class Boss_SpiderQueen : public Monster
{
public:
    Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Boss_SpiderQueen();

    virtual void Init() override;
	virtual void UpdateAI(float deltaTime) override;
	virtual void UpdateMovement(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;

private:
	void OnAttackHit();
	void OnAttackEnd();

    int m_bossPhase;
    float m_specialAttackCooldown;

	static const float ATTACK_RANGE;
	static const float ATTACK_COOLDOWN;

	float m_idleTimer;
	float m_idleDuration;

	BoxCollider* m_attackCollider;
}; 

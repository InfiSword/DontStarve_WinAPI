#pragma once
#include "../Entity.h"

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

class Boss_SpiderQueen : public Entity
{
public:
    Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Boss_SpiderQueen();

	virtual void Update(float deltaTime) override;
    virtual void Init() override;
    virtual bool OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;

private:
	void OnAttackHit();
	void OnAttackEnd();

    int m_bossPhase;
    float m_specialAttackCooldown;

	static const float ATTACK_RANGE;
	static const float ATTACK_COOLDOWN;

	float m_walkSpeed;

	float m_attackCooldownTimer;
	float m_idleTimer;
	float m_idleDuration;

	float m_targetX;
	float m_targetY;

	GameObject* m_aggroTarget;
	BoxCollider* m_attackCollider;
}; 

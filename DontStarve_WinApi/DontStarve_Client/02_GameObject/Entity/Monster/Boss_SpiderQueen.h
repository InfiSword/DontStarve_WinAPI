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
    Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
                     const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
    virtual ~Boss_SpiderQueen();

    virtual void Init() override;
	virtual void UpdateAI(float deltaTime) override;
	virtual void UpdateMovement(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;

protected:
	virtual void OnAttackHit() override;
	virtual void OnAttackEnd() override;

private:
    int m_bossPhase;
    float m_specialAttackCooldown;

	float m_idleTimer;
	float m_idleDuration;

	BoxCollider* m_attackCollider;
}; 

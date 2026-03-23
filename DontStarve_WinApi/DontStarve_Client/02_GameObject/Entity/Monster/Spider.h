#pragma once
#include "Monster.h"

class ResourceManager;
class BoxCollider;
class SpiderEgg;

enum class SpiderState {
	IDLE = (int)CombatantState::IDLE,
	WALK = (int)CombatantState::WALK,
	CHASE = (int)CombatantState::CHASE,
	ATTACK = (int)CombatantState::ATTACK,
	HIT = (int)CombatantState::HIT,
	DEATH = (int)CombatantState::DEATH,

	TAUNT = (int)CombatantState::MAX_COMMON,
	COUNT
};

class Spider : public Monster
{
public:
    Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
           const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
	virtual ~Spider() override;

    virtual void Init() override;
    virtual void UpdateAI(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;    
	virtual void UpdateMovement(float deltaTime) override;
    virtual void Damaged(int damage) override;

    // 어그로 설정
    void SetAggroTarget(GameObject* target);

    // SpiderEgg 설정 (거미집 주변을 배회하도록)
    void SetHomeEgg(SpiderEgg* egg, float spawnRadius);
	bool HasHomeEgg() const { return m_homeEgg != nullptr; }

	SpiderState GetSpiderState() const { return (SpiderState)m_state; }

	// 디버그 레이아웃 시각화
	virtual void RenderDebugOverlay() override;

protected:
	virtual void OnAttackHit() override;
	virtual void OnAttackEnd() override;
	virtual void OnHitEnd() override;
	virtual void Die() override; // ensure death state set when HP<=0
    virtual void ResetAggroSession() override;
    virtual void ResolveWanderCenter(float& outX, float& outY) const override;

private:
    SpiderEgg* m_homeEgg;                // 소속 거미집
    float m_spawnRadius;                 // 거미집 주변 배회 반경
	bool m_bHasTaunted;                  // 현재 어그로 세션에서 도발 수행 여부
};

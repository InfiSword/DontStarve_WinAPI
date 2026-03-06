#pragma once
#include "Monster.h"

class ResourceManager;
class BoxCollider;
class SpiderEgg;

enum class SpiderState {
	IDLE,
	WALK,
	CHASE,
	ATTACK,
	HIT,
	DEATH,
	TAUNT,
	COUNT
};

class Spider : public Monster
{
public:
    Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"", ColliderType colliderType = COLLIDER_BOX);
    virtual ~Spider();

    virtual void Init() override;
    virtual void UpdateAI(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;    
    virtual void Damaged(int damage) override;

    // 어그로 설정
    void SetAggroTarget(GameObject* target);

    // SpiderEgg 설정 (거미집 주변을 배회하도록)
    void SetHomeEgg(SpiderEgg* egg, float spawnRadius);
    
    float GetAggroRadius() const { return m_aggroRadius; }
    float GetDeaggroRadius() const { return m_deaggroRadius; }

	SpiderState GetSpiderState() const { return (SpiderState)m_state; }

	// 디버그 레이아웃 시각화
	virtual void RenderDebugOverlay() override;

protected:
	virtual void OnAttackHit() override;
	virtual void OnAttackEnd() override;

private:
    static const float ATTACK_RANGE;      // 공격 사거리
    static const float ATTACK_COOLDOWN;   // 공격 쿨타임

private:
    SpiderEgg* m_homeEgg;                // 소속 거미집
    float m_spawnRadius;                 // 거미집 주변 배회 반경
};

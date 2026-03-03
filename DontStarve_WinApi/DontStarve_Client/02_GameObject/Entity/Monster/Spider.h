#pragma once
#include "../Entity.h"

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

class Spider : public Entity
{
public:
    Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Spider();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
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

private:
	void OnAttackHit();
	void OnAttackEnd();

private:
    static const float ATTACK_RANGE;      // 공격 사거리
    static const float ATTACK_COOLDOWN;   // 공격 쿨타임

private:
    SpiderEgg* m_homeEgg;                // 소속 거미집
    float m_spawnRadius;                 // 거미집 주변 배회 반경
    float m_aggroRadius;                 // 어그로 감지 범위
    float m_deaggroRadius;               // 어그로 해제 범위
    float m_walkSpeed;                   // 배회 속도
    float m_runSpeed;                    // 추격 속도
    float m_attackCooldownTimer;         // 공격 쿨타임 타이머
    float m_idleTimer;                   // IDLE 경과 시간
    float m_idleDuration;                // IDLE 유지 시간
    float m_targetX;                     // 이동 목표 X
    float m_targetY;                     // 이동 목표 Y
    GameObject* m_aggroTarget;           // 추격 대상
    BoxCollider* m_attackCollider;       // 공격 판정용
};

#pragma once
#include "Monster.h"

class ResourceManager;
class BoxCollider;

class Pig : public Monster
{
public:
    Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Pig();

    virtual void Init() override;
    virtual void Update(float deltaTime) override;
    virtual bool OnInteraction(GameObject* obj) override;
    virtual void Damaged(int damage) override;

    float GetActionRadius() const { return m_actionRadius; }

    virtual void RenderDebugOverlay() override;

private:
    static const float ATTACK_RANGE;  // 플레이어와 이 거리 이내면 MONSTER_ATTACK 진입

private:
    float m_actionRadius;   // 행동 반경 (픽셀)
    float m_targetX;       // 현재 이동 목표 월드 X
    float m_targetY;       // 현재 이동 목표 월드 Y
    float m_idleTimer;     // IDLE 경과 시간
    float m_idleDuration;  // 이번 IDLE 유지 시간 (2~5초 랜덤)
    float m_walkSpeed;     // 초당 이동량 (픽셀)
    float m_runSpeed;     // CHASE 시 이동 속도 (플레이어 추적)
    class GameObject* m_aggroTarget;  // 피격 시 추적 대상 (플레이어)
    BoxCollider* m_attackCollider;    // 공격 판정용 (MONSTER_ATTACK 특정 프레임에만 활성)
};

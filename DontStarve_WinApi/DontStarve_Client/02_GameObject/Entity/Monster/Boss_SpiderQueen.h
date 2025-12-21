#pragma once
#include "Monster.h"

class Boss_SpiderQueen : public Monster
{
public:
    Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Boss_SpiderQueen();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;
    
    // 애니메이션 정의 제공 (Entity 오버라이드)
    virtual std::vector<AnimationDefinition> GetAnimationDefinitions() const override;

private:
    int m_bossPhase;
    float m_specialAttackCooldown;
}; 
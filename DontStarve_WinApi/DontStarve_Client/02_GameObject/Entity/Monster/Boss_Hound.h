#pragma once
#include "Monster.h"

class Boss_Hound : public Monster
{
public:
    Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Boss_Hound();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;
    
    // 애니메이션 정의 제공 (Entity 오버라이드)
    virtual std::vector<AnimationDefinition> GetAnimationDefinitions() const override;

private:
    // Boss Phase
    int m_bossPhase;
    float m_specialAttackCooldown;
    std::wstring m_houndType; 
}; 
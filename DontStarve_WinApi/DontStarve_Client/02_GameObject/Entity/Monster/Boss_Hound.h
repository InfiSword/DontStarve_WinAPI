#pragma once
#include "Monster.h"

class ResourceManager;

class Boss_Hound : public Monster
{
public:
    Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName = L"");
    virtual ~Boss_Hound();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;

private:
    // Boss Phase
    int m_bossPhase;
    float m_specialAttackCooldown;
    std::wstring m_houndType; 
}; 

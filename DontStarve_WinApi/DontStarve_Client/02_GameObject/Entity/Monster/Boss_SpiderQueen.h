#pragma once
#include "Monster.h"

class ResourceManager;

class Boss_SpiderQueen : public Monster
{
public:
	static void RegisterResources(ResourceManager* rm);

    Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName = L"");
    virtual ~Boss_SpiderQueen();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;

private:
    int m_bossPhase;
    float m_specialAttackCooldown;
}; 

#pragma once
#include "../Entity.h"

class Monster : public Entity
{
public:
    Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Monster();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;
    virtual void OnInteraction(GameObject* obj) override;

    virtual void Damaged(int damage) override;

protected:
    int m_hp;
    int maxHp;
    float m_hitAnimTimer;
    MonsterState m_state;
    
}; 

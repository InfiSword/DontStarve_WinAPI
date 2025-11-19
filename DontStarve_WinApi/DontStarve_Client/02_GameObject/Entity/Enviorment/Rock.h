#pragma once
#include "../Entity.h"

class Rock : public Entity<RockState>
{
public:
    Rock(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Rock();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual void OnInteraction(GameObject* obj) override;

    // Entity 인터페이스 구현 (필수 오버라이드)
    virtual void RegisterAllAnimations() override {}
    virtual void UpdateAnimatorState() override {}

    virtual void Damaged(int damage) override;

private:
    int m_hp;
    int maxHp;
    float m_hitAnimTimer;
    
}; 

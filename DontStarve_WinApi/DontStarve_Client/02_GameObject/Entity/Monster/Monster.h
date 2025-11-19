#pragma once
#include "Entity.h"

class Monster : public Entity<MonsterState>
{
public:
    Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Monster();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Render(Gdiplus::Graphics* pGraphics) override; 
    virtual void Release() override;

    virtual Gdiplus::Bitmap* GetBitmap() const override;

    virtual void OnInteraction(GameObject* obj) override;

    // Entity 인터페이스 구현 (Unity Animator 스타일)
    virtual void RegisterAllAnimations() override;
    virtual void UpdateAnimatorState() override;

    virtual void Damaged(int damage) override;

    void OnPlayerInteraction(class Player* player);

private:
    int m_hp;
    int maxHp;
    float m_hitAnimTimer;
    
}; 
#pragma once
#include "Entity.h"

class Spider : public Entity<MonsterState>
{
public:
    Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Spider();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual Gdiplus::Bitmap* GetBitmap() const override;
    virtual void OnInteraction(GameObject* obj) override;

    // Entity 추상메소드 구현 (Unity Animator 스타일)
    virtual void RegisterAllAnimations() override;
    virtual void UpdateAnimatorState() override;

    virtual void Damaged(int damage) override;
    void OnPlayerInteraction(class Player* player);

protected:
    int m_hp;
    int maxHp;
    float m_hitAnimTimer;
}; 
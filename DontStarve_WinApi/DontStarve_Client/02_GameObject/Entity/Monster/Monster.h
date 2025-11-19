#pragma once
#include "../Entity.h"

class Monster : public Entity<MonsterState>
{
public:
    Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Monster();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    virtual Gdiplus::Bitmap* GetBitmap() const override;

    virtual void OnInteraction(GameObject* obj) override;

    // 각 자식 클래스에서 자신의 애니메이션을 등록하도록 순수 가상 함수로 선언
    virtual void RegisterAllAnimations() override = 0;
    virtual void UpdateAnimatorState() override;

    virtual void Damaged(int damage) override;


protected:
    int m_hp;
    int maxHp;
    float m_hitAnimTimer;
    
}; 
#pragma once
#include "Building.h"

class SpiderEgg : public Building
{
public:
    SpiderEgg(GameObjectID id, float x, float y, float pivotX, float pivotY, 
        Direction _dir, const std::wstring& resourcePath = L"",
        const std::wstring& imageName = L"", int hp = 100);
    virtual ~SpiderEgg();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    // Building 특화 메소드
    virtual void Damaged(int damage) override;
    virtual void SetTimeState(BuildingState buildingState) override;
    virtual BuildingState GetTimeState() const override;
    virtual std::wstring GetAnimKey(BuildingState state) override;

    // Entity 인터페이스 구현 (Unity Animator 스타일 - Enum + Direction 키값)
    virtual void RegisterAllAnimations() override;
    virtual void UpdateAnimatorState() override;

    // GameObject의 애니메이션 메소드들 오버라이드
    virtual void UpdateAnimation(float deltaTime) override;
    virtual Gdiplus::Bitmap* GetBitmap() const override;

private:

}; 
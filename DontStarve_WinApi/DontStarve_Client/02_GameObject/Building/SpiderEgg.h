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

    // Building 특화 메서드
    virtual void Damaged(int damage) override;
    virtual void SetTimeState(BuildingState buildingState) override;
    virtual BuildingState GetTimeState() const override;
    
    // 애니메이션 정의 제공 (Entity 오버라이드)
    virtual std::vector<AnimationDefinition> GetAnimationDefinitions() const override;

private:

};

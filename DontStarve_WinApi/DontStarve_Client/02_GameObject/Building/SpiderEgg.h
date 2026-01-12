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
    void SetTimeState(BuildingState buildingState) override;
    BuildingState GetTimeState() const override;

private:

};

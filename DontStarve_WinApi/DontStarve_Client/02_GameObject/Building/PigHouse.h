#pragma once

#include "Building.h"

class PigHouse : public Building
{
public:
    PigHouse(GameObjectID id, float x, float y, float pivotX, float pivotY, 
        Direction _dir, const std::wstring& resourcePath = L"",
        const std::wstring& imageName = L"", int hp = 100);
    virtual ~PigHouse();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    // Building Ưȭ �޼ҵ�
    virtual void Damaged(int damage) override;
    virtual void SetTimeState(BuildingState buildingState) override;
    virtual BuildingState GetTimeState() const override;
   
    // virtual std::wstring GetAnimKey(BuildingState state) override;

private:

};


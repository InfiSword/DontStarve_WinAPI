#pragma once
#include "Monster.h"

class Hound : public Monster
{
public:
    Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Hound();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;
};

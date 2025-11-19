#pragma once
#include "Monster.h"

class Pig : public Monster
{
public:
    Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Pig();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;

    virtual void RegisterAllAnimations() override;
};

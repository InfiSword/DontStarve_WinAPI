#pragma once
#include "Monster.h"

class ResourceManager;

class Hound : public Monster
{
public:
    Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir = L"", const std::wstring& imageName = L"");
    virtual ~Hound();

    virtual void Init() override;
    virtual bool OnInteraction(GameObject* obj) override;
};

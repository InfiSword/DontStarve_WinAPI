#pragma once
#include "Monster.h"

class ResourceManager;

class Pig : public Monster
{
public:
    Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName = L"");
    virtual ~Pig();

    virtual void Init() override;    
    virtual void OnInteraction(GameObject* obj) override;
    virtual void Damaged(int damage) override;
};

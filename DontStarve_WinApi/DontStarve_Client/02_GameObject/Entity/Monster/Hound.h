#pragma once
#include "Monster.h"

class ResourceManager;

class Hound : public Monster
{
public:
	static void RegisterResources(ResourceManager* rm);

    Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName = L"");
    virtual ~Hound();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;
};

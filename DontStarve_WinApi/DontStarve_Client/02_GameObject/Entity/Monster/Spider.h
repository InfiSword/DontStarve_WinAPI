#pragma once
#include "Monster.h"

class ResourceManager;

class Spider : public Monster
{
public:
	static void RegisterResources(ResourceManager* rm);

    Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName = L"");
    virtual ~Spider();

    virtual void Init() override;
    virtual void OnInteraction(GameObject* obj) override;    
    virtual void Damaged(int damage) override;
};

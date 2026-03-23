#pragma once
#include "Item.h"
class Player;

class Ingredient : public Item
{
public:
	Ingredient(GameObjectID id, const std::wstring& name, const std::wstring& desc, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");

	virtual ~Ingredient();
public:
	virtual void Init() override;
	virtual void LateInit() override;
	virtual void Update(float deltaTime) override; 
	virtual void LateUpdate() override;
	virtual void Release() override;

private:
};


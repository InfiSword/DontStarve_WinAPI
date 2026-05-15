#pragma once
#include "../Tool.h"

class Torch : public Tool
{
public:
	Torch(GameObjectID id, float x = 0.0f, float y = 0.0f, float pivotX = 0.5f, float pivotY = 0.5f,
		Direction _dir = DIR_DOWN, const std::wstring& baseDir = L"", const std::wstring& imageName = L"",
		ColliderType col = COLLIDER_BOX, bool isActive = true, bool isInteractive = true);
	virtual ~Torch();

};

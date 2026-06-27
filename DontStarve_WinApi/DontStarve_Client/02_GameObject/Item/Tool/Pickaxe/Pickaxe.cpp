#include "99_Default/pch.h"
#include "Pickaxe.h"

Pickaxe::Pickaxe(GameObjectID id, float x, float y, float pivotX, float pivotY, 
	Direction _dir, const std::wstring& baseDir, const std::wstring& imageName, 
	ColliderType col, bool isActive, bool isInteractive)
	: Tool(id, x, y, pivotX, pivotY, _dir, baseDir, imageName, col, isActive, isInteractive)
{
}

Pickaxe::~Pickaxe()
{
}

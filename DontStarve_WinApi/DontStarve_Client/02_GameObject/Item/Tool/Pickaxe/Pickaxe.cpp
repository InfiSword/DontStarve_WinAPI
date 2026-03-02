#include "99_Default/pch.h"
#include "Pickaxe.h"

Pickaxe::Pickaxe(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
	: Tool(id, name, desc, baseDir, imageName)
{
}

Pickaxe::~Pickaxe()
{
}

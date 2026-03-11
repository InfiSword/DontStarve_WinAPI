#include "99_Default/pch.h"
#include "Pickaxe.h"

Pickaxe::Pickaxe(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName, int damage, float attackRange)
	: Tool(id, name, desc, baseDir, imageName, damage, attackRange)
{
}

Pickaxe::~Pickaxe()
{
}

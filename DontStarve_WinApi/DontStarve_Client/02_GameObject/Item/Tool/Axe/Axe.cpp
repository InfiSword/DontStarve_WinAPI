#include "99_Default/pch.h"
#include "Axe.h"

Axe::Axe(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName, int damage, float attackRange)
	: Tool(id, name, desc, baseDir, imageName, damage, attackRange)
{
}

Axe::~Axe()
{
}

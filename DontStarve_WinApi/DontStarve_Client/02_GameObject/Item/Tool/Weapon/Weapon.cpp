#include "99_Default/pch.h"
#include "Weapon.h"

Weapon::Weapon(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
	: Tool(id, name, desc, baseDir, imageName)
{
}

Weapon::~Weapon()
{
}

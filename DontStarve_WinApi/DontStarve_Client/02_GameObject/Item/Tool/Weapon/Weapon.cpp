#include "99_Default/pch.h"
#include "Weapon.h"
#include "../../../Entity/Entity.h"

static float GetWeaponDamage(GameObjectID id) {
	switch (id) {
		case GOID_TOOL_GOLDEN_SCYTHE: return 20.0f;
		case GOID_TOOL_HAM_BAT:       return 25.0f;
		case GOID_TOOL_SPEAR:         return 15.0f;
		case GOID_TOOL_SWAP_SPEAR:    return 30.0f;
		default:                      return 20.0f;
	}
}

Weapon::Weapon(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
	: Tool(id, name, desc, baseDir, imageName, GetWeaponDamage(static_cast<GameObjectID>(id)))
{
}

Weapon::~Weapon()
{
}

void Weapon::Use(float durabilityCost)
{
	Tool::Use(durabilityCost);
}

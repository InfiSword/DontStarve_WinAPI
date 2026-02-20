#include "99_Default/pch.h"
#include "Pickaxe.h"
#include "../../../Entity/Entity.h"

static float GetPickaxeDamage(GameObjectID id) {
	switch (id) {
		case GOID_TOOL_PICKAXE: return 10.0f;
		default:                return 10.0f;
	}
}

Pickaxe::Pickaxe(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
	: Tool(id, name, desc, baseDir, imageName, GetPickaxeDamage(static_cast<GameObjectID>(id)))
{
}

Pickaxe::~Pickaxe()
{
}

void Pickaxe::Use(float durabilityCost)
{
	Tool::Use(durabilityCost);
}

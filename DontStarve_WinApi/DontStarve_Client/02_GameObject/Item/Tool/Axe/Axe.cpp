#include "99_Default/pch.h"
#include "Axe.h"
#include "../../../Entity/Entity.h"

static AxeStats GetAxeStats(GameObjectID id) {
	switch (id) {
		case GOID_TOOL_RED_AXE:  return { 25, 100.0f, 1.0f };
		case GOID_TOOL_SWAP_AXE: return { 30, 100.0f, 1.0f };
		default:                 return { 25, 100.0f, 1.0f };
	}
}

Axe::Axe(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
	: Tool(id, name, desc, baseDir, imageName, GetAxeStats(static_cast<GameObjectID>(id)).damage, GetAxeStats(static_cast<GameObjectID>(id)).durability, GetAxeStats(static_cast<GameObjectID>(id)).effectiveness)
{
}

Axe::~Axe()
{
}

void Axe::Use(float durabilityCost)
{ 
    Tool::Use(durabilityCost);
}

//void Axe::UseTarget(Entity* entity, int useDurability)
//{
//	if (entity->GetActive())
//	{
//		entity->Damaged(m_damage);
//	}
//	Use(useDurability);
//}

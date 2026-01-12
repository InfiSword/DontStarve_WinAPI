#include "99_Default/pch.h"
#include "Axe.h"
#include "../../../Entity/Entity.h"

Axe::Axe(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& imagePath, float durability, float effectiveness)
	: Tool(id, name, desc, imagePath, durability, effectiveness), m_damage(5)
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

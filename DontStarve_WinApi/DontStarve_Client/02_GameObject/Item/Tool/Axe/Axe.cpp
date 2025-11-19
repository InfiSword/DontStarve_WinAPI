#include "../../../99_Default/pch.h"
#include "Axe.h"
#include "../../../02_GameObject/GameObject/Tree.h"
#include "../../../02_GameObject/GameObject/Monster.h"

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

// 도끼로 나무를 벨 때
void Axe::ChopTree( Tree* tree) {
    tree->Damaged(m_damage); 
    Use(1.0f);
}

// 도끼로 몬스터를 때릴 때
void Axe::AttackMonster( Monster* monster) 
{
    monster->Damaged(m_damage);
    Use(2.0f); 
}
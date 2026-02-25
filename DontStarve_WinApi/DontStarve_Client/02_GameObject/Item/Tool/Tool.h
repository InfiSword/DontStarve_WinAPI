#pragma once
#include "../Item.h"

class Tool : public Item 
{
protected: 
	float m_damage;
	float m_attackRange;
	bool  m_canAttack;

public:
    Tool(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName);
    virtual ~Tool();

	float GetDamage()      const { return m_damage; }
	float GetAttackRange() const { return m_attackRange; }
	bool  CanAttack()      const { return m_canAttack; }
};

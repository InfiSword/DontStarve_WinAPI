#pragma once
#include "../Item.h"

enum class ToolState 
{
	NORMAL,
	CRACKED,
	BROKEN,
};

class Tool : public Item 
{
protected: 
	int m_damage;
	float m_attackRange;
	bool  m_canAttack;
	ToolState m_toolState;

public:
    Tool(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName, int damage, float attackRange);
    virtual ~Tool();

	int GetDamage()      const { return m_damage; }
	float GetAttackRange() const { return m_attackRange; }
	bool  CanAttack()      const { return m_canAttack; }
	ToolState GetToolState() const { return m_toolState; }
};

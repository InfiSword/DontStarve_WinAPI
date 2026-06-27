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
    Tool(GameObjectID id, float x = 0.0f, float y = 0.0f, float pivotX = 0.5f, float pivotY = 0.5f,
         Direction _dir = DIR_DOWN, const std::wstring& baseDir = L"", const std::wstring& imageName = L"", 
         ColliderType col = COLLIDER_BOX, bool isActive = true, bool isInteractive = true);
    virtual ~Tool();

	int GetDamage()      const { return m_damage; }
	float GetAttackRange() const { return m_attackRange; }
	bool  CanAttack()      const { return m_canAttack; }
	ToolState GetToolState() const { return m_toolState; }
};

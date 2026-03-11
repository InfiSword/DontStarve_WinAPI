#include "99_Default/pch.h"
#include "Tool.h"

Tool::Tool(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName, int damage, float attackRange)
    : Item(id, name, desc, baseDir, imageName, 0, 0, 0.5f, 0.5f, DIR_DOWN, true, true)
    , m_damage(damage)
    , m_attackRange(attackRange)
    , m_canAttack(true)
	, m_toolState(ToolState::NORMAL)
{

}

Tool::~Tool()
{

}

#include "99_Default/pch.h"
#include "Tool.h"

Tool::Tool(GameObjectID id, float x, float y, float pivotX, float pivotY, 
    Direction _dir, const std::wstring& baseDir, const std::wstring& imageName, 
    ColliderType col, bool isActive, bool isInteractive)
    : Item(id, x, y, pivotX, pivotY, _dir, baseDir, imageName, col, isActive, isInteractive)
    , m_canAttack(true)
	, m_toolState(ToolState::NORMAL)
{
	const ToolInfo* info = DataTable::GetToolInfo(id);
	if (info) {
		m_damage = info->damage;
		m_attackRange = info->attackRange;
	}
	else {
		m_damage = 0;
		m_attackRange = 0.0f;
	}
}

Tool::~Tool()
{

}

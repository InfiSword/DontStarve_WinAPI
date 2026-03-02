#include "99_Default/pch.h"
#include "Tool.h"

Tool::Tool(GameObjectID id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
    : Item(id, name, desc, baseDir, imageName, 0, 0, 0.5f, 0.5f, DIR_DOWN, true, true)
    , m_damage(10.0f)
    , m_attackRange(100.0f)
    , m_canAttack(true)
	, m_toolState(ToolState::NORMAL)
{
    if (id == GOID_TOOL_SPEAR || id == GOID_TOOL_SWAP_SPEAR) {
        m_damage = 34.0f;
        m_attackRange = 120.0f;
    }
}

Tool::~Tool()
{

}

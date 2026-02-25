#include "99_Default/pch.h"
#include "Tool.h"

struct ToolStats {
    float damage;
    float attackRange;
    bool  canAttack;
};

static ToolStats GetToolStats(GameObjectID id)
{
    switch (id) {
        case GOID_TOOL_RED_AXE:       return { 25.0f,  80.0f, true  };
        case GOID_TOOL_SWAP_AXE:      return { 30.0f,  80.0f, true  };
        case GOID_TOOL_PICKAXE:       return { 10.0f,  80.0f, true  };
        case GOID_TOOL_TORCH:         return {  5.0f,  80.0f, true  };
        case GOID_TOOL_SPEAR:         return { 15.0f,  90.0f, true  };
        case GOID_TOOL_SWAP_SPEAR:    return { 30.0f,  90.0f, true  };
        case GOID_TOOL_HAM_BAT:       return { 25.0f,  80.0f, true  };
        case GOID_TOOL_GOLDEN_SCYTHE: return { 20.0f,  80.0f, true  };
        case GOID_TOOL_HALBERD:       return { 35.0f, 100.0f, true  };
        case GOID_TOOL_HAMMER:        return { 10.0f,  80.0f, false };
        default:                      return {  0.0f,  80.0f, false };
    }
}

Tool::Tool(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& baseDir, const std::wstring& imageName)
    : Item(GOBJ_ITEM, (GameObjectID)id, name, desc, baseDir, imageName)
{
    ToolStats stats = GetToolStats(static_cast<GameObjectID>(id));
    m_damage      = stats.damage;
    m_attackRange = stats.attackRange;
    m_canAttack   = stats.canAttack;
}

Tool::~Tool() {}

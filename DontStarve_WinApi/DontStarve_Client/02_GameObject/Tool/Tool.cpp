#include "../../99_Default/pch.h"
#include "Tool.h"

Tool::Tool(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& imagePath, float maxDurability, float effectiveness)
    : Item(GOBJ_ITEM, (GameObjectID)id, name, desc, L"", imagePath), 
    m_durability(maxDurability), m_maxDurability(maxDurability), m_effectiveness(effectiveness) {
}

Tool::~Tool() {

}

void Tool::Use(float durabilityCost) 
{
    ReduceDurability(durabilityCost); 

    if (IsBroken()) {      
    }
}

void Tool::ReduceDurability(float amount) {
    m_durability -= amount;
    if (m_durability <= 0.0f) {
        m_durability = 0.0f;
    }
}
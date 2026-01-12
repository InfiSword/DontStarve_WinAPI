#pragma once
#include "../Item.h"

class Tool : public Item 
{
protected: 
    float m_durability; 
    float m_maxDurability; 
    float m_effectiveness; 

public:

    Tool(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& imagePath, float maxDurability = 100.0f, float effectiveness = 1.0f);
    virtual ~Tool();

    float GetDurability() const { return m_durability; }
    float GetMaxDurability() const { return m_maxDurability; }
    float GetEffectiveness() const { return m_effectiveness; }


    virtual void Use(float durabilityCost);

    bool IsBroken() const { return m_durability <= 0.0f; }

    void ReduceDurability(float amount);
};

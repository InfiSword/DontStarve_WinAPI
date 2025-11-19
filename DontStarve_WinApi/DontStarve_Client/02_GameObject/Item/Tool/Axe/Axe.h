#pragma once
#include "../Tool.h"

class Axe : public Tool
{
public:
    Axe(UINT id, const std::wstring& name, const std::wstring& desc, const std::wstring& imagePath, float durability = 100.0f, float effectiveness = 1.0f);
    virtual ~Axe();

    virtual void Use(float durabilityCost) override;

    void ChopTree(class Tree* tree);
    void AttackMonster(class Monster* monster); 

private:
    int m_damage;           // 도끼 데미지
}; 
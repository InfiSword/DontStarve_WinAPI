#pragma once
#include "Hound.h"

class Boss_Hound : public Hound
{
public:
    Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Boss_Hound();

    virtual void Init() override;
    virtual void RegisterAllAnimations() override;
    virtual void Damaged(int damage) override;

private:
    // 보스 전용 추가 속성들
    int m_bossPhase;
    float m_specialAttackCooldown;
    std::wstring m_houndType; // "Red" 또는 "Ice"
}; 
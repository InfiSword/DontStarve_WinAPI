#pragma once
#include "Spider.h"

class Boss_SpiderQueen : public Spider
{
public:
    Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath = L"", const std::wstring& imageName = L"");
    virtual ~Boss_SpiderQueen();

    virtual void Init() override;
    virtual void RegisterAllAnimations() override;
    virtual void Damaged(int damage) override;

private:
    // 보스 전용 추가 속성들
    int m_bossPhase;
    float m_specialAttackCooldown;
}; 
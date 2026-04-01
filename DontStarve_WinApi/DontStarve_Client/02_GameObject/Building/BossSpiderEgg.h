#pragma once
#include "SpiderEgg.h"

class BossSpiderEgg : public SpiderEgg
{
public:
    BossSpiderEgg(GameObjectID id, float x, float y, float pivotX, float pivotY, 
        Direction _dir, const std::wstring& resourcePath = L"",
        const std::wstring& imageName = L"", int hp = 80);
    virtual ~BossSpiderEgg() override;

    virtual void Init() override;
    virtual void Update(float deltaTime) override;

protected:
    // 거미 스폰을 위한 내부 로직 재정의
    void PreSpawnSpiders();

private:
    float m_periodicSpawnTimer;
    float m_periodicSpawnInterval;
};

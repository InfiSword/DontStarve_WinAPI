#include "99_Default/pch.h"
#include "BossSpiderEgg.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../Entity/Player/Player.h"
#include "../Entity/Monster/Spider.h"

BossSpiderEgg::BossSpiderEgg(GameObjectID id, float x, float y, float pivotX, float pivotY,
    Direction _dir, const std::wstring& resourcePath,
    const std::wstring& imageName, int hp)
    : SpiderEgg(id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, hp)
    , m_periodicSpawnTimer(0.0f)
    , m_periodicSpawnInterval(5.0f) // 5초마다 스폰 시도
{
}

BossSpiderEgg::~BossSpiderEgg()
{
}

void BossSpiderEgg::Init()
{
    SpiderEgg::Init();
    // 보스 알은 기본 SpiderEgg의 자동 스폰 로직을 사용하지 않기 위해
    // m_remainingSpiders를 0으로 설정하여 PreSpawnSpiders가 아무것도 하지 않게 함
}

void BossSpiderEgg::Update(float deltaTime)
{
    SpiderEgg::Update(deltaTime);

    if (m_buildingState == BuildingState::DESTROYED) return;

    m_periodicSpawnTimer += deltaTime;
    if (m_periodicSpawnTimer >= m_periodicSpawnInterval)
    {
        m_periodicSpawnTimer = 0.0f;
        
        // 거미 한 마리 스폰
        ObjectManager* objectManager = ObjectManager::GetInstance();
        Transform* transform = GetComponent<Transform>();
        if (objectManager && transform)
        {
            GameObjectID spiderID = GOID_MONSTER_SPIDER;
            int roll = rand() % 100;
            
            EggStage stage = GetEggStage();
            if (stage == EggStage::Medium)
            {
                // Medium: 일반 70%, 전사 30%
                if (roll < 30) spiderID = GOID_MONSTER_WARRIOR_SPIDER;
            }
            else if (stage == EggStage::Large)
            {
                // Tall (Large): 일반 50%, 전사 50%
                if (roll < 50) spiderID = GOID_MONSTER_WARRIOR_SPIDER;
            }
            // Small은 100% 일반 거미

            Entity* spiderObj = objectManager->CreateEntity(spiderID, transform->GetX(), transform->GetY());
            if (spiderObj)
            {
                Spider* spider = dynamic_cast<Spider*>(spiderObj);
                if (spider)
                {
                    // 스폰 위치 보정 (알 주변)
                    float angle = (rand() / (float)RAND_MAX) * 6.283185f;
                    float dist = 40.0f + (rand() / (float)RAND_MAX) * 40.0f;
                    spider->GetComponent<Transform>()->SetPosition(transform->GetX() + cosf(angle) * dist, transform->GetY() + sinf(angle) * dist);
                    spider->ClampPositionToMapBounds();

                    // 플레이어 즉시 추격
                    GameObject* player = objectManager->GetPlayer();
                    if (player) spider->SetAggroTarget(player);
                }
            }
        }
    }
}

void BossSpiderEgg::PreSpawnSpiders()
{
    // 보스 알은 Update에서 직접 생성하므로 기본 SpiderEgg의 풀링/자동스폰을 막음
}

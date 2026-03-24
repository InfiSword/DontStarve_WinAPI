#pragma once
#include "Building.h"

// Egg 단계: Sac(땅에서 막 생성), Small, Medium, Large
enum class EggStage { Sac = 0, Small, Medium, Large, EggStageCount };

// Animator 상태 상수 (int state)
enum EggAnimState {
    EGG_STATE_IDLE_SMALL = 0,
    EGG_STATE_IDLE_MEDIUM = 1,
    EGG_STATE_IDLE_LARGE = 2,
    EGG_STATE_HIT_SMALL = 10,
    EGG_STATE_HIT_MEDIUM = 11,
    EGG_STATE_HIT_LARGE = 12,
    EGG_STATE_GROW_SAC_TO_SMALL = 20,
    EGG_STATE_GROW_SMALL_TO_MEDIUM = 21,
    EGG_STATE_GROW_MEDIUM_TO_LARGE = 22,
    EGG_STATE_BURST_LARGE = 30
};

class SpiderEgg : public Building
{
public:
    SpiderEgg(GameObjectID id, float x, float y, float pivotX, float pivotY, 
        Direction _dir, const std::wstring& resourcePath = L"",
        const std::wstring& imageName = L"", int hp = 100);
    virtual ~SpiderEgg();

    virtual void Init() override;
    virtual void LateInit() override;
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override;
    virtual void Release() override;

    // GameObject 상호작용
    virtual bool OnInteraction(GameObject* obj) override;

    // Building 특화 메서드
    virtual void Damaged(int damage) override;
    void SetTimeState(BuildingState buildingState) override;
    BuildingState GetTimeState() const override;

    // 성장: Sac→Small, Small→Medium, Medium→Large. 해당 성장 애니 재생 후 단계 전환.
    void Grow();

    EggStage GetEggStage() const { return m_eggStage; }
    void SetEggStage(EggStage stage) { m_eggStage = stage; }
    
    // 거미 스폰
    void SpawnSpiders();

private:
    void PreSpawnSpiders();

private:
    EggStage m_eggStage;
    bool m_isPlayingGrowth;
    bool m_isPlayingHit;
    float m_spawnRadius;  // 거미 스폰 범위
    float m_invincibleTimer; // 성장 후 무적 타이머

	int m_amountOfSpidersToSpawn; // 한 번에 스폰할 거미 수
	int m_remainingSpiders;       // 현재 거미집 안에 남은 총 거미 수

    std::vector<class Spider*> m_poolSpiders; // 미리 생성해 둔 거미 오브젝트 풀
};

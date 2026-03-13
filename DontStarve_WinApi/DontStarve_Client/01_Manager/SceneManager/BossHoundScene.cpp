#include "99_Default/pch.h"
#include "BossHoundScene.h"
#include "../ObjectManager/ObjectManager.h"
#include "../CameraManager/CameraManager.h"
#include "../UIManager/UIManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../ResourceManager/ResourceManager.h"
#include "../GameProgressManager/GameProgressManager.h"
#include "../SceneManager/SceneManager.h"

BossHoundScene::BossHoundScene()
    : GameScene()
    , m_currentPhase(BossPhase::Phase1_Hounds)
    , m_phaseTimer(0.0f)
    , m_isIntroRunning(false)
    , m_introTimer(0.0f)
    , m_bossesActivated(false)
{
}

BossHoundScene::~BossHoundScene()
{
}

void BossHoundScene::Init()
{
    GameScene::Init();
    
    m_currentPhase = BossPhase::Phase1_Hounds;
    m_phaseTimer = 0.0f;
    m_bossesActivated = false;
    m_isIntroRunning = false;
    m_bossObjects.clear();

    // 맵 데이터에서 생성된 보스들을 찾아 비활성화
    ObjectManager* objMgr = ObjectManager::GetInstance();
    const auto& objects = objMgr->GetGameObjects();
    
    for (auto* obj : objects)
    {
        if (obj && (obj->GetID() == GOID_MONSTER_REDHOUNDDOG || obj->GetID() == GOID_MONSTER_ICEHOUNDDOG))
        {
            obj->SetActive(false);
            m_bossObjects.push_back(obj);
        }
    }

    OutputDebugStringW((L"BossHoundScene: Initialized. Bosses found: " + std::to_wstring(m_bossObjects.size()) + L"\n").c_str());
}

void BossHoundScene::Update(float deltaTime)
{
    // 페이즈에 따라 게임 로직 업데이트 순서 조정 가능
    // Intro 중에는 플레이어 이동 등을 막고 싶을 수 있음 (여기서는 일단 기본 Update 호출)
    GameScene::Update(deltaTime);

    switch (m_currentPhase)
    {
    case BossPhase::Phase1_Hounds:
        UpdatePhase1(deltaTime);
        break;
    case BossPhase::PhaseTransition:
        UpdatePhaseTransition(deltaTime);
        break;
    case BossPhase::Phase2_BossIntro:
        UpdatePhase2Intro(deltaTime);
        break;
    case BossPhase::Phase2_BossBattle:
        UpdatePhase2Battle(deltaTime);
        break;
    }
}

void BossHoundScene::Render()
{
    GameScene::Render();
}

void BossHoundScene::UpdatePhase1(float deltaTime)
{
    // 필드에 있는 모든 일반 하운드(minion)가 죽었는지 체크
    ObjectManager* objMgr = ObjectManager::GetInstance();
    const auto& objects = objMgr->GetGameObjects();
    
    bool houndsAlive = false;
    for (auto* obj : objects)
    {
        // 보스가 아닌 일반 하운드만 체크
        if (obj && obj->GetID() == GOID_MONSTER_HOUNDDOG && obj->IsEnabled())
        {
            houndsAlive = true;
            break;
        }
    }

    if (!houndsAlive)
    {
        m_currentPhase = BossPhase::PhaseTransition;
        m_phaseTimer = 0.0f;
        OutputDebugStringW(L"BossHoundScene: Phase 1 Cleared. Transitioning to Phase 2...\n");
    }
}

void BossHoundScene::UpdatePhaseTransition(float deltaTime)
{
    m_phaseTimer += deltaTime;
    
    // 2초 정도 대기 후 보스 등장 연출 시작
    if (m_phaseTimer >= 2.0f)
    {
        StartBossIntro();
    }
}

void BossHoundScene::StartBossIntro()
{
    m_currentPhase = BossPhase::Phase2_BossIntro;
    m_introTimer = 0.0f;
    m_isIntroRunning = true;

    CameraManager* camMgr = CameraManager::GetInstance();
    camMgr->SetFollowMode(false); // 수동 카메라 제어
    
    m_introStartPos = camMgr->GetCameraPos();
    
    // 보스들이 있는 위치의 중심으로 카메라 이동
    if (!m_bossObjects.empty())
    {
        float totalX = 0, totalY = 0;
        for (auto* boss : m_bossObjects)
        {
            Transform* tr = boss->GetComponent<Transform>();
            if (tr)
            {
                totalX += tr->GetX();
                totalY += tr->GetY();
            }
        }
        m_introTargetPos = { totalX / m_bossObjects.size(), totalY / m_bossObjects.size() };
    }
    else
    {
        // 보스가 없으면 (에러 상황) 플레이어 위치
        Player* player = ObjectManager::GetInstance()->GetPlayer();
        if (player) {
            Transform* tr = player->GetComponent<Transform>();
            m_introTargetPos = { tr->GetX(), tr->GetY() };
        }
    }

    OutputDebugStringW(L"BossHoundScene: Boss Intro Started.\n");
}

void BossHoundScene::UpdatePhase2Intro(float deltaTime)
{
    m_introTimer += deltaTime;
    CameraManager* camMgr = CameraManager::GetInstance();

    float moveDuration = 2.0f; // 카메라 이동 시간
    float waitDuration = 1.5f; // 보스 활성화 후 대기 시간
    float returnDuration = 1.5f; // 플레이어에게 돌아가는 시간

    if (m_introTimer <= moveDuration)
    {
        // 1. 타겟 위치로 카메라 이동
        float t = m_introTimer / moveDuration;
        float curX = m_introStartPos.X + (m_introTargetPos.X - m_introStartPos.X) * t;
        float curY = m_introStartPos.Y + (m_introTargetPos.Y - m_introStartPos.Y) * t;
        camMgr->SetCameraPos(curX, curY);
    }
    else if (m_introTimer <= moveDuration + waitDuration)
    {
        // 2. 보스 활성화
        if (!m_bossesActivated)
        {
            SpawnBoss(); // 실제로 활성화하는 함수
        }
    }
    else if (m_introTimer <= moveDuration + waitDuration + returnDuration)
    {
        // 3. 플레이어에게 카메라 복귀
        Player* player = ObjectManager::GetInstance()->GetPlayer();
        if (player)
        {
            Transform* tr = player->GetComponent<Transform>();
            float t = (m_introTimer - moveDuration - waitDuration) / returnDuration;
            float curX = m_introTargetPos.X + (tr->GetX() - m_introTargetPos.X) * t;
            float curY = m_introTargetPos.Y + (tr->GetY() - m_introTargetPos.Y) * t;
            camMgr->SetCameraPos(curX, curY);
        }
    }
    else
    {
        // 연출 종료
        camMgr->SetFollowMode(true);
        m_currentPhase = BossPhase::Phase2_BossBattle;
        OutputDebugStringW(L"BossHoundScene: Phase 2 Battle Started.\n");
    }
}

void BossHoundScene::SpawnBoss()
{
    if (m_bossesActivated) return;

    for (auto* boss : m_bossObjects)
    {
        if (boss) boss->SetActive(true);
    }
    m_bossesActivated = true;
    OutputDebugStringW(L"BossHoundScene: Bosses Activated!\n");
}

void BossHoundScene::UpdatePhase2Battle(float deltaTime)
{
    // 모든 보스가 죽었는지 체크
    bool bossesAlive = false;
    for (auto* boss : m_bossObjects)
    {
        if (boss && boss->IsEnabled())
        {
            bossesAlive = true;
            break;
        }
    }

    if (!bossesAlive)
    {
        m_currentPhase = BossPhase::Cleared;
        OutputDebugStringW(L"BossHoundScene: All Bosses Defeated! Scene Cleared.\n");
        
        // 클리어 처리
        // 어떤 보스든 하나만 넘겨도 되는지, 아니면 씬 클리어 이벤트가 따로 있는지 확인
        // 여기선 첫 번째 보스 ID를 넘겨 기록
        if (!m_bossObjects.empty())
            GameProgressManager::GetInstance()->OnMonsterKilled(m_bossObjects[0]->GetID(), GetSceneType());
    }
}

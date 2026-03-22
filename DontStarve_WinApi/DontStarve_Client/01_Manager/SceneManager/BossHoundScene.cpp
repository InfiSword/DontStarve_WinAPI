#include "99_Default/pch.h"
#include "BossHoundScene.h"
#include "../ObjectManager/ObjectManager.h"
#include "../CameraManager/CameraManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/Entity/Entity.h"
#include "../../02_GameObject/Entity/Monster/Monster.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../ResourceManager/ResourceManager.h"
#include "../GameProgressManager/GameProgressManager.h"
#include "../SceneManager/SceneManager.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIText.h"
#include "../../02_GameObject/UI/HPUI.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

BossHoundScene::BossHoundScene()
    : GameScene()
    , m_currentPhase(BossPhase::Phase1_Hounds)
    , m_phaseTimer(0.0f)
    , m_isIntroRunning(false)
    , m_introTimer(0.0f)
    , m_introTargetBossIndex(0)
    , m_bossesActivated(false)
    , m_isClearUIShown(false)
    , m_clearBanner(nullptr)
    , m_clearText(nullptr)
    , m_iceBossHPUI(nullptr)
    , m_redBossHPUI(nullptr)
{
}

BossHoundScene::~BossHoundScene()
{
}

void BossHoundScene::Init(const MapData* mapData)
{
    // 부모 클래스의 Init(mapData) 호출하여 맵 데이터 기반 오브젝트들 생성
    GameScene::Init(mapData);

    // 부모 클래스 초기화 후 상태값 재설정 (Init() 호출 시 덮어써지는 것 방지)
    m_currentPhase = BossPhase::Phase1_Hounds;
    m_phaseTimer = 0.0f;
    m_bossesActivated = false;
    m_isIntroRunning = false;
    m_isClearUIShown = false;
    m_introTargetBossIndex = 0;
    m_bossObjects.clear();
    m_clearBanner = nullptr;
    m_clearText = nullptr;
    m_iceBossHPUI = nullptr;
    m_redBossHPUI = nullptr;

    ObjectManager* uiMgr = ObjectManager::GetInstance();
    // 클리어 UI 미리 생성
    m_clearBanner = new UIImage(GOID_NONE, 600.0f, 150.0f, LAYER_UI_BACKGROUND, L"Resource/UI/BG_Banner.png", 999.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, -50.0f);
    m_clearBanner->SetActive(false);
    uiMgr->AddGameObject(m_clearBanner);

    m_clearText = new UIText(GOID_NONE, 400.0f, 100.0f, L"CLEARED!", Gdiplus::Color(255, 255, 255, 0), LAYER_UI_FOREGROUND, 1000.0f, L"Arial", 48.0f, Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, -50.0f);
    m_clearText->SetActive(false);
    uiMgr->AddGameObject(m_clearText);

    // 맵 데이터에서 생성된 보스들을 찾아 비활성화 (전투 준비 단계)
    // 연출 순서 보장을 위해 순서대로 찾음 (Ice -> Red)
    ObjectManager* objMgr = ObjectManager::GetInstance();
    const auto& objects = objMgr->GetGameObjects();
    
    GameObject* iceBoss = nullptr;
    GameObject* redBoss = nullptr;

    for (auto* obj : objects)
    {
        if (!obj) continue;
        if (obj->GetID() == GOID_MONSTER_ICEHOUNDDOG) iceBoss = obj;
        else if (obj->GetID() == GOID_MONSTER_REDHOUNDDOG) redBoss = obj;
    }

    if (iceBoss) { iceBoss->SetActive(false); m_bossObjects.push_back(iceBoss); }
    if (redBoss) { redBoss->SetActive(false); m_bossObjects.push_back(redBoss); }

    // 보스 HP UI 미리 생성 (Order: Ice -> Red)
    if (iceBoss)
    {
        m_iceBossHPUI = new HPUI(dynamic_cast<Entity*>(iceBoss), L"ICE HOUND", 600.0f, 25.0f, 
            Gdiplus::Color(200, 40, 0, 0), Gdiplus::Color(255, 200, 0, 0), Gdiplus::Color(255, 100, 150, 255),
            0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 60.0f,
            1000.0f, 1002.0f, false, false);
        m_iceBossHPUI->SetActive(false);
        uiMgr->AddGameObject(m_iceBossHPUI);
    }
    
    if (redBoss)
    {
        m_redBossHPUI = new HPUI(dynamic_cast<Entity*>(redBoss), L"RED HOUND", 600.0f, 25.0f,
            Gdiplus::Color(200, 40, 0, 0), Gdiplus::Color(255, 200, 0, 0), Gdiplus::Color(255, 255, 100, 100),
            0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 150.0f,
            1000.0f, 1002.0f, false, false);
        m_redBossHPUI->SetActive(false);
        uiMgr->AddGameObject(m_redBossHPUI);
    }


    OutputDebugStringW((L"BossHoundScene: Initialized with MapData. Bosses hidden: " + std::to_wstring(m_bossObjects.size()) + L"\n").c_str());
}

void BossHoundScene::Update(float deltaTime)
{
    // 페이즈에 따라 게임 로직 업데이트 순서 조정 가능
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
    case BossPhase::Cleared:
        UpdateCleared(deltaTime);
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
    m_introTargetBossIndex = 0;

    CameraManager* camMgr = CameraManager::GetInstance();
    camMgr->SetFollowMode(false); // 수동 카메라 제어
    
    m_introStartPos = camMgr->GetCameraPos();

    // 플레이어 입력 비활성화
    Player* player = ObjectManager::GetInstance()->GetPlayer();
    if (player) player->SetInputEnabled(false);

    // 만약 초기화 시 보스를 못 찾았다면 여기서 동적으로 생성
    if (m_bossObjects.empty())
    {
        ObjectManager* objMgr = ObjectManager::GetInstance();
        float px = 0, py = 0;
        if (player)
        {
            Transform* tr = player->GetComponent<Transform>();
            px = tr->GetX();
            py = tr->GetY();
        }

        // 보스 생성 (Ice -> Red 순서로 리스트에 추가)
        GameObject* iceBoss = objMgr->CreateGameObject(GOID_MONSTER_ICEHOUNDDOG, px - 300.0f, py - 200.0f);
        GameObject* redBoss = objMgr->CreateGameObject(GOID_MONSTER_REDHOUNDDOG, px + 300.0f, py - 200.0f);

        if (iceBoss) { iceBoss->SetActive(false); m_bossObjects.push_back(iceBoss); }
        if (redBoss) { redBoss->SetActive(false); m_bossObjects.push_back(redBoss); }
    }
    
    // 첫 번째 보스 위치로 타겟 설정
    if (!m_bossObjects.empty())
    {
        Transform* tr = m_bossObjects[0]->GetComponent<Transform>();
        if (tr) m_introTargetPos = { tr->GetX(), tr->GetY() };
    }
    else
    {
        m_introTargetPos = m_introStartPos;
    }

    OutputDebugStringW(L"BossHoundScene: Boss Intro Started. Player input disabled.\n");
}

void BossHoundScene::UpdatePhase2Intro(float deltaTime)
{
    m_introTimer += deltaTime;
    CameraManager* camMgr = CameraManager::GetInstance();

    float moveDuration = 1.0f;   // 보스 한 마리에게 이동하는 시간
    float waitDuration = 1.5f;   // 보스를 보여주는 시간 (Howling 애니메이션 고려)
    float returnDuration = 1.5f; // 플레이어에게 돌아오는 시간

    int bossCount = (int)m_bossObjects.size();
    float cycleDuration = moveDuration + waitDuration;
    float totalBossIntroDuration = bossCount * cycleDuration;

    if (m_introTimer <= totalBossIntroDuration)
    {
        // 보스들을 순차적으로 보여줌
        int currentIndex = (int)(m_introTimer / cycleDuration);
        float timeInCycle = fmod(m_introTimer, cycleDuration);

        if (currentIndex < bossCount)
        {
            GameObject* boss = m_bossObjects[currentIndex];
            Transform* tr = boss->GetComponent<Transform>();
            if (tr)
            {
                Gdiplus::PointF bossPos = { tr->GetX(), tr->GetY() };
                Gdiplus::PointF startPos;

                if (currentIndex == 0)
                    startPos = m_introStartPos;
                else
                {
                    Transform* prevTr = m_bossObjects[currentIndex - 1]->GetComponent<Transform>();
                    startPos = prevTr ? Gdiplus::PointF{ prevTr->GetX(), prevTr->GetY() } : m_introStartPos;
                }

                if (timeInCycle <= moveDuration)
                {
                    float t = timeInCycle / moveDuration;
                    float smoothT = t * t * (3 - 2 * t);
                    float curX = startPos.X + (bossPos.X - startPos.X) * smoothT;
                    float curY = startPos.Y + (bossPos.Y - startPos.Y) * smoothT;
                    camMgr->SetCameraPos(curX, curY);
                }
                else
                {
                    camMgr->SetCameraPos(bossPos.X, bossPos.Y);
                    
                    // 보스 활성화
                    if (!boss->IsEnabled())
                    {
                        boss->SetActive(true);
                        OutputDebugStringW((L"BossHoundScene: Boss " + std::to_wstring(currentIndex) + L" Activated!\n").c_str());
                    }
                }
            }
        }
    }
    else if (m_introTimer <= totalBossIntroDuration + returnDuration)
    {
        // 플레이어에게 카메라가 돌아가기 시작할 때 추격 허용

        // 플레이어에게 복귀
        Player* player = ObjectManager::GetInstance()->GetPlayer();
        if (player)
        {
            Transform* tr = player->GetComponent<Transform>();

            Gdiplus::PointF lastPos = m_introStartPos;
            if (!m_bossObjects.empty())
            {
                Transform* lastTr = m_bossObjects.back()->GetComponent<Transform>();
                if (lastTr) lastPos = { lastTr->GetX(), lastTr->GetY() };
            }

            float t = (m_introTimer - totalBossIntroDuration) / returnDuration;
            float smoothT = t * t * (3 - 2 * t);
            float curX = lastPos.X + (tr->GetX() - lastPos.X) * smoothT;
            float curY = lastPos.Y + (tr->GetY() - lastPos.Y) * smoothT;
            camMgr->SetCameraPos(curX, curY);
        }
    }
    else
    {
    	for (auto* boss : m_bossObjects)
    	{
    		Monster* pMonster = dynamic_cast<Monster*>(boss);
    		if (pMonster) pMonster->SetCanChase(true);
    	}

        // 모든 연출 종료
        m_bossesActivated = true;
        camMgr->SetFollowMode(true);
        m_currentPhase = BossPhase::Phase2_BossBattle;

        // 보스 HP UI 활성화
        if (m_iceBossHPUI) m_iceBossHPUI->SetActive(true);
        if (m_redBossHPUI) m_redBossHPUI->SetActive(true);

        // 플레이어 입력 재활성화
        Player* player = ObjectManager::GetInstance()->GetPlayer();
        if (player) player->SetInputEnabled(true);

        OutputDebugStringW(L"BossHoundScene: Phase 2 Battle Started. Player input re-enabled.\n");
    }
}

bool BossHoundScene::IsIntroReturning() const
{
    float moveDuration = 1.0f;
    float waitDuration = 1.5f;
    int bossCount = (int)m_bossObjects.size();
    float totalBossIntroDuration = bossCount * (moveDuration + waitDuration);

    return (m_currentPhase == BossPhase::Phase2_BossIntro && m_introTimer > totalBossIntroDuration);
}

void BossHoundScene::SpawnBoss()
{
    if (m_bossesActivated) return;

    for (auto* boss : m_bossObjects)
    {
        if (boss) boss->SetActive(true);
    }
    m_bossesActivated = true;
    OutputDebugStringW(L"BossHoundScene: Bosses Spawned and Activated!\n");
}

void BossHoundScene::UpdatePhase2Battle(float deltaTime)
{
    // 플레이어가 입력 가능할 때 보스 몬스터 2명도 동시에 Update (중첩 최소화, 중복 호출 제거)
    // ObjectManager에서 이미 모든 GameObject의 Update를 호출하므로, 별도 호출 불필요

    // 기존 보스 생존 체크 로직 유지
    ObjectManager* objMgr = ObjectManager::GetInstance();
    const auto& objects = objMgr->GetGameObjects();
    bool bossesAlive = false;

    bool iceAlive = false;
    bool redAlive = false;

    for (auto* obj : objects)
    {
        if (obj && obj->IsEnabled())
        {
            if (obj->GetID() == GOID_MONSTER_ICEHOUNDDOG) iceAlive = true;
            else if (obj->GetID() == GOID_MONSTER_REDHOUNDDOG) redAlive = true;
        }
    }

    if (iceAlive || redAlive) bossesAlive = true;

    // 죽은 보스의 HP UI 숨기기
    if (!iceAlive && m_iceBossHPUI) m_iceBossHPUI->SetActive(false);
    if (!redAlive && m_redBossHPUI) m_redBossHPUI->SetActive(false);

    if (!bossesAlive)
    {
        m_currentPhase = BossPhase::Cleared;
        m_phaseTimer = 0.0f;
        m_isClearUIShown = false;

        if (m_iceBossHPUI) m_iceBossHPUI->SetActive(false);
        if (m_redBossHPUI) m_redBossHPUI->SetActive(false);

        OutputDebugStringW(L"BossHoundScene: All Bosses Defeated! Scene Cleared.\n");
        // 클리어 기록 및 캐릭터 해금
        GameProgressManager::GetInstance()->ClearScene(SCENE_GAME_HOUND_FOREST);
    }
}

void BossHoundScene::UpdateCleared(float deltaTime)
{
    m_phaseTimer += deltaTime;

    // 1초 뒤에 클리어 UI 표시
    if (m_phaseTimer >= 1.0f && !m_isClearUIShown)
    {
        // 뒷배경 어둡게 하기 (또는 클리어 배너 표시)
        if (m_clearBanner) m_clearBanner->SetActive(true);
        if (m_clearText) m_clearText->SetActive(true);

        m_isClearUIShown = true;
        OutputDebugStringW(L"BossHoundScene: Clear UI displayed.\n");
    }

    // 클리어 화면 뜬지 3초(총 4초) 뒤에 씬 이동
    if (m_phaseTimer >= 4.0f)
    {
        OutputDebugStringW(L"BossHoundScene: Transitioning back to Farming Area...\n");
        SceneManager::GetInstance()->LoadGameScene(SCENE_GAME_FARMING_AREA, GetSelectedCharacterID());
    }
}

void BossHoundScene::Release()
{
    // UI 포인터 정리 (실제 삭제는 ObjectManager에서 수행됨)
    m_clearBanner = nullptr;
    m_clearText = nullptr;
    m_iceBossHPUI = nullptr;
    m_redBossHPUI = nullptr;
    m_bossObjects.clear();

    GameScene::Release();
}

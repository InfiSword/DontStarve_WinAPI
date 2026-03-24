#include "99_Default/pch.h"
#include "BossSpiderQueenScene.h"
#include "../ObjectManager/ObjectManager.h"
#include "../CameraManager/CameraManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/Entity/Monster/Monster.h"
#include "../../02_GameObject/Entity/Monster/Spider.h"
#include "../../02_GameObject/Building/SpiderEgg.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../GameProgressManager/GameProgressManager.h"
#include "../SceneManager/SceneManager.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIText.h"
#include "../../02_GameObject/UI/GameClearUI.h"
#include "../../02_GameObject/UI/HPUI.h"
#include "../../02_GameObject/Entity/Enviorment/Tree.h"
#include "../../03_Animation/Animator.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

BossSpiderQueenScene::BossSpiderQueenScene()
    : GameScene()
    , m_bossName(L"SPIDER QUEEN")
    , m_bossHPUI(nullptr)
    , m_currentPhase(BossPhase::Phase1_Minions)
    , m_phaseTimer(0.0f)
    , m_isIntroRunning(false)
    , m_introTimer(0.0f)
    , m_bossActivated(false)
    , m_isClearUIShown(false)
    , m_bossObject(nullptr)
    , m_chaseStarted(false)
    , m_gameClearUI(nullptr)
{
}

BossSpiderQueenScene::~BossSpiderQueenScene()
{
}

void BossSpiderQueenScene::Init(const MapData* mapData)
{
    // 부모 클래스의 Init(mapData) 호출하여 맵 데이터 기반 오브젝트들 생성
    GameScene::Init(mapData);

    m_currentPhase = BossPhase::Phase1_Minions;
    m_phaseTimer = 0.0f;
    m_bossActivated = false;
    m_isIntroRunning = false;
    m_isClearUIShown = false;
    m_chaseStarted = false;
    m_bossObject = nullptr;
    m_minionObjects.clear();
    m_bossHPUI = nullptr;
    m_gameClearUI = nullptr;

    ObjectManager* uiMgr = ObjectManager::GetInstance();
    
    // 게임 클리어 UI 생성
    m_gameClearUI = new GameClearUI();
    m_gameClearUI->Init();
    uiMgr->AddGameObject(m_gameClearUI);

    ObjectManager* objMgr = ObjectManager::GetInstance();
    const auto& objects = objMgr->GetGameObjects();
    
    for (auto* obj : objects)
    {
        if (!obj) continue;

        // 보스(거미여왕) 찾기 및 비활성화
        if (obj->GetID() == GOID_MONSTER_QUEEN_SPIDER)
        {
            m_bossObject = obj;
            obj->SetActive(false);
        }
        // 일반 거미들 찾기
        else if (obj->GetID() == GOID_MONSTER_SPIDER || obj->GetID() == GOID_MONSTER_WARRIOR_SPIDER)
        {
            m_minionObjects.push_back(obj);
            Monster* pMinion = dynamic_cast<Monster*>(obj);
            Spider* pSpider = dynamic_cast<Spider*>(obj);
            if (pMinion) pMinion->SetCanChase(pSpider && !pSpider->HasHomeEgg() ? true : false);
        }
        // 나무 오브젝트 상호작용 비활성화
        else if (dynamic_cast<Tree*>(obj))
        {
            obj->SetInteractive(false);
        }
    }

    // 보스 HP UI 미리 생성
    if (m_bossObject)
    {
        m_bossHPUI = new HPUI(dynamic_cast<Entity*>(m_bossObject), m_bossName, 600.0f, 25.0f,
            Gdiplus::Color(200, 40, 0, 0), Gdiplus::Color(255, 200, 0, 0), Gdiplus::Color(255, 255, 255, 255),
            0.5f, 0.0f, 0.5f, 0.0f, 0.0f, 60.0f,
            1000.0f, 1002.0f, false, false);
        m_bossHPUI->SetActive(false);
        objMgr->AddGameObject(m_bossHPUI);
    }

    OutputDebugStringW(L"BossSpiderQueenScene: Initialized. Trees disabled.\n");
}

void BossSpiderQueenScene::Update(float deltaTime)
{
    GameScene::Update(deltaTime);

    switch (m_currentPhase)
    {
    case BossPhase::Phase1_Minions:
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

void BossSpiderQueenScene::Render()
{
    GameScene::Render();
}

void BossSpiderQueenScene::Release()
{
    // 보스 관련 포인터 정리
    m_bossObject = nullptr;
    m_minionObjects.clear();
    
    // UI 포인터 정리 (실제 삭제는 ObjectManager에서 수행됨)
    m_bossHPUI = nullptr;
    m_gameClearUI = nullptr;

    // 부모 클래스의 Release 호출 (매니저들 및 공통 UI 정리)
    GameScene::Release();
}

void BossSpiderQueenScene::UpdatePhase1(float deltaTime)
{
    ObjectManager* objMgr = ObjectManager::GetInstance();
    const auto& objects = objMgr->GetGameObjects();
    
    bool minionsAlive = false;
    bool eggsExist = false;

    for (auto* obj : objects)
    {
        if (!obj || !obj->IsEnabled()) continue;

        GameObjectID id = obj->GetID();
        
        // 거미류 체크 (죽는 애니메이션 중인 객체는 제외)
        if (id == GOID_MONSTER_SPIDER || id == GOID_MONSTER_WARRIOR_SPIDER)
        {
            Monster* monster = dynamic_cast<Monster*>(obj);
            if (monster && !monster->IsDead())
            {
                minionsAlive = true;
            }
        }
        // 거미알류 체크 (파괴된 객체는 제외)
        else if (id == GOID_BUILDING_SPIDER_SMALLEGG || id == GOID_BUILDING_SPIDER_NORMALEGG || 
                 id == GOID_BUILDING_SPIDER_TALLEGG || id == GOID_BUILDING_SPIDER_SACEGG)
        {
            SpiderEgg* egg = dynamic_cast<SpiderEgg*>(obj);
            if (egg && egg->GetHp() > 0)
            {
                eggsExist = true;
            }
        }

        if (minionsAlive && eggsExist) break;
    }

    if (!minionsAlive && !eggsExist)
    {
        m_currentPhase = BossPhase::PhaseTransition;
        m_phaseTimer = 0.0f;
        OutputDebugStringW(L"BossSpiderQueenScene: Phase 1 Cleared (All Spiders and Eggs destroyed). Transitioning to Boss Intro...\n");
    }
}

void BossSpiderQueenScene::UpdatePhaseTransition(float deltaTime)
{
    m_phaseTimer += deltaTime;
    
    // 2초 대기 후 보스 등장
    if (m_phaseTimer >= 2.0f)
    {
        StartBossIntro();
    }
}

void BossSpiderQueenScene::StartBossIntro()
{
    m_currentPhase = BossPhase::Phase2_BossIntro;
    m_introTimer = 0.0f;
    m_isIntroRunning = true;

    CameraManager* camMgr = CameraManager::GetInstance();
    camMgr->SetFollowMode(false);
    m_introStartPos = camMgr->GetCameraPos();

    Player* player = ObjectManager::GetInstance()->GetPlayer();
    if (player) player->SetInputEnabled(false);

    if (m_bossObject)
    {
        Transform* tr = m_bossObject->GetComponent<Transform>();
        if (tr) m_introTargetPos = { tr->GetX(), tr->GetY() };
    }
    else
    {
        m_introTargetPos = m_introStartPos;
    }

    OutputDebugStringW(L"BossSpiderQueenScene: Boss Intro Started.\n");
}

void BossSpiderQueenScene::UpdatePhase2Intro(float deltaTime)
{
    m_introTimer += deltaTime;
    CameraManager* camMgr = CameraManager::GetInstance();

    float moveDuration = 1.0f;
    float waitDuration = 2.0f; // Taunt 애니메이션 시간 고려
    float returnDuration = 1.5f;

	float totalBossIntroDuration = moveDuration + waitDuration;

    if (m_introTimer <= totalBossIntroDuration)
    {
        // 보스에게 이동
        float t = m_introTimer / moveDuration;
        float smoothT = t * t * (3 - 2 * t);
        float curX = m_introStartPos.X + (m_introTargetPos.X - m_introStartPos.X) * smoothT;
        float curY = m_introStartPos.Y + (m_introTargetPos.Y - m_introStartPos.Y) * smoothT;
        camMgr->SetCameraPos(curX, curY);
    }
    else if (m_introTimer <= moveDuration + waitDuration + returnDuration)
    {
        // 플레이어에게 복귀
        Player* player = ObjectManager::GetInstance()->GetPlayer();
        if (player)
        {
            Transform* tr = player->GetComponent<Transform>();

            float t = (m_introTimer - (moveDuration + waitDuration)) / returnDuration;
            float smoothT = t * t * (3 - 2 * t);
            float curX = m_introTargetPos.X + (tr->GetX() - m_introTargetPos.X) * smoothT;
            float curY = m_introTargetPos.Y + (tr->GetY() - m_introTargetPos.Y) * smoothT;
            camMgr->SetCameraPos(curX, curY);
        }

        // 복귀 도중 추격 시작
        if (!m_chaseStarted)
        {
            ObjectManager* objMgr = ObjectManager::GetInstance();
            const auto& objects = objMgr->GetGameObjects();
            for (auto* obj : objects)
            {
                if (obj && (obj->GetID() == GOID_MONSTER_SPIDER || obj->GetID() == GOID_MONSTER_WARRIOR_SPIDER))
                {
                    Monster* pMinion = dynamic_cast<Monster*>(obj);
                    if (pMinion) pMinion->SetCanChase(true);
                }
            }
            m_chaseStarted = true;
        }
    }
    else
    {
        // 연출 종료
        camMgr->SetFollowMode(true);
		Player* player = ObjectManager::GetInstance()->GetPlayer();
		if (player) player->SetInputEnabled(true);

		if (m_bossObject)
		{
			Monster* pBoss = dynamic_cast<Monster*>(m_bossObject);
			if (pBoss) pBoss->SetCanChase(true);
		}

        m_currentPhase = BossPhase::Phase2_BossBattle;

        if (m_bossHPUI) m_bossHPUI->SetActive(true); 

        OutputDebugStringW(L"BossSpiderQueenScene: Boss Battle Started.\n");
    }
}

void BossSpiderQueenScene::UpdatePhase2Battle(float deltaTime)
{
    // 보스가 죽었는지 체크
    if (m_bossObject && !m_bossObject->IsEnabled())
    {
        m_currentPhase = BossPhase::Cleared;
        m_phaseTimer = 0.0f;
        m_isClearUIShown = false;
        
        if (m_bossHPUI) m_bossHPUI->SetActive(false);

        OutputDebugStringW(L"BossSpiderQueenScene: Spider Queen Defeated!\n");
        GameProgressManager::GetInstance()->ClearScene(SCENE_GAME_SPIDER_QUEEN_HOUSE);
    }
}

void BossSpiderQueenScene::UpdateCleared(float deltaTime)
{
    m_phaseTimer += deltaTime;

    // 보스 처치 후 약 1.5초 뒤에 게임 클리어 UI 표시
    if (m_phaseTimer >= 1.5f && !m_isClearUIShown)
    {
        if (m_gameClearUI)
        {
            m_gameClearUI->Show();
        }

        m_isClearUIShown = true;
    }
}

bool BossSpiderQueenScene::IsIntroReturning() const
{
    float moveDuration = 1.0f;
    float waitDuration = 2.0f;
    return (m_currentPhase == BossPhase::Phase2_BossIntro && m_introTimer > (moveDuration + waitDuration));
}

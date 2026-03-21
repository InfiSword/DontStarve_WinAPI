#include "99_Default/pch.h"
#include "BossSpiderQueenScene.h"
#include "../ObjectManager/ObjectManager.h"
#include "../CameraManager/CameraManager.h"
#include "../UIManager/UIManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/Entity/Monster/Monster.h"
#include "../../02_GameObject/Entity/Monster/Spider.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../GameProgressManager/GameProgressManager.h"
#include "../SceneManager/SceneManager.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIText.h"
#include "../../02_GameObject/Entity/Enviorment/Tree.h"
#include "../../03_Animation/Animator.h"

BossSpiderQueenScene::BossSpiderQueenScene()
    : GameScene()
    , m_currentPhase(BossPhase::Phase1_Minions)
    , m_phaseTimer(0.0f)
    , m_isIntroRunning(false)
    , m_introTimer(0.0f)
    , m_bossActivated(false)
    , m_isClearUIShown(false)
    , m_bossObject(nullptr)
    , m_chaseStarted(false)
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

void BossSpiderQueenScene::UpdatePhase1(float deltaTime)
{
    // 필드에 있는 모든 일반 거미가 죽었는지 체크
    ObjectManager* objMgr = ObjectManager::GetInstance();
    const auto& objects = objMgr->GetGameObjects();
    
    bool minionsAlive = false;
    for (auto* obj : objects)
    {
        if (obj && (obj->GetID() == GOID_MONSTER_SPIDER || obj->GetID() == GOID_MONSTER_WARRIOR_SPIDER) && obj->IsEnabled())
        {
            minionsAlive = true;
            break;
        }
    }

    if (!minionsAlive)
    {
        m_currentPhase = BossPhase::PhaseTransition;
        m_phaseTimer = 0.0f;
        OutputDebugStringW(L"BossSpiderQueenScene: Phase 1 Cleared. Transitioning to Boss Intro...\n");
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

    if (m_introTimer <= moveDuration)
    {
        // 보스에게 이동
        float t = m_introTimer / moveDuration;
        float smoothT = t * t * (3 - 2 * t);
        float curX = m_introStartPos.X + (m_introTargetPos.X - m_introStartPos.X) * smoothT;
        float curY = m_introStartPos.Y + (m_introTargetPos.Y - m_introStartPos.Y) * smoothT;
        camMgr->SetCameraPos(curX, curY);
    }
    else if (m_introTimer <= moveDuration + waitDuration)
    {
        // 보스 활성화 및 Taunt 애니메이션
        if (m_bossObject && !m_bossObject->IsEnabled())
        {
            m_bossObject->SetActive(true);
            camMgr->SetCameraPos(m_introTargetPos.X, m_introTargetPos.Y);
            
            // 보스 Taunt 애니메이션 강제 설정 (상태값이 TAUNT인 경우)
            // Boss_SpiderQueen의 SpiderQueenState::TAUNT는 5번임 (Boss_SpiderQueen.h 참고)
            Animator* anim = m_bossObject->GetComponent<Animator>();
            if (anim) anim->SetState(5, m_bossObject->GetComponent<Transform>()->GetDirection());
            
            OutputDebugStringW(L"BossSpiderQueenScene: Spider Queen Activated!\n");
        }
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

        // 복귀 도중 추격 시작 (유저 요청: "이 때 주변에 다른 거미들도 동시에 플레이어를 추격을 시작")
        if (!m_chaseStarted)
        {
            if (m_bossObject)
            {
                Monster* pBoss = dynamic_cast<Monster*>(m_bossObject);
                if (pBoss) pBoss->SetCanChase(true);
            }

            // 주변 모든 거미(민ion)들도 추격 시작
            ObjectManager* objMgr = ObjectManager::GetInstance();
            const auto& objects = objMgr->GetGameObjects();
            for (auto* obj : objects)
            {
                if (obj && (obj->GetID() == GOID_MONSTER_SPIDER || obj->GetID() == GOID_MONSTER_WARRIOR_SPIDER))
                {
                    Monster* pMinion = dynamic_cast<Monster*>(obj);
                    if (pMinion)
                    {
                        pMinion->SetCanChase(true);
                        // 즉시 플레이어 타겟팅 (Spider::SetAggroTarget 등의 기능이 있다면 사용)
                        // 여기서는 Monster 수준에서 타겟 설정 로직이 AIUpdate에 포함되어 있다고 가정
                    }
                }
            }
            m_chaseStarted = true;
        }
    }
    else
    {
        // 연출 종료
        camMgr->SetFollowMode(true);
        m_currentPhase = BossPhase::Phase2_BossBattle;

        Player* player = ObjectManager::GetInstance()->GetPlayer();
        if (player) player->SetInputEnabled(true);

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
        OutputDebugStringW(L"BossSpiderQueenScene: Spider Queen Defeated!\n");

        // 클리어 기록 및 캐릭터 해금
        GameProgressManager::GetInstance()->ClearScene(SCENE_GAME_SPIDER_QUEEN_HOUSE);
    }
}

void BossSpiderQueenScene::UpdateCleared(float deltaTime)
{
    m_phaseTimer += deltaTime;

    if (m_phaseTimer >= 1.0f && !m_isClearUIShown)
    {
        UIManager* uiMgr = UIManager::GetInstance();
        
        UIImage* clearBanner = new UIImage(GOID_NONE, 600.0f, 150.0f, LAYER_UI_BACKGROUND, L"Resource/UI/BG_Banner.png", 999.0f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, -50.0f);
        uiMgr->AddUIImage(clearBanner);

        UIText* clearText = new UIText(GOID_NONE, 400.0f, 100.0f, L"CLEARED!", Gdiplus::Color(255, 255, 255, 0), LAYER_UI_FOREGROUND, 1000.0f, L"Arial", 48.0f, Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, -50.0f);
        uiMgr->AddUIText(clearText);

        m_isClearUIShown = true;
    }

    if (m_phaseTimer >= 4.0f)
    {
        SceneManager::GetInstance()->LoadGameScene(SCENE_GAME_FARMING_AREA, GetSelectedCharacterID());
    }
}

bool BossSpiderQueenScene::IsIntroReturning() const
{
    float moveDuration = 1.0f;
    float waitDuration = 2.0f;
    return (m_currentPhase == BossPhase::Phase2_BossIntro && m_introTimer > (moveDuration + waitDuration));
}

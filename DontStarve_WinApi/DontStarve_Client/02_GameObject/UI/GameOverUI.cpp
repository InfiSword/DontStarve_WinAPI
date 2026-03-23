#include "99_Default/pch.h"
#include "GameOverUI.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../01_Manager/SceneManager/SceneManager.h"
#include "../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../Component/Transform/RectTransform.h"
#include "../UI/UIText.h"
#include "../UI/UIButton.h"
#include "../Entity/Player/Player.h"

const float GameOverUI::SORT_KEY = 100.0f;

GameOverUI::GameOverUI(float anchorMinX, float anchorMinY,
                       float anchorMaxX, float anchorMaxY,
                       float anchoredPosX, float anchoredPosY)
    : UIElement(GOID_UI_GAME_OVER, L"", L"", true, false)
    , m_gameOverText(nullptr)
    , m_btnToLobby(nullptr)
    , m_btnToLobbyText(nullptr)
    , m_btnQuit(nullptr)
    , m_btnQuitText(nullptr)
{
    if (m_rectTransform) {
        m_rectTransform->SetAnchorMin(anchorMinX, anchorMinY);
        m_rectTransform->SetAnchorMax(anchorMaxX, anchorMaxY);
        m_rectTransform->SetAnchoredPosition(anchoredPosX, anchoredPosY);
        m_rectTransform->SetPivot(0.5f, 0.5f);
    }
    SetActive(false);
}

GameOverUI::~GameOverUI()
{
    Release();
}

void GameOverUI::Init()
{
    UIElement::Init();

    ResourceManager* pRM = ResourceManager::GetInstance();
    if (!pRM) return;

    std::shared_ptr<Sprite> btnSprite = pRM->LoadSprite(L"Resource/UI/frontscreen.png");
    std::shared_ptr<Sprite> btnHoverSprite = pRM->LoadSprite(L"Resource/UI/HighLight_frontscreen.png");
    if (!btnSprite.get()) btnSprite = pRM->LoadSprite(L"Resource\\UI\\slot.png");
    if (!btnHoverSprite.get()) btnHoverSprite = btnSprite;

    ObjectManager* objManager = ObjectManager::GetInstance();

    float centerX = m_rectTransform->GetAnchoredPosition().X;
    float centerY = m_rectTransform->GetAnchoredPosition().Y;
    float anchorX = m_rectTransform->GetAnchorMin().X;
    float anchorY = m_rectTransform->GetAnchorMin().Y;

    // 패배 텍스트
    m_gameOverText = new UIText(
        GOID_NONE, 400.0f, 80.0f, L"Game Over", Gdiplus::Color(255, 255, 255, 255),
        LAYER_UI_FOREGROUND, SORT_KEY + 1.0f, L"Arial", 48.0f,
        Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
        anchorX, anchorY, anchorX, anchorY, centerX, centerY - 80.0f
    );
    m_gameOverText->Init();
    m_gameOverText->SetActive(false);
    objManager->AddGameObject(m_gameOverText);

    // Title 버튼
    m_btnToLobby = new UIButton(
        GOID_NONE, 200.0f, 50.0f, btnSprite, btnHoverSprite,
        anchorX, anchorY, anchorX, anchorY, centerX, centerY + 20.0f
    );
    m_btnToLobby->SetOnClickCallback([]() {
        // 플레이어 사망 시 게임 데이터 초기화
        ObjectManager* objMgr = ObjectManager::GetInstance();
        Player* player = objMgr->GetPlayer();
        if (player) {
            InventoryManager::GetInstance()->ResetPlayerInventory(player);
        }
        GameProgressManager::GetInstance()->ResetRuntimeData();
        SceneManager::GetInstance()->LoadTitleScene();
    });
    m_btnToLobby->Init();
    m_btnToLobby->SetSortKey(SORT_KEY + 2.0f);
    m_btnToLobby->SetActive(false);
    objManager->AddGameObject(m_btnToLobby);

    m_btnToLobbyText = new UIText(
        GOID_NONE, 200.0f, 50.0f, L"Title", Gdiplus::Color(255, 255, 255, 255),
        LAYER_UI_FOREGROUND, SORT_KEY + 3.0f, L"Arial", 24.0f,
        Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
        anchorX, anchorY, anchorX, anchorY, centerX, centerY + 20.0f
    );
    m_btnToLobbyText->Init();
    m_btnToLobbyText->SetActive(false);
    objManager->AddGameObject(m_btnToLobbyText);

    // Exit 버튼
    m_btnQuit = new UIButton(
        GOID_NONE, 200.0f, 50.0f, btnSprite, btnHoverSprite,
        anchorX, anchorY, anchorX, anchorY, centerX, centerY + 85.0f
    );
    m_btnQuit->SetOnClickCallback([]() {
        PostQuitMessage(0);
    });
    m_btnQuit->Init();
    m_btnQuit->SetSortKey(SORT_KEY + 2.0f);
    m_btnQuit->SetActive(false);
    objManager->AddGameObject(m_btnQuit);

    m_btnQuitText = new UIText(
        GOID_NONE, 200.0f, 50.0f, L"Exit", Gdiplus::Color(255, 255, 255, 255),
        LAYER_UI_FOREGROUND, SORT_KEY + 3.0f, L"Arial", 24.0f,
        Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
        anchorX, anchorY, anchorX, anchorY, centerX, centerY + 85.0f
    );
    m_btnQuitText->Init();
    m_btnQuitText->SetActive(false);
    objManager->AddGameObject(m_btnQuitText);
}

void GameOverUI::Update(float deltaTime)
{
    UIElement::Update(deltaTime);

    // 자식 요소들의 활성화 상태 동기화 (HPUI 방식)
    bool isEnabled = IsEnabled();
    if (m_gameOverText && m_gameOverText->IsEnabled() != isEnabled) m_gameOverText->SetActive(isEnabled);
    if (m_btnToLobby && m_btnToLobby->IsEnabled() != isEnabled) m_btnToLobby->SetActive(isEnabled);
    if (m_btnToLobbyText && m_btnToLobbyText->IsEnabled() != isEnabled) m_btnToLobbyText->SetActive(isEnabled);
    if (m_btnQuit && m_btnQuit->IsEnabled() != isEnabled) m_btnQuit->SetActive(isEnabled);
    if (m_btnQuitText && m_btnQuitText->IsEnabled() != isEnabled) m_btnQuitText->SetActive(isEnabled);

    Player* player = ObjectManager::GetInstance() ? ObjectManager::GetInstance()->GetPlayer() : nullptr;
    
    // 플레이어가 죽으면 자동으로 표시
    if (player && player->IsDead() && !isEnabled) {
        Show();
    }
}

void GameOverUI::Render()
{
    if (!IsEnabled()) return;

    RenderManager* pRM = RenderManager::GetInstance();
    if (!pRM) return;

    // 전체 화면 어둡게 블록 (다른 UI 요소들보다 아래에 위치하도록 SORT_KEY 조정)
    float screenW = static_cast<float>(WINCX);
    float screenH = static_cast<float>(WINCY);
    Gdiplus::RectF blockRect(0.0f, 0.0f, screenW, screenH);
    // SORT_KEY가 100.0f이므로, 99.0f 정도로 설정하여 자식 UI들(101.0f 이상)보다 뒤에 오도록 함
    pRM->AddFillRectangleCommand(blockRect, Gdiplus::Color(150, 0, 0, 0), LAYER_UI_FOREGROUND, SORT_KEY - 1.0f);
}

void GameOverUI::Show()
{
    if (IsEnabled()) return;
    SetActive(true);
}

void GameOverUI::Hide()
{
    if (!IsEnabled()) return;
    SetActive(false);
}

void GameOverUI::Release()
{
    Hide();

    ObjectManager* objManager = ObjectManager::GetInstance();
    if (!objManager) return;

    if (m_gameOverText) { 
        objManager->RemoveGameObject(m_gameOverText);
        m_gameOverText = nullptr; 
    }
    if (m_btnToLobby) { 
        objManager->RemoveGameObject(m_btnToLobby);
        m_btnToLobby = nullptr; 
    }
    if (m_btnToLobbyText) { 
        objManager->RemoveGameObject(m_btnToLobbyText);
        m_btnToLobbyText = nullptr; 
    }
    if (m_btnQuit) { 
        objManager->RemoveGameObject(m_btnQuit);
        m_btnQuit = nullptr; 
    }
    if (m_btnQuitText) { 
        objManager->RemoveGameObject(m_btnQuitText);
        m_btnQuitText = nullptr; 
    }
    
    UIElement::Release();
}

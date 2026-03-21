#include "99_Default/pch.h"
#include "GameOverUI.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../01_Manager/UIManager/UIManager.h"
#include "../../01_Manager/SceneManager/SceneManager.h"
#include "../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../UI/UIText.h"
#include "../UI/UIButton.h"
#include "../Entity/Player/Player.h"

const float GameOverUI::SORT_KEY = 100.0f;

GameOverUI::GameOverUI()
    : m_gameOverText(nullptr)
    , m_btnToLobby(nullptr)
    , m_btnToLobbyText(nullptr)
    , m_btnQuit(nullptr)
    , m_btnQuitText(nullptr)
    , m_isVisible(false)
{
}

GameOverUI::~GameOverUI()
{
    Release();
}

void GameOverUI::Init()
{
    ResourceManager* pRM = ResourceManager::GetInstance();
    if (!pRM) return;

    std::shared_ptr<Sprite> btnSprite = pRM->LoadSprite(L"Resource/UI/frontscreen.png");
    std::shared_ptr<Sprite> btnHoverSprite = pRM->LoadSprite(L"Resource/UI/HighLight_frontscreen.png");
    if (!btnSprite.get()) btnSprite = pRM->LoadSprite(L"Resource\\UI\\slot.png");
    if (!btnHoverSprite.get()) btnHoverSprite = btnSprite;

    // 패배 텍스트
    m_gameOverText = new UIText(
        GOID_NONE, 400.0f, 80.0f, L"Game Over", Gdiplus::Color(255, 255, 255, 255),
        LAYER_UI_FOREGROUND, SORT_KEY + 1.0f, L"Arial", 48.0f,
        Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
        0.5f, 0.5f, 0.5f, 0.5f, 0.0f, -80.0f
    );
    m_gameOverText->Init();
    m_gameOverText->SetActive(false);

    // Title 버튼
    m_btnToLobby = new UIButton(
        GOID_NONE, 200.0f, 50.0f, btnSprite, btnHoverSprite,
        0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 20.0f
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

    m_btnToLobbyText = new UIText(
        GOID_NONE, 200.0f, 50.0f, L"Title", Gdiplus::Color(255, 255, 255, 255),
        LAYER_UI_FOREGROUND, SORT_KEY + 3.0f, L"Arial", 24.0f,
        Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
        0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 20.0f
    );
    m_btnToLobbyText->Init();
    m_btnToLobbyText->SetActive(false);

    // Exit 버튼
    m_btnQuit = new UIButton(
        GOID_NONE, 200.0f, 50.0f, btnSprite, btnHoverSprite,
        0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 85.0f
    );
    m_btnQuit->SetOnClickCallback([]() {
        PostQuitMessage(0);
    });
    m_btnQuit->Init();
    m_btnQuit->SetSortKey(SORT_KEY + 2.0f);
    m_btnQuit->SetActive(false);

    m_btnQuitText = new UIText(
        GOID_NONE, 200.0f, 50.0f, L"Exit", Gdiplus::Color(255, 255, 255, 255),
        LAYER_UI_FOREGROUND, SORT_KEY + 3.0f, L"Arial", 24.0f,
        Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
        0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 85.0f
    );
    m_btnQuitText->Init();
    m_btnQuitText->SetActive(false);
}

void GameOverUI::Update(float deltaTime)
{
    Player* player = ObjectManager::GetInstance() ? ObjectManager::GetInstance()->GetPlayer() : nullptr;
    
    // 플레이어가 죽으면 자동으로 표시
    if (player && player->IsDead() && !m_isVisible) {
        Show();
    }
}

void GameOverUI::Render()
{
    if (!m_isVisible) return;

    RenderManager* pRM = RenderManager::GetInstance();
    if (!pRM) return;

    // 전체 화면 어둡게 블록
    float screenW = static_cast<float>(WINCX);
    float screenH = static_cast<float>(WINCY);
    Gdiplus::RectF blockRect(0.0f, 0.0f, screenW, screenH);
    pRM->AddFillRectangleCommand(blockRect, Gdiplus::Color(150, 0, 0, 0), LAYER_UI_FOREGROUND, SORT_KEY + 10.0f);
}

void GameOverUI::Show()
{
    if (m_isVisible) return;
    UIManager* uiManager = UIManager::GetInstance();
    if (!uiManager) return;

    m_isVisible = true;
    m_gameOverText->SetActive(true);
    m_btnToLobby->SetActive(true);
    m_btnToLobbyText->SetActive(true);
    m_btnQuit->SetActive(true);
    m_btnQuitText->SetActive(true);

    uiManager->AddUIText(m_gameOverText);
    uiManager->AddUIButton(m_btnToLobby);
    uiManager->AddUIText(m_btnToLobbyText);
    uiManager->AddUIButton(m_btnQuit);
    uiManager->AddUIText(m_btnQuitText);
}

void GameOverUI::Hide()
{
    if (!m_isVisible) return;
    UIManager* uiManager = UIManager::GetInstance();
    if (!uiManager) return;

    m_isVisible = false;
    m_gameOverText->SetActive(false);
    m_btnToLobby->SetActive(false);
    m_btnToLobbyText->SetActive(false);
    m_btnQuit->SetActive(false);
    m_btnQuitText->SetActive(false);

    uiManager->RemoveUIText(m_gameOverText);
    uiManager->RemoveUIButton(m_btnToLobby);
    uiManager->RemoveUIText(m_btnToLobbyText);
    uiManager->RemoveUIButton(m_btnQuit);
    uiManager->RemoveUIText(m_btnQuitText);
}

void GameOverUI::Release()
{
    Hide();

    if (m_gameOverText) { m_gameOverText->Release(); delete m_gameOverText; m_gameOverText = nullptr; }
    if (m_btnToLobby) { m_btnToLobby->Release(); delete m_btnToLobby; m_btnToLobby = nullptr; }
    if (m_btnToLobbyText) { m_btnToLobbyText->Release(); delete m_btnToLobbyText; m_btnToLobbyText = nullptr; }
    if (m_btnQuit) { m_btnQuit->Release(); delete m_btnQuit; m_btnQuit = nullptr; }
    if (m_btnQuitText) { m_btnQuitText->Release(); delete m_btnQuitText; m_btnQuitText = nullptr; }
}

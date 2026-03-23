#include "99_Default/pch.h"
#include "GameClearUI.h"
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
#include <iomanip>
#include <sstream>

const float GameClearUI::SORT_KEY = 100.0f;

GameClearUI::GameClearUI(float anchorMinX, float anchorMinY,
                         float anchorMaxX, float anchorMaxY,
                         float anchoredPosX, float anchoredPosY)
    : UIElement(GOID_UI_GAME_CLEAR, L"", L"", true, false)
    , m_clearText(nullptr)
    , m_timeText(nullptr)
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

GameClearUI::~GameClearUI()
{
    Release();
}

void GameClearUI::Init()
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

    // 클리어 텍스트
    m_clearText = new UIText(
        GOID_NONE, 400.0f, 80.0f, L"GAME CLEAR!", Gdiplus::Color(255, 255, 215, 0), // Gold color
        LAYER_UI_FOREGROUND, SORT_KEY + 1.0f, L"Arial", 48.0f,
        Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
        anchorX, anchorY, anchorX, anchorY, centerX, centerY - 120.0f
    );
    m_clearText->Init();
    m_clearText->SetActive(false);
    objManager->AddGameObject(m_clearText);

    // 시간 텍스트
    m_timeText = new UIText(
        GOID_NONE, 400.0f, 50.0f, L"Clear Time: 00:00", Gdiplus::Color(255, 255, 255, 255),
        LAYER_UI_FOREGROUND, SORT_KEY + 1.0f, L"Arial", 24.0f,
        Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
        anchorX, anchorY, anchorX, anchorY, centerX, centerY - 60.0f
    );
    m_timeText->Init();
    m_timeText->SetActive(false);
    objManager->AddGameObject(m_timeText);

    // Title 버튼
    m_btnToLobby = new UIButton(
        GOID_NONE, 200.0f, 50.0f, btnSprite, btnHoverSprite,
        anchorX, anchorY, anchorX, anchorY, centerX, centerY + 20.0f
    );
    m_btnToLobby->SetOnClickCallback([]() {
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

void GameClearUI::Update(float deltaTime)
{
    UIElement::Update(deltaTime);

    bool isEnabled = IsEnabled();
    if (m_clearText && m_clearText->IsEnabled() != isEnabled) m_clearText->SetActive(isEnabled);
    if (m_timeText && m_timeText->IsEnabled() != isEnabled) m_timeText->SetActive(isEnabled);
    if (m_btnToLobby && m_btnToLobby->IsEnabled() != isEnabled) m_btnToLobby->SetActive(isEnabled);
    if (m_btnToLobbyText && m_btnToLobbyText->IsEnabled() != isEnabled) m_btnToLobbyText->SetActive(isEnabled);
    if (m_btnQuit && m_btnQuit->IsEnabled() != isEnabled) m_btnQuit->SetActive(isEnabled);
    if (m_btnQuitText && m_btnQuitText->IsEnabled() != isEnabled) m_btnQuitText->SetActive(isEnabled);
}

void GameClearUI::Render()
{
    if (!IsEnabled()) return;

    RenderManager* pRM = RenderManager::GetInstance();
    if (!pRM) return;

    // 전체 화면 어둡게 블록
    float screenW = static_cast<float>(WINCX);
    float screenH = static_cast<float>(WINCY);
    Gdiplus::RectF blockRect(0.0f, 0.0f, screenW, screenH);
    pRM->AddFillRectangleCommand(blockRect, Gdiplus::Color(180, 0, 0, 0), LAYER_UI_FOREGROUND, SORT_KEY - 1.0f);
}

void GameClearUI::Show()
{
    if (IsEnabled()) return;
    
    // 시간 업데이트
    if (m_timeText) {
        float totalTime = GameProgressManager::GetInstance()->GetTotalGameTime();
        m_timeText->SetText(L"Clear Time: " + FormatTime(totalTime));
    }

    SetActive(true);
}

void GameClearUI::Hide()
{
    if (!IsEnabled()) return;
    SetActive(false);
}

void GameClearUI::Release()
{
    Hide();

    ObjectManager* objManager = ObjectManager::GetInstance();
    if (!objManager) return;

    if (m_clearText) { objManager->RemoveGameObject(m_clearText); m_clearText = nullptr; }
    if (m_timeText) { objManager->RemoveGameObject(m_timeText); m_timeText = nullptr; }
    if (m_btnToLobby) { objManager->RemoveGameObject(m_btnToLobby); m_btnToLobby = nullptr; }
    if (m_btnToLobbyText) { objManager->RemoveGameObject(m_btnToLobbyText); m_btnToLobbyText = nullptr; }
    if (m_btnQuit) { objManager->RemoveGameObject(m_btnQuit); m_btnQuit = nullptr; }
    if (m_btnQuitText) { objManager->RemoveGameObject(m_btnQuitText); m_btnQuitText = nullptr; }
    
    UIElement::Release();
}

std::wstring GameClearUI::FormatTime(float totalSeconds)
{
    int hours = static_cast<int>(totalSeconds) / 3600;
    int minutes = (static_cast<int>(totalSeconds) % 3600) / 60;
    int seconds = static_cast<int>(totalSeconds) % 60;

    std::wstringstream wss;
    if (hours > 0) {
        wss << std::setfill(L'0') << std::setw(2) << hours << L":";
    }
    wss << std::setfill(L'0') << std::setw(2) << minutes << L":"
        << std::setfill(L'0') << std::setw(2) << seconds;

    return wss.str();
}

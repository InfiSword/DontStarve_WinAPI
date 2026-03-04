#include "99_Default/pch.h"
#include "PlayerHPUI.h"
#include "../Entity/Player/Player.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../01_Manager/UIManager/UIManager.h"
#include "../../01_Manager/SceneManager/SceneManager.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Sprite.h"
#include "../UI/UIText.h"
#include "../UI/UIButton.h"
#include "../UI/UIImage.h"

const float PlayerHPUI::MARGIN = 20.0f;
const float PlayerHPUI::BAR_WIDTH = 200.0f;
const float PlayerHPUI::BAR_HEIGHT = 28.0f;
const float PlayerHPUI::ICON_SIZE = 48.0f;
const float PlayerHPUI::GAP = 10.0f;
const float PlayerHPUI::GAME_OVER_SORT_KEY = 100.0f;

PlayerHPUI::PlayerHPUI()
	: m_hpText(nullptr)
	, m_gameOverText(nullptr)
	, m_btnToLobby(nullptr)
	, m_btnQuit(nullptr)
	, m_gameOverPanelVisible(false)
	, m_lastDisplayedHp(-1)
	, m_lastDisplayedMaxHp(-1)
{
}

PlayerHPUI::~PlayerHPUI()
{
	Release();
}

void PlayerHPUI::Init()
{
	ResourceManager* pRM = ResourceManager::GetInstance();
	UIManager* uiManager = UIManager::GetInstance();
	if (!pRM || !uiManager) return;

	m_hpIconSprite = pRM->LoadSprite(L"Resource\\UI\\HP.png");

	// HP 텍스트: 게이지 바 영역 중앙, 우측 상단 앵커
	float barCenterOffsetX = MARGIN + BAR_WIDTH * 0.5f + GAP + ICON_SIZE;
	float barCenterY = MARGIN + BAR_HEIGHT * 0.5f;
	m_hpText = new UIText(
		GOID_NONE,
		BAR_WIDTH,
		BAR_HEIGHT,
		L"100/100",
		Gdiplus::Color(255, 255, 255, 255),
		LAYER_UI_FOREGROUND,
		2.0f,
		L"Arial",
		16.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		1.0f, 0.0f,  // anchorMin: top-right
		1.0f, 0.0f,  // anchorMax: top-right
		-barCenterOffsetX,
		barCenterY
	);
	m_hpText->Init();
	uiManager->AddUIText(m_hpText);

	// Game Over 패널 요소 생성 (초기에는 비활성화, 표시 시 UIManager에 추가)
	std::shared_ptr<Sprite> btnSprite = pRM->LoadSprite(L"Resource/UI/frontscreen.png");
	std::shared_ptr<Sprite> btnHoverSprite = pRM->LoadSprite(L"Resource/UI/HighLight_frontscreen.png");
	if (!btnSprite.get()) btnSprite = pRM->LoadSprite(L"Resource\\UI\\slot.png");
	if (!btnHoverSprite.get()) btnHoverSprite = btnSprite;

	m_gameOverText = new UIText(
		GOID_NONE,
		400.0f,
		80.0f,
		L"Game Over",
		Gdiplus::Color(255, 255, 255, 255),
		LAYER_UI_FOREGROUND,
		GAME_OVER_SORT_KEY + 1.0f,
		L"Arial",
		48.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.5f,
		0.5f, 0.5f,
		0.0f, -80.0f
	);
	m_gameOverText->Init();
	m_gameOverText->SetActive(false);

	m_btnToLobby = new UIButton(
		GOID_NONE,
		200.0f,
		50.0f,
		btnSprite,
		btnHoverSprite,
		0.5f, 0.5f, 0.5f, 0.5f,
		0.0f, 20.0f
	);
	m_btnToLobby->SetOnClickCallback([]() {
		SceneManager::GetInstance()->LoadCharacterSelectScene();
	});
	m_btnToLobby->Init();
	m_btnToLobby->SetSortKey(GAME_OVER_SORT_KEY + 2.0f);
	m_btnToLobby->SetActive(false);

	m_btnQuit = new UIButton(
		GOID_NONE,
		200.0f,
		50.0f,
		btnSprite,
		btnHoverSprite,
		0.5f, 0.5f, 0.5f, 0.5f,
		0.0f, 85.0f
	);
	m_btnQuit->SetOnClickCallback([]() {
		PostQuitMessage(0);
	});
	m_btnQuit->Init();
	m_btnQuit->SetSortKey(GAME_OVER_SORT_KEY + 2.0f);
	m_btnQuit->SetActive(false);
}

void PlayerHPUI::Update(float deltaTime)
{
	Player* player = ObjectManager::GetInstance() ? ObjectManager::GetInstance()->GetPlayer() : nullptr;
	if (player && m_hpText) {
		int currentHp = player->GetHp();
		int maxHp = player->GetMaxHp();
		UpdateHPDisplay(currentHp, maxHp);
		if (player->IsDead())
			ShowGameOverPanel();
		else
			HideGameOverPanel();
	}
	else {
		HideGameOverPanel();
	}
}

void PlayerHPUI::UpdateHPDisplay(int currentHp, int maxHp)
{
	if (!m_hpText) return;
	// 값이 바뀐 경우에만 SetText 호출하여 매 프레임 문자열 할당 방지
	if (currentHp == m_lastDisplayedHp && maxHp == m_lastDisplayedMaxHp)
		return;
	m_lastDisplayedHp = currentHp;
	m_lastDisplayedMaxHp = maxHp;
	std::wstring str = std::to_wstring(maxHp) + L"/" + std::to_wstring(currentHp);
	m_hpText->SetText(str);
}

void PlayerHPUI::ShowGameOverPanel()
{
	if (m_gameOverPanelVisible) return;
	UIManager* uiManager = UIManager::GetInstance();
	if (!uiManager) return;

	m_gameOverPanelVisible = true;
	m_gameOverText->SetActive(true);
	m_btnToLobby->SetActive(true);
	m_btnQuit->SetActive(true);
	uiManager->AddUIText(m_gameOverText);
	uiManager->AddUIButton(m_btnToLobby);
	uiManager->AddUIButton(m_btnQuit);
}

void PlayerHPUI::HideGameOverPanel()
{
	if (!m_gameOverPanelVisible) return;
	UIManager* uiManager = UIManager::GetInstance();
	if (!uiManager) return;

	m_gameOverPanelVisible = false;
	m_gameOverText->SetActive(false);
	m_btnToLobby->SetActive(false);
	m_btnQuit->SetActive(false);
	uiManager->RemoveUIText(m_gameOverText);
	uiManager->RemoveUIButton(m_btnToLobby);
	uiManager->RemoveUIButton(m_btnQuit);
}

void PlayerHPUI::Render()
{
	RenderManager* pRM = RenderManager::GetInstance();
	if (!pRM) return;

	float screenW = static_cast<float>(WINCX);
	float screenH = static_cast<float>(WINCY);

	// 우측 상단 기준
	float barLeft = screenW - MARGIN - BAR_WIDTH;
	float barTop = MARGIN;
	float iconCenterX = barLeft - GAP - ICON_SIZE * 0.5f;
	float iconCenterY = barTop + BAR_HEIGHT * 0.5f;

	// 1) HP 아이콘
	if (m_hpIconSprite && m_hpIconSprite->bitmap) {
		Gdiplus::Bitmap* bmp = m_hpIconSprite->bitmap.get();
		pRM->AddUIImageCommand(
			bmp,
			iconCenterX, iconCenterY,
			ICON_SIZE, ICON_SIZE,
			0.5f, 0.5f,
			LAYER_UI_FOREGROUND,
			1.0f
		);
	}

	// 2) 게이지 바 배경
	Gdiplus::RectF barRect(barLeft, barTop, BAR_WIDTH, BAR_HEIGHT);
	pRM->AddFillRectangleCommand(barRect, Gdiplus::Color(80, 40, 40, 255), LAYER_UI_FOREGROUND, 1.1f);

	// 3) 게이지 바 채움 (HP 비율)
	Player* player = ObjectManager::GetInstance() ? ObjectManager::GetInstance()->GetPlayer() : nullptr;
	float ratio = 1.0f;
	if (player) {
		int maxHp = player->GetMaxHp();
		int hp = player->GetHp();
		ratio = (maxHp > 0) ? (static_cast<float>(hp) / static_cast<float>(maxHp)) : 0.0f;
		if (ratio < 0.0f) ratio = 0.0f;
		if (ratio > 1.0f) ratio = 1.0f;
	}
	float fillWidth = BAR_WIDTH * ratio;
	if (fillWidth > 0.01f) {
		Gdiplus::RectF fillRect(barLeft, barTop, fillWidth, BAR_HEIGHT);
		pRM->AddFillRectangleCommand(fillRect, Gdiplus::Color(255, 70, 70, 255), LAYER_UI_FOREGROUND, 1.2f);
	}

	// 4) Game Over 시 전체 화면 반투명 블록
	if (m_gameOverPanelVisible) {
		Gdiplus::RectF blockRect(0.0f, 0.0f, screenW, screenH);
		pRM->AddFillRectangleCommand(blockRect, Gdiplus::Color(150, 0, 0, 0), LAYER_UI_FOREGROUND, GAME_OVER_SORT_KEY);
	}
}

void PlayerHPUI::Release()
{
	HideGameOverPanel();

	UIManager* uiManager = UIManager::GetInstance();
	if (uiManager) {
		if (m_hpText) {
			uiManager->RemoveUIText(m_hpText);
			m_hpText->Release();
			delete m_hpText;
			m_hpText = nullptr;
		}
	}
	// Game Over 요소는 HideGameOverPanel에서 이미 Remove됨
	if (m_gameOverText) {
		m_gameOverText->Release();
		delete m_gameOverText;
		m_gameOverText = nullptr;
	}
	if (m_btnToLobby) {
		m_btnToLobby->Release();
		delete m_btnToLobby;
		m_btnToLobby = nullptr;
	}
	if (m_btnQuit) {
		m_btnQuit->Release();
		delete m_btnQuit;
		m_btnQuit = nullptr;
	}

	m_hpIconSprite.reset();
	m_gameOverPanelVisible = false;
}

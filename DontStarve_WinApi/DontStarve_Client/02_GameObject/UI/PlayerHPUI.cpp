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
		11.0f,
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
}

void PlayerHPUI::Update(float deltaTime)
{
	Player* player = ObjectManager::GetInstance() ? ObjectManager::GetInstance()->GetPlayer() : nullptr;
	if (player && m_hpText) {
		int currentHp = player->GetHp();
		int maxHp = player->GetMaxHp();
		UpdateHPDisplay(currentHp, maxHp);
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
	std::wstring str = std::to_wstring(currentHp) + L"/" + std::to_wstring(maxHp);
	m_hpText->SetText(str);
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

	// 1) HP 아이콘 - 높은 SortKey (10.0f)
	if (m_hpIconSprite && m_hpIconSprite->bitmap) {
		Gdiplus::Bitmap* bmp = m_hpIconSprite->bitmap.get();
		pRM->AddUIImageCommand(
			bmp,
			iconCenterX, iconCenterY,
			ICON_SIZE, ICON_SIZE,
			0.5f, 0.5f,
			LAYER_UI_FOREGROUND,
			10.0f
		);
	}

	// 2) 게이지 바 배경 - 높은 SortKey (10.1f)
	Gdiplus::RectF barRect(barLeft, barTop, BAR_WIDTH, BAR_HEIGHT);
	pRM->AddFillRectangleCommand(barRect, Gdiplus::Color(255, 60, 0, 0), LAYER_UI_FOREGROUND, 10.1f);

	// 3) 게이지 바 채움 (HP 비율) - 높은 SortKey (10.2f), 선명한 빨간색
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
		pRM->AddFillRectangleCommand(fillRect, Gdiplus::Color(255, 255, 0, 0), LAYER_UI_FOREGROUND, 10.2f);
	}
}

void PlayerHPUI::Release()
{
	UIManager* uiManager = UIManager::GetInstance();
	if (uiManager) {
		if (m_hpText) {
			uiManager->RemoveUIText(m_hpText);
			m_hpText->Release();
			delete m_hpText;
			m_hpText = nullptr;
		}
	}

	m_hpIconSprite.reset();
}

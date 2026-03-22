#include "99_Default/pch.h"
#include "CraftingUI.h"
#include "CraftingRecipe.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../Entity/Player/Player.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UIText.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../../01_Manager/SceneManager/SceneManager.h"

MenuUI::MenuUI()
	: UIElement(GOID_CRAFT_BAR, L"", L"", true, false),
	m_menuBar(nullptr),
	m_toolPanelBg(nullptr),
	m_ingredientPanelBg(nullptr),
	m_craftIcon(nullptr),
	m_menuCreateIcon(nullptr),
	m_menuEditIcon(nullptr),
	m_menuBattleIcon(nullptr),
	m_menuCookIcon(nullptr),
	m_craftButton(nullptr),
	m_craftButtonText(nullptr),
	m_craftingItemNameText(nullptr),
	m_bossOverlay(nullptr),
	m_houndBossPanel(nullptr),
	m_spiderQueenBossPanel(nullptr),
	m_bossChallengeButton(nullptr),
	m_bossChallengeButtonText(nullptr),
	m_houndClearText(nullptr),
	m_spiderQueenClearText(nullptr),
	m_isBossPanelVisible(false),
	m_selectedBossID(GOID_NONE),
	m_isToolListVisible(false),
	m_isCreateListVisible(false),
	m_isCookListVisible(false),
	m_selectedToolID(GOID_NONE),

	m_craftBarWidth(125.0f),
	m_craftBarHeight(400.0f),
	m_iconSize(64.0f),
	m_toolButtonSize(64.0f),
	m_toolButtonSpacing(8.0f),
	m_columnsPerRow(3),
	m_craftButtonWidth(120.0f),
	m_craftButtonHeight(40.0f),
	m_ingredientImageSize(60.0f),      // 74.0f에서 60.0f로 감소
	m_ingredientSpacing(50.0f),        // 58.0f에서 50.0f로 감소
	m_ingredientStartY(0.0f),
	m_ingredientPanelCenterX(0.0f),
	m_ingredientPanelOffsetX(45.0f),
	m_ingredientPanelOffsetY(20.0f),
	m_menuIconX(40.0f),
	m_menuIconStartY(140.0f),
	m_menuIconSpacing(72.0f),
	m_menuIconY(),
	m_paletteBgOffsetX(20.0f),
	m_paletteBgOffsetY(-30.0f),
	m_toolButtonOffsetX(45.0f)
{
	m_availableTools.clear();
	for (size_t i = 0; i < CraftingRecipeCount; ++i) {
		GameObjectID id = CraftingRecipeTable[i].toolID;
		if (id >= GOID_TOOL_GOLDEN_SCYTHE && id <= GOID_TOOL_HAMMER)
			m_availableTools.push_back(id);
	}
	m_availableCreateItems = { GOID_ITEM_CUT_NORMAL_STONE, GOID_ITEM_ROPE, GOID_ITEM_WOOD_2 };
	m_availableCookItems = { GOID_ITEM_COOKED_MONSTER_MEAT, GOID_ITEM_COOKED_SMALL_MEAT, GOID_ITEM_COOKED_MEAT };

	// 파생 레이아웃 계산
	const float toolPanelOffsetX = m_craftBarWidth;

	// 팔레트 BG 하단 Y 계산 (CreatePanelBackgrounds와 동일한 수식)
	const int   maxItemCount = static_cast<int>((std::max)({
		m_availableTools.size(), m_availableCreateItems.size(), m_availableCookItems.size() }));
	const int   maxRows = (maxItemCount + m_columnsPerRow - 1) / m_columnsPerRow;
	const float palettePadding = 48.0f;
	const float toolPanelH = maxRows * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing + palettePadding * 2.0f;
	const float toolButtonStartY = -(m_craftBarHeight * 0.5f);
	const float toolPanelCY = toolButtonStartY + toolPanelH * 0.5f - palettePadding;
	// 팔레트 BG 하단 = 중심Y + 높이/2  (앵커 기준 오프셋)
	const float paletteBgBottomY = toolPanelCY + m_paletteBgOffsetY + toolPanelH * 0.5f;

	// 재료 패널 Y: 팔레트 BG 하단 기준 + 재료 이미지 반높이 + 추가 오프셋
	m_ingredientStartY = paletteBgBottomY + m_ingredientImageSize * 0.5f + m_ingredientPanelOffsetY;
	m_ingredientPanelCenterX = toolPanelOffsetX + (m_columnsPerRow * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing) * 0.5f + m_ingredientPanelOffsetX;

	// 메뉴바 아이콘 Y 배열 계산 ([0]=맨위 ~ [4]=맨아래)
	for (int i = 0; i < 5; ++i)
		m_menuIconY[i] = m_menuIconStartY - i * m_menuIconSpacing;
}

MenuUI::~MenuUI()
{
	Release();
}

void MenuUI::Init()
{
	InitializeCraftingUI();
}

void MenuUI::Update(float deltaTime)
{
	GameObject::Update(deltaTime);
}

void MenuUI::Release()
{
	ObjectManager* objManager = ObjectManager::GetInstance();

	// 생성한 UI 요소들을 ObjectManager에서 제거
	if (m_menuBar) {
		objManager->RemoveGameObject(m_menuBar);
		delete m_menuBar;
		m_menuBar = nullptr;
	}

	if (m_toolPanelBg) {
		objManager->RemoveGameObject(m_toolPanelBg);
		delete m_toolPanelBg;
		m_toolPanelBg = nullptr;
	}

	if (m_ingredientPanelBg) {
		objManager->RemoveGameObject(m_ingredientPanelBg);
		delete m_ingredientPanelBg;
		m_ingredientPanelBg = nullptr;
	}

	// 보스 UI 요소 제거
	if (m_bossOverlay) {
		objManager->RemoveGameObject(m_bossOverlay);
		delete m_bossOverlay;
		m_bossOverlay = nullptr;
	}

	if (m_houndBossPanel) {
		objManager->RemoveGameObject(m_houndBossPanel);
		delete m_houndBossPanel;
		m_houndBossPanel = nullptr;
	}

	if (m_spiderQueenBossPanel) {
		objManager->RemoveGameObject(m_spiderQueenBossPanel);
		delete m_spiderQueenBossPanel;
		m_spiderQueenBossPanel = nullptr;
	}

	if (m_houndClearText) {
		objManager->RemoveGameObject(m_houndClearText);
		delete m_houndClearText;
		m_houndClearText = nullptr;
	}

	if (m_spiderQueenClearText) {
		objManager->RemoveGameObject(m_spiderQueenClearText);
		delete m_spiderQueenClearText;
		m_spiderQueenClearText = nullptr;
	}

	if (m_bossChallengeButton) {
		objManager->RemoveGameObject(m_bossChallengeButton);
		delete m_bossChallengeButton;
		m_bossChallengeButton = nullptr;
	}

	if (m_bossChallengeButtonText) {
		objManager->RemoveGameObject(m_bossChallengeButtonText);
		delete m_bossChallengeButtonText;
		m_bossChallengeButtonText = nullptr;
	}

	if (m_craftIcon) {
		objManager->RemoveGameObject(m_craftIcon);
		delete m_craftIcon;
		m_craftIcon = nullptr;
	}

	if (m_menuCreateIcon) {
		objManager->RemoveGameObject(m_menuCreateIcon);
		delete m_menuCreateIcon;
		m_menuCreateIcon = nullptr;
	}
	if (m_menuEditIcon) {
		objManager->RemoveGameObject(m_menuEditIcon);
		delete m_menuEditIcon;
		m_menuEditIcon = nullptr;
	}
	if (m_menuBattleIcon) {
		objManager->RemoveGameObject(m_menuBattleIcon);
		delete m_menuBattleIcon;
		m_menuBattleIcon = nullptr;
	}
	if (m_menuCookIcon) {
		objManager->RemoveGameObject(m_menuCookIcon);
		delete m_menuCookIcon;
		m_menuCookIcon = nullptr;
	}

	if (m_craftButton) {
		objManager->RemoveGameObject(m_craftButton);
		delete m_craftButton;
		m_craftButton = nullptr;
	}
	if (m_craftButtonText) {
	 objManager->RemoveGameObject(m_craftButtonText);
	 delete m_craftButtonText;
	 m_craftButtonText = nullptr;
	}

	for (auto* button : m_toolButtons) {
		if (button) {
			objManager->RemoveGameObject(button);
			delete button;
		}
	}
	m_toolButtons.clear();
	for (auto* button : m_createItemButtons) {
		if (button) {
			objManager->RemoveGameObject(button);
			delete button;
		}
	}
	m_createItemButtons.clear();
	for (auto* button : m_cookItemButtons) {
		if (button) {
			objManager->RemoveGameObject(button);
			delete button;
		}
	}
	m_cookItemButtons.clear();

	for (auto* image : m_ingredientImages) {
		if (image) {
			objManager->RemoveGameObject(image);
			delete image;
		}
	}
	m_ingredientImages.clear();

	for (auto* text : m_ingredientTexts) {
		if (text) {
			objManager->RemoveGameObject(text);
			delete text;
		}
	}
	m_ingredientTexts.clear();

	if (m_craftingItemNameText) {
		objManager->RemoveGameObject(m_craftingItemNameText);
		delete m_craftingItemNameText;
		m_craftingItemNameText = nullptr;
	}

	UIElement::Release();
}

void MenuUI::InitializeCraftingUI()
{
	CreateCraftBar();
	CreatePanelBackgrounds();
	CreateMenuBarIcons();
	CreateToolButtons();
	CreateCreateItemButtons();
	CreateCookItemButtons();
	CreateCraftButton();
	CreateIngredientDisplay();
	CreateBossUI();
}

void MenuUI::CreateCraftBar()
{
	ObjectManager* objManager = ObjectManager::GetInstance();
	if (!objManager) return;

	m_menuBar = new UIImage(
		GOID_CRAFT_BAR,
		m_craftBarWidth,
		m_craftBarHeight,
		LAYER_UI_BACKGROUND,
		L"Resource/UI/CraftBar.png",
		0.0f,
		0.0f, 0.5f,
		0.0f, 0.5f,
		50.0f, 0.0f
	);
	objManager->AddGameObject(m_menuBar);
	}

	void MenuUI::CreatePanelBackgrounds()
	{
	ObjectManager* objManager = ObjectManager::GetInstance();
	if (!objManager) return;

	const float toolPanelOffsetX = m_craftBarWidth;
	const float toolButtonStartY = -(m_craftBarHeight * 0.5f);

	const int maxItemCount = static_cast<int>((std::max)({
		m_availableTools.size(),
		m_availableCreateItems.size(),
		m_availableCookItems.size() }));
	const int maxRows = (maxItemCount + m_columnsPerRow - 1) / m_columnsPerRow;

	const float palettePadding = 48.0f;
	const float toolPanelW = m_columnsPerRow * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing + palettePadding * 2.0f;
	const float toolPanelH = maxRows * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing + palettePadding * 2.0f;
	const float toolPanelCX = toolPanelOffsetX + toolPanelW * 0.5f - palettePadding;
	const float toolPanelCY = toolButtonStartY + toolPanelH * 0.5f - palettePadding;

	m_toolPanelBg = new UIImage(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 500),
		toolPanelW,
		toolPanelH,
		LAYER_UI_BACKGROUND,
		L"Resource/UI/CraftUI_PaletteBG.png",
		-0.05f,
		0.0f, 0.5f,
		0.0f, 0.5f,
		toolPanelCX + m_paletteBgOffsetX, toolPanelCY + m_paletteBgOffsetY
	);
	m_toolPanelBg->SetActive(false);
	objManager->AddGameObject(m_toolPanelBg);

	const float ingPadding = 16.0f;
	const float craftButtonY = m_ingredientStartY + m_ingredientImageSize * 0.5f + 28.0f + m_craftButtonHeight * 0.5f;
	const float ingTop = m_ingredientStartY - m_ingredientImageSize * 0.5f - ingPadding;
	const float ingBot = craftButtonY + m_craftButtonHeight * 0.5f + ingPadding;
	const float ingPanelH = ingBot - ingTop;
	const float ingPanelW = 2.0f * m_ingredientImageSize + m_ingredientSpacing + ingPadding * 2.0f + 50;

	m_ingredientPanelBg = new UIImage(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 501),
		ingPanelW,
		ingPanelH,
		LAYER_UI_BACKGROUND,
		L"Resource/UI/CraftUI_IngredientBG.png",
		-0.05f,
		0.0f, 0.5f,
		0.0f, 0.5f,
		m_ingredientPanelCenterX, ingTop + ingPanelH * 0.5f
	);
	m_ingredientPanelBg->SetActive(false);
	objManager->AddGameObject(m_ingredientPanelBg);
	}

	void MenuUI::CreateMenuBarIcons()
	{
	ObjectManager* objManager = ObjectManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!objManager || !resourceManager) return;

	std::shared_ptr<Sprite> craftIconNormal = resourceManager->LoadSprite(L"Resource/UI/CraftIcon.png");
	std::shared_ptr<Sprite> craftIconHover = resourceManager->LoadSprite(L"Resource/UI/CraftIcon.png");
	std::shared_ptr<Sprite> createNormal = resourceManager->LoadSprite(L"Resource/UI/CreateIcon.png");
	std::shared_ptr<Sprite> createHover = resourceManager->LoadSprite(L"Resource/UI/CreateIcon.png");
	std::shared_ptr<Sprite> editNormal = resourceManager->LoadSprite(L"Resource/UI/EditIcon.png");
	std::shared_ptr<Sprite> editHover = resourceManager->LoadSprite(L"Resource/UI/EditIcon.png");
	std::shared_ptr<Sprite> battleNormal = resourceManager->LoadSprite(L"Resource/UI/BattleIcon.png");
	std::shared_ptr<Sprite> battleHover = resourceManager->LoadSprite(L"Resource/UI/BattleIcon.png");
	std::shared_ptr<Sprite> cookNormal = resourceManager->LoadSprite(L"Resource/UI/CookIcon.png");
	std::shared_ptr<Sprite> cookHover = resourceManager->LoadSprite(L"Resource/UI/CookIcon.png");

	// m_menuIconY[0]=맨위(Edit) ~ m_menuIconY[4]=맨아래(Craft)
	m_craftIcon = new UIButton(GOID_CRAFT_ICON, m_iconSize, m_iconSize,
		craftIconNormal, craftIconHover, 0.0f, 0.5f, 0.0f, 0.5f, m_menuIconX, m_menuIconY[4]);
	m_craftIcon->SetOnClickCallback([this]() { ToggleToolList(); });
	objManager->AddGameObject(m_craftIcon);

	m_menuCreateIcon = new UIButton(static_cast<GameObjectID>(GOID_CRAFT_BAR + 10),
		m_iconSize + 20, m_iconSize + 20, createNormal, createHover,
		0.0f, 0.5f, 0.0f, 0.5f, m_menuIconX, m_menuIconY[3]);
	m_menuCreateIcon->SetOnClickCallback([this]() { ToggleCreateList(); });
	objManager->AddGameObject(m_menuCreateIcon);

	m_menuCookIcon = new UIButton(static_cast<GameObjectID>(GOID_CRAFT_BAR + 13),
		m_iconSize, m_iconSize, cookNormal, cookHover,
		0.0f, 0.5f, 0.0f, 0.5f, m_menuIconX, m_menuIconY[2]);
	m_menuCookIcon->SetOnClickCallback([this]() { ToggleCookList(); });
	objManager->AddGameObject(m_menuCookIcon);

	m_menuBattleIcon = new UIButton(static_cast<GameObjectID>(GOID_CRAFT_BAR + 12),
		m_iconSize, m_iconSize, battleNormal, battleHover,
		0.0f, 0.5f, 0.0f, 0.5f, m_menuIconX, m_menuIconY[1]);
	m_menuBattleIcon->SetOnClickCallback([this]() { ToggleBossPanel(); });
	objManager->AddGameObject(m_menuBattleIcon);

	m_menuEditIcon = new UIButton(static_cast<GameObjectID>(GOID_CRAFT_BAR + 11),
		m_iconSize, m_iconSize, editNormal, editHover,
		0.0f, 0.5f, 0.0f, 0.5f, m_menuIconX, m_menuIconY[0]);
	m_menuEditIcon->SetOnClickCallback([]() {});
	objManager->AddGameObject(m_menuEditIcon);
	}

	void MenuUI::CreateToolButtons()
	{
	ObjectManager* objManager = ObjectManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!objManager || !resourceManager) return;

	const float toolButtonStartY = -(m_craftBarHeight * 0.5f);
	const float startX = m_craftBarWidth + m_toolButtonSize * 0.5f + m_toolButtonOffsetX;

	for (size_t i = 0; i < m_availableTools.size(); ++i) {
		GameObjectID toolID = m_availableTools[i];

		std::wstring imagePath = ResourceUtils::GetResourceImagePath(toolID);
		if (imagePath.empty()) {
			std::wstring debugMsg = L"CraftingUI: 도구 버튼 이미지 경로 없음 - ID: " + std::to_wstring(toolID) + L"\n";
			OutputDebugStringW(debugMsg.c_str());
			continue;
		}

		int row = static_cast<int>(i / m_columnsPerRow);
		int col = static_cast<int>(i % m_columnsPerRow);
		float xPos = startX + col * (m_toolButtonSize + m_toolButtonSpacing);
		float yPos = toolButtonStartY + row * (m_toolButtonSize + m_toolButtonSpacing);

		GameObjectID buttonID = static_cast<GameObjectID>(GOID_CRAFT_TOOL_GOLDEN_SCYTHE + i);

		UIButton* toolButton = new UIButton(buttonID, m_toolButtonSize, m_toolButtonSize,
			resourceManager->LoadSprite(imagePath),
			resourceManager->LoadSprite(imagePath),
			0.0f, 0.5f, 0.0f, 0.5f, xPos, yPos);
		toolButton->SetActive(false);
		toolButton->SetOnClickCallback([this, toolID]() { SelectTool(toolID); });
		if (const ComponentElement::Image* img = toolButton->GetImageComponent())
			img->SetDisplaySizeProportional(m_toolButtonSize);
		m_toolButtons.push_back(toolButton);
		objManager->AddGameObject(toolButton);
	}
	}

	void MenuUI::CreateCreateItemButtons()
	{
	ObjectManager* objManager = ObjectManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!objManager || !resourceManager) return;

	const float toolButtonStartY = -(m_craftBarHeight * 0.5f);
	const float startX = m_craftBarWidth + m_toolButtonSize * 0.5f + m_toolButtonOffsetX;
	const int   createCount = static_cast<int>(m_availableCreateItems.size());

	for (int i = 0; i < createCount; ++i) {
		GameObjectID itemID = m_availableCreateItems[i];
		std::wstring imagePath = ResourceUtils::GetResourceImagePath(itemID);
		if (imagePath.empty()) {
			std::wstring debugMsg = L"CraftingUI: Create 버튼 이미지 경로 없음 - ID: " + std::to_wstring(itemID) + L"\n";
			OutputDebugStringW(debugMsg.c_str());
			continue;
		}


		float xPos = startX + (i % m_columnsPerRow) * (m_toolButtonSize + m_toolButtonSpacing);
		float yPos = toolButtonStartY;

		UIButton* btn = new UIButton(static_cast<GameObjectID>(GOID_CRAFT_BAR + 20 + i),
			m_toolButtonSize, m_toolButtonSize,
			resourceManager->LoadSprite(imagePath.c_str()),
			resourceManager->LoadSprite(imagePath.c_str()),
			0.0f, 0.5f, 0.0f, 0.5f, xPos, yPos);
		btn->SetActive(false);
		btn->SetOnClickCallback([this, itemID]() { SelectTool(itemID); });
		if (const ComponentElement::Image* img = btn->GetImageComponent())
			img->SetDisplaySizeProportional(m_toolButtonSize);
		m_createItemButtons.push_back(btn);
		objManager->AddGameObject(btn);
	}
	}

	void MenuUI::CreateCookItemButtons()
	{
	ObjectManager* objManager = ObjectManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!objManager || !resourceManager) return;

	const float toolButtonStartY = -(m_craftBarHeight * 0.5f);
	const float startX = m_craftBarWidth + m_toolButtonSize * 0.5f + m_toolButtonOffsetX;
	const int   cookCount = static_cast<int>(m_availableCookItems.size());

	for (int i = 0; i < cookCount; ++i) {
		GameObjectID itemID = m_availableCookItems[i];
		std::wstring imagePath = ResourceUtils::GetResourceImagePath(itemID);
		if (imagePath.empty()) {
			std::wstring debugMsg = L"CraftingUI: Cook 버튼 이미지 경로 없음 - ID: " + std::to_wstring(itemID) + L"\n";
			OutputDebugStringW(debugMsg.c_str());
			continue;
		}

		std::wstring debugMsg = L"CraftingUI: Cook 버튼 생성 [" + std::to_wstring(i) + L"] - " + imagePath + L"\n";
		OutputDebugStringW(debugMsg.c_str());

		float xPos = startX + (i % m_columnsPerRow) * (m_toolButtonSize + m_toolButtonSpacing);
		float yPos = toolButtonStartY;

		UIButton* btn = new UIButton(static_cast<GameObjectID>(GOID_CRAFT_BAR + 40 + i),
			m_toolButtonSize, m_toolButtonSize,
			resourceManager->LoadSprite(imagePath.c_str()),
			resourceManager->LoadSprite(imagePath.c_str()),
			0.0f, 0.5f, 0.0f, 0.5f, xPos, yPos);
		btn->SetActive(false);
		btn->SetOnClickCallback([this, itemID]() { SelectTool(itemID); });
		if (const ComponentElement::Image* img = btn->GetImageComponent())
			img->SetDisplaySizeProportional(m_toolButtonSize);
		m_cookItemButtons.push_back(btn);
		objManager->AddGameObject(btn);
	}
	}

	void MenuUI::CreateCraftButton()
	{
	ObjectManager* objManager = ObjectManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!objManager || !resourceManager) return;

	// 제작 버튼 Y = 재료 행 아래 레이아웃으로부터 계산 (재료중심 + 간격 + 버튼 반높이)
	const float craftButtonY = m_ingredientStartY + m_ingredientImageSize * 0.5f + 28.0f + m_craftButtonHeight * 0.5f;

	// 크래프팅 버튼 생성 (재료 행 아래, 도구 패널 가로 중앙에 배치)
	std::shared_ptr<Sprite> buttonNormalSprite = resourceManager->LoadSprite(L"Resource/UI/frontscreen.png");
	std::shared_ptr<Sprite> buttonHoverSprite = resourceManager->LoadSprite(L"Resource/UI/HighLight_frontscreen.png");

	m_craftButton = new UIButton(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 300),
		m_craftButtonWidth,
		m_craftButtonHeight,
		buttonNormalSprite,
		buttonHoverSprite,
		0.0f, 0.5f,
		0.0f, 0.5f,
		m_ingredientPanelCenterX, craftButtonY
	);

	m_craftButton->SetActive(false);

	m_craftButton->SetOnClickCallback([this]() {
		ObjectManager* objectManager = ObjectManager::GetInstance();
		Player* player = objectManager ? objectManager->GetPlayer() : nullptr;
		if (player) {
			TryCraftSelectedTool(player);
		}
		});

	objManager->AddGameObject(m_craftButton);

	// "제작하기" 버튼 텍스트
	m_craftButtonText = new UIText(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 301),
		m_craftButtonWidth,
		m_craftButtonHeight,
		L"제작하기",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.3f,
		L"맑은 고딕",
		14.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.0f, 0.5f,
		0.0f, 0.5f,
		m_ingredientPanelCenterX, craftButtonY
	);

	m_craftButtonText->SetActive(false);
	objManager->AddGameObject(m_craftButtonText);
	}

	void MenuUI::CreateIngredientDisplay()
	{
	ObjectManager* objManager = ObjectManager::GetInstance();
	if (!objManager) return;

	const float textHeight = 28.0f;
	const int   maxIngredients = 2;

	// 제작 아이템 이름 텍스트 생성 (재료 이미지 위쪽에 배치)
	float itemNameY = m_ingredientStartY - m_ingredientImageSize * 0.5f;
	m_craftingItemNameText = new UIText(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 400),
		250.0f, 30.0f, L"",
		Gdiplus::Color(255, 255, 255), LAYER_UI_FOREGROUND, 0.25f,
		L"맑은 고딕", 18.0f,
		Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
		0.0f, 0.5f,
		0.0f, 0.5f,
		m_ingredientPanelCenterX, itemNameY);
	m_craftingItemNameText->SetActive(false);
	objManager->AddGameObject(m_craftingItemNameText);

	for (int i = 0; i < maxIngredients; ++i) {
		float xPos = m_ingredientPanelCenterX + (i - 0.5f) * (m_ingredientImageSize + m_ingredientSpacing);
		float yPos = m_ingredientStartY;

		UIImage* ingredientImage = new UIImage(
			static_cast<GameObjectID>(GOID_CRAFT_BAR + 100 + i),
			m_ingredientImageSize, m_ingredientImageSize,
			LAYER_UI_FOREGROUND, L"", 0.1f,
			0.0f, 0.5f, 0.0f, 0.5f, xPos, yPos);
		ingredientImage->SetActive(false);
		m_ingredientImages.push_back(ingredientImage);
		objManager->AddGameObject(ingredientImage);

		float textY = yPos + m_ingredientImageSize * 0.5f + textHeight * 0.5f - 10.0f;
		UIText* ingredientText = new UIText(
			static_cast<GameObjectID>(GOID_CRAFT_BAR + 200 + i),
			180.0f, textHeight, L"",
			Gdiplus::Color(255, 255, 255), LAYER_UI_FOREGROUND, 0.25f,
			L"맑은 고딕", 13.0f,
			Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
			0.0f, 0.5f, 0.0f, 0.5f, xPos, textY);
		ingredientText->SetActive(false);
		m_ingredientTexts.push_back(ingredientText);
		objManager->AddGameObject(ingredientText);
	}
	}

	void MenuUI::ClearAllPanels()
	{
	// 모든 패널 상태 플래그 초기화
	m_isToolListVisible = false;
	m_isCreateListVisible = false;
	m_isCookListVisible = false;
	m_isBossPanelVisible = false;

	// 모든 버튼 숨김
	for (auto* button : m_toolButtons) {
		if (button) button->SetActive(false);
	}
	for (auto* button : m_createItemButtons) {
		if (button) button->SetActive(false);
	}
	for (auto* button : m_cookItemButtons) {
		if (button) button->SetActive(false);
	}

	m_bossOverlay->SetActive(false);
	m_houndBossPanel->SetActive(false);
	m_spiderQueenBossPanel->SetActive(false);

	if (m_houndClearText) m_houndClearText->SetActive(false);
	if (m_spiderQueenClearText) m_spiderQueenClearText->SetActive(false);

	m_bossChallengeButton->SetActive(false);
	m_bossChallengeButtonText->SetActive(false);

	// 패널 배경 숨김
	m_toolPanelBg->SetActive(false);
	m_ingredientPanelBg->SetActive(false);

	// 제작 버튼 숨김
	m_craftButton->SetActive(false);
	m_craftButtonText->SetActive(false);

	// 재료 UI 완전히 숨김
	m_craftingItemNameText->SetActive(false);

	for (auto* image : m_ingredientImages) {
		if (image) image->SetActive(false);
	}
	for (auto* text : m_ingredientTexts) {
		if (text) text->SetActive(false);
	}

	// 선택 초기화
	m_selectedToolID = GOID_NONE;
	m_selectedBossID = GOID_NONE;
	}

	void MenuUI::ToggleToolList()
	{
	// 원하는 동작: 현재 상태를 반전시키되, "열기" 동작을 수행할 때만 다른 패널을 닫음
	bool newState = !m_isToolListVisible;
	ClearAllPanels();

	m_isToolListVisible = newState;

	for (auto* button : m_toolButtons) {
		if (button) button->SetActive(m_isToolListVisible);
	}

	if (m_toolPanelBg) m_toolPanelBg->SetActive(m_isToolListVisible);
	}

	void MenuUI::ToggleCreateList()
	{
	bool newState = !m_isCreateListVisible;

	ClearAllPanels();
	m_isCreateListVisible = newState;

	for (auto* button : m_createItemButtons) {
		if (button) button->SetActive(m_isCreateListVisible);
	}

	if (m_toolPanelBg) m_toolPanelBg->SetActive(m_isCreateListVisible);
	}

	void MenuUI::ToggleCookList()
	{
	bool newState = !m_isCookListVisible;
	ClearAllPanels();

	m_isCookListVisible = newState;

	for (auto* button : m_cookItemButtons) {
		if (button) button->SetActive(m_isCookListVisible);
	}

	if (m_toolPanelBg) m_toolPanelBg->SetActive(m_isCookListVisible);
	}

	void MenuUI::ToggleBossPanel()
	{
	bool newState = !m_isBossPanelVisible;

	ClearAllPanels();
	m_isBossPanelVisible = newState;

	if (m_bossOverlay) m_bossOverlay->SetActive(m_isBossPanelVisible);
	if (m_houndBossPanel) m_houndBossPanel->SetActive(m_isBossPanelVisible);
	if (m_spiderQueenBossPanel) m_spiderQueenBossPanel->SetActive(m_isBossPanelVisible);

	if (m_isBossPanelVisible) {
		UpdateBossPanelHighlight();
	}

	if (m_bossChallengeButton) m_bossChallengeButton->SetActive(m_isBossPanelVisible);
	if (m_bossChallengeButtonText) m_bossChallengeButtonText->SetActive(m_isBossPanelVisible);
	}

	void MenuUI::SelectTool(GameObjectID toolID)
	{

	if (toolID == m_selectedToolID) {
		m_selectedToolID = GOID_NONE;
		UpdateIngredientDisplay();
		if (m_ingredientPanelBg) m_ingredientPanelBg->SetActive(false);
		return;
	}
	m_selectedToolID = toolID;
	UpdateIngredientDisplay();
	if (m_ingredientPanelBg) m_ingredientPanelBg->SetActive(true);

	if (m_craftButton)     m_craftButton->SetActive(true);
	if (m_craftButtonText) m_craftButtonText->SetActive(true);
	}

	void MenuUI::UpdateIngredientDisplay()
	{
	if (m_selectedToolID == GOID_NONE) {
		for (auto* image : m_ingredientImages) {
			if (image) image->SetActive(false);
		}
		for (auto* text : m_ingredientTexts) {
			if (text) text->SetActive(false);
		}
		if (m_craftingItemNameText) m_craftingItemNameText->SetActive(false);
		if (m_craftButton) m_craftButton->SetActive(false);
		if (m_craftButtonText) m_craftButtonText->SetActive(false);
		return;
	}

	// 제작 아이템 이름 표시
	if (m_craftingItemNameText) {
		std::wstring itemName = ResourceUtils::GetResourceDisplayName(m_selectedToolID);
		m_craftingItemNameText->SetText(itemName);
		m_craftingItemNameText->SetActive(true);
	}

	// InventoryManager에서 레시피 가져오기
	InventoryManager* inventoryManager = InventoryManager::GetInstance();
	if (!inventoryManager) return;

	const std::map<UINT, UINT>* recipe = inventoryManager->GetCraftingRecipe(m_selectedToolID);
	if (!recipe || recipe->empty()) {
		OutputDebugStringW(L"CraftingUI: 레시피를 찾을 수 없습니다.\n");
		return;
	}
	int ingredientIndex = 0;

	// 재료 정보 표시
	for (const auto& ingredient : *recipe) {
		if (ingredientIndex >= 2) break;

		GameObjectID ingredientID = static_cast<GameObjectID>(ingredient.first);
		UINT count = ingredient.second;

		// 재료 이미지 업데이트
		if (ingredientIndex < static_cast<int>(m_ingredientImages.size())) {
			UIImage* image = m_ingredientImages[ingredientIndex];
			if (image) {
				std::wstring imagePath = ResourceUtils::GetResourceImagePath(ingredientID);
				if (!imagePath.empty()) {
					image->LoadSprite(imagePath);
					if (const ComponentElement::Image* img = image->GetImageComponent()) {
						img->SetDisplaySizeProportional(m_ingredientImageSize);
					}
					image->SetActive(true);

					std::wstring debugMsg = L"CraftingUI: 재료 이미지 로드 [" + std::to_wstring(ingredientIndex) + L"] - " + imagePath + L"\n";
					OutputDebugStringW(debugMsg.c_str());
				}
				else {
					std::wstring debugMsg = L"CraftingUI: 재료 이미지 경로 없음 - ID: " + std::to_wstring(ingredientID) + L"\n";
					OutputDebugStringW(debugMsg.c_str());
				}
			}
		}

		// 재료 텍스트 업데이트
		if (ingredientIndex < static_cast<int>(m_ingredientTexts.size())) {
			UIText* text = m_ingredientTexts[ingredientIndex];
			if (text) {
				std::wstring textStr = std::to_wstring(count) + L" x " + ResourceUtils::GetResourceDisplayName(ingredientID);
				text->SetText(textStr);
				text->SetActive(true);
			}
		}

		++ingredientIndex;
	}

	// 나머지 재료 슬롯 숨김
	for (int i = ingredientIndex; i < 2; ++i) {
		if (i < static_cast<int>(m_ingredientImages.size())) {
			if (m_ingredientImages[i]) {
				m_ingredientImages[i]->SetActive(false);
			}
		}
		if (i < static_cast<int>(m_ingredientTexts.size())) {
			if (m_ingredientTexts[i]) {
				m_ingredientTexts[i]->SetActive(false);
			}
		}
	}
	}

	bool MenuUI::TryCraftSelectedTool(Player* player)
	{
	if (m_selectedToolID == GOID_NONE) {
		OutputDebugStringW(L"CraftingUI: 선택된 도구/아이템이 없습니다.\n");
		return false;
	}

	if (!player) {
		OutputDebugStringW(L"CraftingUI: 플레이어가 없습니다.\n");
		return false;
	}

	InventoryManager* inventoryManager = InventoryManager::GetInstance();
	if (!inventoryManager) {
		OutputDebugStringW(L"CraftingUI: InventoryManager를 찾을 수 없습니다.\n");
		return false;
	}

	// InventoryManager를 통해 제작 시도
	bool craftSuccess = inventoryManager->TryCraftItem(player, m_selectedToolID);

	if (craftSuccess) {
		std::wstring itemName = ResourceUtils::GetResourceDisplayName(m_selectedToolID);
		std::wstring msg = L"CraftingUI: " + itemName + L" 제작 성공!\n";
		OutputDebugStringW(msg.c_str());

		// 재료 표시 업데이트
		UpdateIngredientDisplay();
	}
	else {
		std::wstring itemName = ResourceUtils::GetResourceDisplayName(m_selectedToolID);
		std::wstring msg = L"CraftingUI: " + itemName + L" 제작 실패 (재료 부족)\n";
		OutputDebugStringW(msg.c_str());
	}

	return craftSuccess;
	}

	void MenuUI::CreateBossUI()
	{
	ObjectManager* objManager = ObjectManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!objManager || !resourceManager) return;

	OutputDebugStringW(L"CraftingUI: CreateBossUI 시작\n");

	const float screenCenterX = 0.0f;
	const float screenCenterY = 0.0f;

	// 1. 반투명 검은색 오버레이 (전체 화면)
	m_bossOverlay = new UIImage(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 600),
		1920.0f, 1080.0f,
		LAYER_UI_BACKGROUND,
		L"Resource/UI/frontscreen.png",
		-0.1f,
		0.5f, 0.5f,
		0.5f, 0.5f,
		screenCenterX, screenCenterY
	);
	m_bossOverlay->SetActive(false);
	m_bossOverlay->SetTintColor(0, 0, 0, 180);
	objManager->AddGameObject(m_bossOverlay);
	OutputDebugStringW(L"CraftingUI: 보스 오버레이 생성 완료\n");

	const float panelWidth = 300.0f;
	const float panelHeight = 400.0f;
	const float panelSpacing = 100.0f;

	// 하운드 보스 패널 (왼쪽)
	const float houndPanelX = screenCenterX - panelWidth * 0.5f - panelSpacing * 0.5f;
	const float panelY = screenCenterY;

	std::shared_ptr<Sprite> houndPanelNormal = resourceManager->LoadSprite(L"Resource/UI/BossBattleHound.png");
	std::shared_ptr<Sprite> houndPanelHover = resourceManager->LoadSprite(L"Resource/UI/BossBattleHound.png");

	if (!houndPanelNormal || !houndPanelNormal->bitmap) {
		OutputDebugStringW(L"CraftingUI: 하운드 패널 이미지 로드 실패 - Resource/UI/BossBattleHound.png\n");
	}
	else {
		OutputDebugStringW(L"CraftingUI: 하운드 패널 이미지 로드 성공\n");
	}

	m_houndBossPanel = new UIButton(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 601),
		panelWidth, panelHeight,
		houndPanelNormal, houndPanelHover,
		0.5f, 0.5f,
		0.5f, 0.5f,
		houndPanelX, panelY
	);
	m_houndBossPanel->SetActive(false);
	m_houndBossPanel->SetOnClickCallback([this]() { SelectBoss(GOID_MONSTER_HOUNDDOG); });
	objManager->AddGameObject(m_houndBossPanel);

	// 하운드 CLEAR 텍스트
	m_houndClearText = new UIText(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 605),
		200.0f, 60.0f, L"CLEAR", Gdiplus::Color::Red, LAYER_UI_FOREGROUND, 1000.0f, L"Arial", 48.0f
	);
	m_houndClearText->GetRectTransform()->SetPosition(houndPanelX, panelY);
	m_houndClearText->GetRectTransform()->SetRotation(-25.0f); // 약간 기울임
	m_houndClearText->SetActive(false);
	objManager->AddGameObject(m_houndClearText);

	if (ComponentElement::Image* img = const_cast<ComponentElement::Image*>(m_houndBossPanel->GetImageComponent())) {
		img->SetTintColor(200, 200, 200, 255);
	}

	// 스파이더 퀸 보스 패널 (오른쪽)
	const float spiderPanelX = screenCenterX + panelWidth * 0.5f + panelSpacing * 0.5f;

	std::shared_ptr<Sprite> spiderPanelNormal = resourceManager->LoadSprite(L"Resource/UI/BossBattleSpiderQueen.png");
	std::shared_ptr<Sprite> spiderPanelHover = resourceManager->LoadSprite(L"Resource/UI/BossBattleSpiderQueen.png");

	if (!spiderPanelNormal || !spiderPanelNormal->bitmap) {
		OutputDebugStringW(L"CraftingUI: 스파이더 퀸 패널 이미지 로드 실패 - Resource/UI/BossBattleSpiderQueen.png\n");
	}
	else {
		OutputDebugStringW(L"CraftingUI: 스파이더 퀸 패널 이미지 로드 성공\n");
	}

	m_spiderQueenBossPanel = new UIButton(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 602),
		panelWidth, panelHeight,
		spiderPanelNormal, spiderPanelHover,
		0.5f, 0.5f,
		0.5f, 0.5f,
		spiderPanelX, panelY
	);
	m_spiderQueenBossPanel->SetActive(false);
	m_spiderQueenBossPanel->SetOnClickCallback([this]() {
		SelectBoss(GOID_MONSTER_QUEEN_SPIDER);
		});

	objManager->AddGameObject(m_spiderQueenBossPanel);

	// 스파이더 퀸 CLEAR 텍스트
	m_spiderQueenClearText = new UIText(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 606),
		200.0f, 60.0f, L"CLEAR", Gdiplus::Color::Red, LAYER_UI_FOREGROUND, 1000.0f, L"Arial", 48.0f
	);
	m_spiderQueenClearText->GetRectTransform()->SetPosition(spiderPanelX, panelY);
	m_spiderQueenClearText->GetRectTransform()->SetRotation(-25.0f); // 약간 기울임
	m_spiderQueenClearText->SetActive(false);
	objManager->AddGameObject(m_spiderQueenClearText);

	if (ComponentElement::Image* img = const_cast<ComponentElement::Image*>(m_spiderQueenBossPanel->GetImageComponent())) {
		img->SetTintColor(150, 150, 150, 255);
	}

	// 3. 보스 도전 버튼
	const float buttonWidth = 200.0f;
	const float buttonHeight = 60.0f;
	const float buttonY = panelY + panelHeight * 0.5f + 80.0f;

	std::shared_ptr<Sprite> buttonNormal = resourceManager->LoadSprite(L"Resource/UI/frontscreen.png");
	std::shared_ptr<Sprite> buttonHover = resourceManager->LoadSprite(L"Resource/UI/HighLight_frontscreen.png");

	m_bossChallengeButton = new UIButton(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 603),
		buttonWidth, buttonHeight,
		buttonNormal, buttonHover,
		0.5f, 0.5f,
		0.5f, 0.5f,
		screenCenterX, buttonY
	);
	m_bossChallengeButton->SetActive(false);
	m_bossChallengeButton->SetOnClickCallback([this]() { TryChallengeBoss(); });
	objManager->AddGameObject(m_bossChallengeButton);

	m_bossChallengeButtonText = new UIText(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 604),
		buttonWidth, buttonHeight,
		L"보스 도전",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.2f,
		L"맑은 고딕",
		18.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.5f,
		0.5f, 0.5f,
		screenCenterX, buttonY
	);
	m_bossChallengeButtonText->SetActive(false);
	objManager->AddGameObject(m_bossChallengeButtonText);

	// 초기 하이라이트 상태 업데이트
	UpdateBossPanelHighlight();

	OutputDebugStringW(L"CraftingUI: CreateBossUI 완료\n");
	}

void MenuUI::SelectBoss(GameObjectID bossID)
{
	// 스파이더 퀸은 하운드 클리어 후에만 선택 가능
	if (bossID == GOID_MONSTER_QUEEN_SPIDER && !IsHoundBossCleared()) {
		OutputDebugStringW(L"CraftingUI: 스파이더 퀸은 하운드 보스를 클리어해야 도전할 수 있습니다.\n");
		// 선택하지 않고 리턴 (클릭 무시)
		return;
	}

	m_selectedBossID = bossID;
	UpdateBossPanelHighlight();

	std::wstring msg = L"CraftingUI: 보스 선택 - ID: " + std::to_wstring(bossID) + L"\n";
	OutputDebugStringW(msg.c_str());
}

void MenuUI::TryChallengeBoss()
{
	if (m_selectedBossID == GOID_NONE) {
		// 보스가 선택되지 않았을 때 기본 보스 선택
		if (!IsHoundBossCleared()) {
			m_selectedBossID = GOID_MONSTER_HOUNDDOG;
		}
		else {
			m_selectedBossID = GOID_MONSTER_QUEEN_SPIDER;
		}
	}

	// 스파이더 퀸 도전 전 하운드 클리어 확인
	if (m_selectedBossID == GOID_MONSTER_QUEEN_SPIDER && !IsHoundBossCleared()) {
		OutputDebugStringW(L"CraftingUI: 스파이더 퀸에 도전하려면 먼저 하운드 보스를 클리어해야 합니다.\n");
		return;
	}

	// 보스 씬으로 전환 요청
	std::wstring msg = L"CraftingUI: 보스 도전 시도 - BossID: " + std::to_wstring(m_selectedBossID) + L"\n";
	OutputDebugStringW(msg.c_str());

	// 현재 플레이어의 캐릭터 ID를 전달
	ObjectManager* objectManager = ObjectManager::GetInstance();
	Player* player = objectManager ? objectManager->GetPlayer() : nullptr;
	GameObjectID selectedChar = player ? player->GetID() : GOID_NONE;

	if (m_selectedBossID == GOID_MONSTER_HOUNDDOG) {
		// 하우드 보스 맵 로드
		SceneManager::GetInstance()->LoadGameScene(SCENE_GAME_HOUND_FOREST, selectedChar);
		OutputDebugStringW(L"CraftingUI: 하운드 숲으로 이동 요청 전송\n");
	} else if (m_selectedBossID == GOID_MONSTER_QUEEN_SPIDER) {
		// 스파이더 퀸 보스 맵 로드
		SceneManager::GetInstance()->LoadGameScene(SCENE_GAME_SPIDER_QUEEN_HOUSE, selectedChar);
		OutputDebugStringW(L"CraftingUI: 스파이더 퀸의 집으로 이동 요청 전송\n");
	}

	// 전환 요청 후 UI 닫기
	ClearAllPanels();
}

bool MenuUI::IsHoundBossCleared() const
{
	GameProgressManager* progressManager = GameProgressManager::GetInstance();
	if (!progressManager) return false;
	return progressManager->IsSceneCleared(SCENE_GAME_HOUND_FOREST);
}

bool MenuUI::IsSpiderQueenBossCleared() const
{
	GameProgressManager* progressManager = GameProgressManager::GetInstance();
	if (!progressManager) return false;
	return progressManager->IsSceneCleared(SCENE_GAME_SPIDER_QUEEN_HOUSE);
}

void MenuUI::UpdateBossPanelHighlight()
{
	if (!m_houndBossPanel || !m_spiderQueenBossPanel) return;

	bool isHoundCleared = IsHoundBossCleared();
	bool isSpiderQueenCleared = IsSpiderQueenBossCleared();

	// 하운드 보스 패널 효과
	m_houndBossPanel->SetDisabled(isHoundCleared);
	if (m_houndClearText) m_houndClearText->SetActive(m_isBossPanelVisible && isHoundCleared);

	if (m_selectedBossID == GOID_MONSTER_HOUNDDOG && !isHoundCleared)
	{
		// 선택됨: 빨간색 틴트 + 크기 확대
		if (ComponentElement::Image* img = const_cast<ComponentElement::Image*>(m_houndBossPanel->GetImageComponent())) {
			img->SetTintColor(255, 180, 180, 255);
		}
		if (RectTransform* rectTransform = m_houndBossPanel->GetRectTransform()) {
			rectTransform->SetScale(1.1f, 1.1f);
		}
	}
	else
	{
		if (ComponentElement::Image* img = const_cast<ComponentElement::Image*>(m_houndBossPanel->GetImageComponent())) {
			if (isHoundCleared) img->SetTintColor(100, 100, 100, 255); // 클리어 시 어둡게
			else img->SetTintColor(200, 200, 200, 255);
		}
		if (RectTransform* rectTransform = m_houndBossPanel->GetRectTransform()) {
			rectTransform->SetScale(1.0f, 1.0f);
		}
	}

	// 스파이더 퀸 보스 패널 효과
	bool isSpiderQueenLocked = !isHoundCleared;
	m_spiderQueenBossPanel->SetDisabled(isSpiderQueenLocked || isSpiderQueenCleared);
	if (m_spiderQueenClearText) m_spiderQueenClearText->SetActive(m_isBossPanelVisible && isSpiderQueenCleared);

	if (m_selectedBossID == GOID_MONSTER_QUEEN_SPIDER && !isSpiderQueenLocked && !isSpiderQueenCleared) {
		// 선택됨 (해금된 상태): 빨간색 틴트 + 크기 확대
		if (ComponentElement::Image* img = const_cast<ComponentElement::Image*>(m_spiderQueenBossPanel->GetImageComponent())) {
			img->SetTintColor(255, 180, 180, 255);
		}
		if (RectTransform* rectTransform = m_spiderQueenBossPanel->GetRectTransform()) {
			rectTransform->SetScale(1.1f, 1.1f);
		}
	}
	else {
		if (ComponentElement::Image* img = const_cast<ComponentElement::Image*>(m_spiderQueenBossPanel->GetImageComponent())) {
			if (isSpiderQueenCleared) img->SetTintColor(100, 100, 100, 255); // 클리어 시 어둡게
			else if (isSpiderQueenLocked) img->SetTintColor(120, 120, 120, 255); // 잠금 시 어둡게
			else img->SetTintColor(150, 150, 150, 255);
		}
		// 원래 크기로 복원
		if (RectTransform* rectTransform = m_spiderQueenBossPanel->GetRectTransform()) {
			rectTransform->SetScale(1.0f, 1.0f);
		}
	}
}

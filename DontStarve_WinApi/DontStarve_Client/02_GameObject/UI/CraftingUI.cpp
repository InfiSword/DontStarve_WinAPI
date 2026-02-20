#include "99_Default/pch.h"
#include "CraftingUI.h"
#include "CraftingRecipe.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UIText.h"
#include "../../01_Manager/UIManager/UIManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../Entity/Player/Player.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"

CraftingUI::CraftingUI()
	: UIElement(GOBJ_UI, GOID_CRAFT_BAR, L"", L"", true, false),
	m_craftBar(nullptr),
	m_craftIcon(nullptr),
	m_craftButton(nullptr),
	m_craftButtonText(nullptr),
	m_isToolListVisible(false),
	m_selectedToolID(GOID_NONE),
	// UI 레이아웃 상수 초기화
	m_craftBarWidth(125.0f),
	m_craftBarHeight(400.0f),
	m_iconSize(64.0f),
	m_iconOffsetFromTop(80.0f),
	m_toolButtonSize(64.0f),
	m_toolButtonSpacing(8.0f),
	m_toolPanelOffsetX(0.0f), // 생성자에서 계산
	m_toolButtonStartY(0.0f), // 생성자에서 계산
	m_columnsPerRow(3),
	m_craftButtonWidth(120.0f),
	m_craftButtonHeight(40.0f),
	m_craftButtonOffsetFromTop(140.0f),
	m_ingredientImageSize(74.0f),
	m_ingredientTextHeight(28.0f),
	m_ingredientSpacing(58.0f),
	m_ingredientStartY(0.0f),
	m_ingredientToolGap(24.0f),
	m_toolPanelBottomY(0.0f),
	m_ingredientPanelCenterX(0.0f),
	m_craftButtonY(0.0f)
{
	// 제작 가능한 도구 목록 초기화 (CraftingRecipeTable에서 자동 생성)
	m_availableTools.clear();
	for (size_t i = 0; i < CraftingRecipeCount; ++i) {
		m_availableTools.push_back(CraftingRecipeTable[i].toolID);
	}

	// 계산된 값들 초기화 (도구 패널 하단 → 재료 행 → 제작 버튼 순, Y 증가 = 화면 아래)
	m_toolPanelOffsetX = m_craftBarWidth;
	m_toolButtonStartY = -(m_craftBarHeight * 0.5f);
	int toolRows = (static_cast<int>(m_availableTools.size()) + m_columnsPerRow - 1) / m_columnsPerRow;
	// 도구 마지막 행의 하단 Y (재료는 이 아래에 배치, 살짝 더 아래로 추가 오프셋 20)
	m_toolPanelBottomY = m_toolButtonStartY + (toolRows - 1) * (m_toolButtonSize + m_toolButtonSpacing) + m_toolButtonSize * 0.5f;
	const float extraOffsetDown = 20.0f;
	m_ingredientStartY = m_toolPanelBottomY + m_ingredientToolGap + m_ingredientImageSize * 0.5f + extraOffsetDown;
	m_ingredientPanelCenterX = m_toolPanelOffsetX + (m_columnsPerRow * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing) * 0.5f;
	m_craftButtonY = m_ingredientStartY + m_ingredientImageSize * 0.5f + 28.0f + m_craftButtonHeight * 0.5f;
}

CraftingUI::~CraftingUI()
{
	Release();
}

void CraftingUI::Init()
{
	InitializeCraftingUI();
}

void CraftingUI::Update(float deltaTime)
{
	GameObject::Update(deltaTime);
}

void CraftingUI::Release()
{
	UIManager* uiManager = UIManager::GetInstance();
	
	// 생성한 UI 요소들을 UIManager에서 제거
	if (uiManager) {
		if (m_craftBar) {
			uiManager->RemoveUIImage(m_craftBar);
			delete m_craftBar;
			m_craftBar = nullptr;
		}
		
		if (m_craftIcon) {
			uiManager->RemoveUIButton(m_craftIcon);
			delete m_craftIcon;
			m_craftIcon = nullptr;
		}
		
		if (m_craftButton) {
			uiManager->RemoveUIButton(m_craftButton);
			delete m_craftButton;
			m_craftButton = nullptr;
		}
		if (m_craftButtonText) {
			uiManager->RemoveUIText(m_craftButtonText);
			delete m_craftButtonText;
			m_craftButtonText = nullptr;
		}
		
		for (auto* button : m_toolButtons) {
			if (button) {
				uiManager->RemoveUIButton(button);
				delete button;
			}
		}
		m_toolButtons.clear();
		
		for (auto* image : m_ingredientImages) {
			if (image) {
				uiManager->RemoveUIImage(image);
				delete image;
			}
		}
		m_ingredientImages.clear();
		
		for (auto* text : m_ingredientTexts) {
			if (text) {
				uiManager->RemoveUIText(text);
				delete text;
			}
		}
		m_ingredientTexts.clear();
	}
	
	UIElement::Release();
}

void CraftingUI::InitializeCraftingUI()
{
	CreateCraftBar();
	CreateCraftIcon();
	CreateToolButtons();
	CreateCraftButton();
	CreateIngredientDisplay();
}

void CraftingUI::CreateCraftBar()
{
	UIManager* uiManager = UIManager::GetInstance();
	if (!uiManager) return;

	// CraftBar 배경 이미지 생성 (화면 가장 왼쪽 중앙)
	// pivot: (0.0, 0.5) - 왼쪽 중앙 기준
	// anchor: (0.0, 0.5) - 왼쪽 중앙에 고정
	m_craftBar = new UIImage(
		GOID_CRAFT_BAR,
		m_craftBarWidth,
		m_craftBarHeight,
		LAYER_UI_BACKGROUND,
		L"Resource/UI/CraftBar.png",
		0.0f,
		0.0f, 0.5f,  // anchorMin (왼쪽 중앙)
		0.0f, 0.5f,  // anchorMax (왼쪽 중앙)
		-10.0f, 0.0f // anchoredPosition (패널 너비/2만큼 오른쪽, pivot이 왼쪽이므로)
	);

	// pivot 설정 (왼쪽 중앙)
	if (m_craftBar && m_craftBar->GetRectTransform()) {
		m_craftBar->GetRectTransform()->SetPivot(0.0f, 0.5f);
	}

	uiManager->AddUIImage(m_craftBar);
}

void CraftingUI::CreateCraftIcon()
{
	UIManager* uiManager = UIManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!uiManager || !resourceManager) return;

	// CraftIcon 버튼 생성 (CraftBar 내부 상단에 배치)
	// CraftBar 기준으로 상대 위치 계산
	std::shared_ptr<Sprite> iconNormalSprite = resourceManager->LoadSprite(L"Resource/UI/CraftIcon.png");
	std::shared_ptr<Sprite> iconHoverSprite = resourceManager->LoadSprite(L"Resource/UI/CraftIcon.png");

	m_craftIcon = new UIButton(
		GOID_CRAFT_ICON,
		m_iconSize,
		m_iconSize,
		iconNormalSprite,
		iconHoverSprite,
		0.0f, 0.0f,  // anchorMin 
		0.0f, 0.5f,  // anchorMax 
		(m_iconOffsetFromTop) * 0.5f, (m_iconOffsetFromTop - m_iconSize * 0.5f) // CraftBar 내부 상단 중앙
	);
	// UIButton 생성자에서 이미 pivot (0.5, 0.5)로 설정됨

	// 클릭 콜백 설정
	m_craftIcon->SetOnClickCallback([this]() {
		ToggleToolList();
	});

	uiManager->AddUIButton(m_craftIcon);
}

void CraftingUI::CreateToolButtons()
{
	UIManager* uiManager = UIManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!uiManager || !resourceManager) return;

	// 도구 버튼 ID·이미지 경로 매핑 테이블 (static map 제거 → 배열로 대체)
	struct ToolButtonInfo {
		GameObjectID toolID;
		GameObjectID buttonID;
		const wchar_t* imagePath;
	};
	static constexpr ToolButtonInfo toolInfoTable[] = {
		{ GOID_TOOL_GOLDEN_SCYTHE, GOID_CRAFT_TOOL_GOLDEN_SCYTHE, L"Resource/Objects/Tools/Golden_Scythe_02.png" },
		{ GOID_TOOL_HAM_BAT,       GOID_CRAFT_TOOL_HAM_BAT,       L"Resource/Objects/Tools/hamBat_01.png" },
		{ GOID_TOOL_PICKAXE,       GOID_CRAFT_TOOL_PICKAXE,       L"Resource/Objects/Tools/pickaxe-0.png" },
		{ GOID_TOOL_RED_AXE,       GOID_CRAFT_TOOL_RED_AXE,       L"Resource/Objects/Tools/Red_Axe_02.png" },
		{ GOID_TOOL_SPEAR,         GOID_CRAFT_TOOL_SPEAR,         L"Resource/Objects/Tools/spear_03.png" },
		{ GOID_TOOL_SWAP_AXE,      GOID_CRAFT_TOOL_SWAP_AXE,      L"Resource/Objects/Tools/swap_axe-0.png" },
		{ GOID_TOOL_SWAP_SPEAR,    GOID_CRAFT_TOOL_SWAP_SPEAR,    L"Resource/Objects/Tools/swap_spear_wathgrithr_lightning-5.png" },
		{ GOID_TOOL_TORCH,         GOID_CRAFT_TOOL_TORCH,         L"Resource/Objects/Tools/torch.png" },
	};
	static constexpr size_t toolInfoCount = sizeof(toolInfoTable) / sizeof(ToolButtonInfo);

	const float startX = m_toolPanelOffsetX + m_toolButtonSize * 0.5f;

	for (size_t i = 0; i < m_availableTools.size(); ++i) {
		GameObjectID toolID = m_availableTools[i];

		// 배열에서 일치하는 항목 찾기
		const ToolButtonInfo* info = nullptr;
		for (size_t j = 0; j < toolInfoCount; ++j) {
			if (toolInfoTable[j].toolID == toolID) {
				info = &toolInfoTable[j];
				break;
			}
		}
		if (!info) continue;

		std::shared_ptr<Sprite> toolNormalSprite = resourceManager->LoadSprite(info->imagePath);
		std::shared_ptr<Sprite> toolHoverSprite  = resourceManager->LoadSprite(info->imagePath);

		int row = static_cast<int>(i / m_columnsPerRow);
		int col = static_cast<int>(i % m_columnsPerRow);

		float xPos = startX + col * (m_toolButtonSize + m_toolButtonSpacing);
		float yPos = m_toolButtonStartY + row * (m_toolButtonSize + m_toolButtonSpacing);

		UIButton* toolButton = new UIButton(
			info->buttonID,
			m_toolButtonSize,
			m_toolButtonSize,
			toolNormalSprite,
			toolHoverSprite,
			0.0f, 0.5f,
			0.0f, 0.5f,
			xPos, yPos
		);

		// UIButton 생성자에서 이미 pivot (0.5, 0.5)로 설정됨

		// 초기에는 숨김
		toolButton->SetActive(false);

		// 클릭 콜백 설정
		toolButton->SetOnClickCallback([this, toolID]() {
			SelectTool(toolID);
		});

		if (const ComponentElement::Image* img = toolButton->GetImageComponent())
			img->SetDisplaySizeProportional(m_toolButtonSize);

		m_toolButtons.push_back(toolButton);
		uiManager->AddUIButton(toolButton);
	}
}

void CraftingUI::CreateCraftButton()
{
	UIManager* uiManager = UIManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!uiManager || !resourceManager) return;

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
		m_ingredientPanelCenterX, m_craftButtonY
	);

	m_craftButton->SetActive(false);

	m_craftButton->SetOnClickCallback([this]() {
		ObjectManager* objectManager = ObjectManager::GetInstance();
		Player* player = objectManager ? objectManager->GetPlayer() : nullptr;
		if (player) {
			TryCraftSelectedTool(player);
		}
	});

	uiManager->AddUIButton(m_craftButton);

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
		m_ingredientPanelCenterX, m_craftButtonY
	);

	m_craftButtonText->SetActive(false);
	uiManager->AddUIText(m_craftButtonText);
}

void CraftingUI::CreateIngredientDisplay()
{
	UIManager* uiManager = UIManager::GetInstance();
	if (!uiManager) return;

	// 재료 이미지와 텍스트 생성 (최대 2개, Tools 영역 아래 가로 배치)
	const int maxIngredients = 2;
	for (int i = 0; i < maxIngredients; ++i) {
		float xPos = m_ingredientPanelCenterX + (i - 0.5f) * (m_ingredientImageSize + m_ingredientSpacing);
		float yPos = m_ingredientStartY;

		UIImage* ingredientImage = new UIImage(
			static_cast<GameObjectID>(GOID_CRAFT_BAR + 100 + i),
			m_ingredientImageSize,
			m_ingredientImageSize,
			LAYER_UI_FOREGROUND,
			L"",
			0.1f,
			0.0f, 0.5f,
			0.0f, 0.5f,
			xPos, yPos
		);

		ingredientImage->SetActive(false);
		m_ingredientImages.push_back(ingredientImage);
		uiManager->AddUIImage(ingredientImage);

		float textY = yPos + m_ingredientImageSize * 0.5f  + m_ingredientTextHeight * 0.5f -10;
		UIText* ingredientText = new UIText(
			static_cast<GameObjectID>(GOID_CRAFT_BAR + 200 + i),
			180.0f,
			m_ingredientTextHeight,
			L"",
			Gdiplus::Color(0, 0, 0),
			LAYER_UI_FOREGROUND,
			0.25f,
			L"맑은 고딕",
			15.0f,
			Gdiplus::StringAlignmentCenter,
			Gdiplus::StringAlignmentCenter,
			0.0f, 0.5f,
			0.0f, 0.5f,
			xPos, textY
		);

		ingredientText->SetActive(false);
		m_ingredientTexts.push_back(ingredientText);
		uiManager->AddUIText(ingredientText);
	}
}

void CraftingUI::ToggleToolList()
{
	m_isToolListVisible = !m_isToolListVisible;

	// 도구 버튼들 표시/숨김
	for (auto* button : m_toolButtons) {
		if (button) {
			button->SetActive(m_isToolListVisible);
		}
	}
	// 닫을 때 선택 해제 및 재료·제작하기 비활성화
	if (!m_isToolListVisible) {
		m_selectedToolID = GOID_NONE;
		UpdateIngredientDisplay();
	}
}

void CraftingUI::SelectTool(GameObjectID toolID)
{
	// 같은 도구를 다시 클릭하면 선택 해제(재료·제작하기 비활성화)
	if (toolID == m_selectedToolID) {
		m_selectedToolID = GOID_NONE;
		UpdateIngredientDisplay();
		return;
	}
	m_selectedToolID = toolID;
	UpdateIngredientDisplay();
	
	if (m_craftButton) {
		m_craftButton->SetActive(true);
	}
	if (m_craftButtonText) {
		m_craftButtonText->SetActive(true);
	}
}

void CraftingUI::UpdateIngredientDisplay()
{
	if (m_selectedToolID == GOID_NONE) {
		for (auto* image : m_ingredientImages) {
			if (image) image->SetActive(false);
		}
		for (auto* text : m_ingredientTexts) {
			if (text) text->SetActive(false);
		}
		if (m_craftButton) m_craftButton->SetActive(false);
		if (m_craftButtonText) m_craftButtonText->SetActive(false);
		return;
	}

	// InventoryManager에서 레시피 가져오기
	InventoryManager* inventoryManager = InventoryManager::GetInstance();
	if (!inventoryManager) return;
	
	const std::map<UINT, UINT>* recipe = inventoryManager->GetCraftingRecipe(m_selectedToolID);
	if (!recipe || recipe->empty()) {
		return;
	}
	int ingredientIndex = 0;

	// 재료 정보 표시
	for (const auto& ingredient : *recipe) {
		if (ingredientIndex >= 2) break; // 최대 2개만 표시

		GameObjectID ingredientID = static_cast<GameObjectID>(ingredient.first);
		UINT count = ingredient.second;

		// 재료 이미지 업데이트 (로드 후 목표 크기로 스케일 적용)
		if (ingredientIndex < static_cast<int>(m_ingredientImages.size())) {
			UIImage* image = m_ingredientImages[ingredientIndex];
			if (image) {
				std::wstring imagePath = GetIngredientImagePath(ingredientID);
				if (!imagePath.empty()) {
					image->LoadSprite(imagePath);
					if (const ComponentElement::Image* img = image->GetImageComponent())
						img->SetDisplaySizeProportional(m_ingredientImageSize);
					image->SetActive(true);
				}
			}
		}

		// 재료 텍스트 업데이트 (수량 x 이름)
		if (ingredientIndex < static_cast<int>(m_ingredientTexts.size())) {
			UIText* text = m_ingredientTexts[ingredientIndex];
			if (text) {
				std::wstring textStr = std::to_wstring(count) + L" x " + GetIngredientDisplayName(ingredientID);
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

bool CraftingUI::TryCraftSelectedTool(Player* player)
{
	if (m_selectedToolID == GOID_NONE || !player) {
		return false;
	}

	InventoryManager* inventoryManager = InventoryManager::GetInstance();
	if (!inventoryManager) {
		return false;
	}

	return inventoryManager->TryCraftItem(player, m_selectedToolID);
}

std::wstring CraftingUI::GetIngredientImagePath(GameObjectID ingredientID)
{
	// ObjectResourceTable에서 재료 이미지 경로 찾기
	for (size_t i = 0; i < ResourcePathUtils::ObjectResourceCount; ++i) {
		const auto& entry = ResourcePathUtils::ObjectResourceTable[i];
		if (entry.id == ingredientID) {
			return ResourcePathUtils::BuildResourcePath(entry.baseDir, entry.imageName);
		}
	}
	return L"";
}

std::wstring CraftingUI::GetIngredientDisplayName(GameObjectID ingredientID)
{
	switch (ingredientID) {
		case GOID_ITEM_NORMAL_TWIGS:       return L"나뭇가지";
		case GOID_ITEM_NORMAL_TREE_LOG:     return L"통나무";
		case GOID_ITEM_NORMAL_ROCK:        return L"돌";
		case GOID_ITEM_GOLD_ROCK:          return L"금";
		case GOID_ITEM_CUT_NORMAL_GRASS:   return L"풀";
		case GOID_ITEM_BERRY:             return L"열매";
		case GOID_ITEM_MEAT:               return L"고기";
		case GOID_ITEM_ROPE:               return L"밧줄";
		default:                           return L"재료 " + std::to_wstring(ingredientID);
	}
}

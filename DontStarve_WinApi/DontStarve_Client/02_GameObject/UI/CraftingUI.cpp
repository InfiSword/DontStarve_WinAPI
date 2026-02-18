#include "99_Default/pch.h"
#include "CraftingUI.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UIText.h"
#include "../../01_Manager/UIManager/UIManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../Entity/Player/Player.h"
#include "../Component/Transform/RectTransform.h"

CraftingUI::CraftingUI()
	: UIElement(GOBJ_UI, GOID_CRAFT_BAR, L"", L"", true, false),
	m_craftBar(nullptr),
	m_craftIcon(nullptr),
	m_craftButton(nullptr),
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
	m_ingredientImageSize(32.0f),
	m_ingredientTextHeight(20.0f),
	m_ingredientSpacing(50.0f),
	m_ingredientStartY(0.0f) // 생성자에서 계산
{
	// 제작 가능한 도구 목록 초기화
	m_availableTools = {
		GOID_TOOL_GOLDEN_SCYTHE,
		GOID_TOOL_HAM_BAT,
		GOID_TOOL_PICKAXE,
		GOID_TOOL_RED_AXE,
		GOID_TOOL_SPEAR,
		GOID_TOOL_SWAP_AXE,
		GOID_TOOL_SWAP_SPEAR,
		GOID_TOOL_TORCH
	};

	// 계산된 값들 초기화
	m_toolPanelOffsetX = m_craftBarWidth;
	m_toolButtonStartY = -(m_craftBarHeight * 0.5f);
	m_ingredientStartY = m_craftBarHeight * 0.5f - 200.0f;
}

CraftingUI::~CraftingUI()
{
	Release();
}

void CraftingUI::Init()
{
	LoadCraftingRecipes();
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

	// 각 도구에 대한 버튼 생성
	GameObjectID toolButtonIDs[] = {
		GOID_CRAFT_TOOL_GOLDEN_SCYTHE,
		GOID_CRAFT_TOOL_HAM_BAT,
		GOID_CRAFT_TOOL_PICKAXE,
		GOID_CRAFT_TOOL_RED_AXE,
		GOID_CRAFT_TOOL_SPEAR,
		GOID_CRAFT_TOOL_SWAP_AXE,
		GOID_CRAFT_TOOL_SWAP_SPEAR,
		GOID_CRAFT_TOOL_TORCH
	};

	std::wstring toolImagePaths[] = {
		L"Resource/Objects/Tools/Golden_Scythe_02.png",
		L"Resource/Objects/Tools/hamBat_01.png",
		L"Resource/Objects/Tools/pickaxe-0.png",
		L"Resource/Objects/Tools/Red_Axe_02.png",
		L"Resource/Objects/Tools/spear_03.png",
		L"Resource/Objects/Tools/swap_axe-0.png",
		L"Resource/Objects/Tools/swap_spear_wathgrithr_lightning-5.png",
		L"Resource/Objects/Tools/torch.png"
	};

	// Tool 버튼들을 CraftBar 오른쪽에 3열로 배치 (겹치지 않도록)
	const float startX = m_toolPanelOffsetX + m_toolButtonSize * 0.5f; // 첫 번째 열의 중앙 X

	for (size_t i = 0; i < m_availableTools.size() && i < 8; ++i) {
		std::shared_ptr<Sprite> toolNormalSprite = resourceManager->LoadSprite(toolImagePaths[i]);
		std::shared_ptr<Sprite> toolHoverSprite = resourceManager->LoadSprite(toolImagePaths[i]);

		// 3열 그리드 계산 (CraftBar 오른쪽에 위치)
		int row = static_cast<int>(i / m_columnsPerRow);
		int col = static_cast<int>(i % m_columnsPerRow);
		
		float xPos = startX + col * (m_toolButtonSize + m_toolButtonSpacing);
		float yPos = m_toolButtonStartY + row * (m_toolButtonSize + m_toolButtonSpacing);

		UIButton* toolButton = new UIButton(
			toolButtonIDs[i],
			m_toolButtonSize,
			m_toolButtonSize,
			toolNormalSprite,
			toolHoverSprite,
			0.0f, 0.5f,  // anchorMin (왼쪽 중앙 - CraftBar와 동일)
			0.0f, 0.5f,  // anchorMax (왼쪽 중앙)
			xPos, yPos
		);
		// UIButton 생성자에서 이미 pivot (0.5, 0.5)로 설정됨

		// 초기에는 숨김
		toolButton->SetActive(false);

		// 클릭 콜백 설정
		GameObjectID toolID = m_availableTools[i];
		toolButton->SetOnClickCallback([this, toolID]() {
			SelectTool(toolID);
		});

		m_toolButtons.push_back(toolButton);
		uiManager->AddUIButton(toolButton);
	}
}

void CraftingUI::CreateCraftButton()
{
	UIManager* uiManager = UIManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	if (!uiManager || !resourceManager) return;

	// 크래프팅 버튼 생성 (재료 표시 아래에 배치)
	std::shared_ptr<Sprite> buttonNormalSprite = resourceManager->LoadSprite(L"Resource/UI/frontscreen.png");
	std::shared_ptr<Sprite> buttonHoverSprite = resourceManager->LoadSprite(L"Resource/UI/HighLight_frontscreen.png");

	// 크래프팅 버튼을 CraftBar 내부 상단(아이콘 아래쪽)에 세로 레이아웃처럼 배치
	m_craftButton = new UIButton(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 300), // 임시 ID
		m_craftButtonWidth,
		m_craftButtonHeight,
		buttonNormalSprite,
		buttonHoverSprite,
		0.0f, 0.5f, // anchorMin (왼쪽 중앙 - CraftBar와 동일)
		0.0f, 0.5f, // anchorMax (왼쪽 중앙)
		m_craftBarWidth * 0.5f, -(m_craftBarHeight * 0.5f - m_craftButtonOffsetFromTop - m_craftButtonHeight * 0.5f) // CraftBar 내부 상단에서 아래로 offset
	);

	// UIButton 생성자에서 이미 pivot (0.5, 0.5)로 설정됨

	// 초기에는 숨김
	m_craftButton->SetActive(false);

	// 클릭 콜백 설정
	m_craftButton->SetOnClickCallback([this]() {
		// 플레이어 가져오기 (ObjectManager를 통해)
		ObjectManager* objectManager = ObjectManager::GetInstance();
		Player* player = objectManager ? objectManager->GetPlayer() : nullptr;
		if (player) {
			TryCraftSelectedTool(player);
		}
	});

	uiManager->AddUIButton(m_craftButton);

	// 크래프팅 버튼 텍스트 (버튼과 동일한 위치)
	UIText* craftButtonText = new UIText(
		static_cast<GameObjectID>(GOID_CRAFT_BAR + 301), // 임시 ID
		m_craftButtonWidth,
		m_craftButtonHeight,
		L"제작",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.3f,
		L"맑은 고딕",
		14.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.0f, 0.5f, // anchorMin
		0.0f, 0.5f, // anchorMax
		m_craftBarWidth * 0.5f, -(m_craftBarHeight * 0.5f - m_craftButtonOffsetFromTop - m_craftButtonHeight * 0.5f) // 버튼과 동일한 위치
	);

	// UIText 생성자에서 이미 pivot (0.5, 0.5)로 설정됨

	craftButtonText->SetActive(false);
	m_ingredientTexts.push_back(craftButtonText); // 임시로 텍스트 리스트에 추가
	uiManager->AddUIText(craftButtonText);
}

void CraftingUI::CreateIngredientDisplay()
{
	UIManager* uiManager = UIManager::GetInstance();
	if (!uiManager) return;

	// 재료 이미지와 텍스트 생성 (최대 2개 재료)
	// CraftBar 내부에 상대적으로 배치
	for (int i = 0; i < 2; ++i) {
		float yPos = m_ingredientStartY - i * m_ingredientSpacing;
		
		// 재료 이미지
		UIImage* ingredientImage = new UIImage(
			static_cast<GameObjectID>(GOID_CRAFT_BAR + 100 + i), // 임시 ID
			m_ingredientImageSize,
			m_ingredientImageSize,
			LAYER_UI_FOREGROUND,
			L"",        // 초기에는 빈 경로
			0.1f,
			0.0f, 0.5f, // anchorMin (왼쪽 중앙 - CraftBar와 동일)
			0.0f, 0.5f, // anchorMax (왼쪽 중앙)
			m_craftBarWidth * 0.5f, yPos // CraftBar 내부 중앙
		);

		// UIImage 생성자에서 이미 pivot (0.5, 0.5)로 설정됨

		ingredientImage->SetActive(false); // 초기에는 숨김
		m_ingredientImages.push_back(ingredientImage);
		uiManager->AddUIImage(ingredientImage);

		// 재료 설명 텍스트 (이미지 아래에 배치)
		UIText* ingredientText = new UIText(
			static_cast<GameObjectID>(GOID_CRAFT_BAR + 200 + i), // 임시 ID
			m_craftBarWidth - 20.0f,     // 너비 (CraftBar 너비에 맞춤)
			m_ingredientTextHeight,
			L"",        // 초기 텍스트
			Gdiplus::Color::White,
			LAYER_UI_FOREGROUND,
			0.2f,
			L"맑은 고딕",
			12.0f,
			Gdiplus::StringAlignmentCenter,
			Gdiplus::StringAlignmentCenter,
			0.0f, 0.5f, // anchorMin
			0.0f, 0.5f, // anchorMax
			m_craftBarWidth * 0.5f, yPos + m_ingredientImageSize * 0.5f + m_ingredientTextHeight * 0.5f + 5.0f // 이미지 아래
		);

		// UIText 생성자에서 이미 pivot (0.5, 0.5)로 설정됨

		ingredientText->SetActive(false); // 초기에는 숨김
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
}

void CraftingUI::SelectTool(GameObjectID toolID)
{
	m_selectedToolID = toolID;
	UpdateIngredientDisplay();
	
	// 크래프팅 버튼 표시
	if (m_craftButton) {
		m_craftButton->SetActive(true);
	}
	// 크래프팅 버튼 텍스트도 표시 (재료 텍스트 다음에 추가된 버튼 텍스트)
	if (m_ingredientTexts.size() > 2 && m_ingredientTexts[2]) {
		m_ingredientTexts[2]->SetActive(true);
	}
}

void CraftingUI::UpdateIngredientDisplay()
{
	if (m_selectedToolID == GOID_NONE) {
		// 재료 표시 숨김
		for (auto* image : m_ingredientImages) {
			if (image) image->SetActive(false);
		}
		for (auto* text : m_ingredientTexts) {
			if (text) text->SetActive(false);
		}
		return;
	}

	// 선택된 도구의 레시피 가져오기
	auto recipeIt = m_craftingRecipes.find(m_selectedToolID);
	if (recipeIt == m_craftingRecipes.end()) {
		return;
	}

	const auto& recipe = recipeIt->second;
	int ingredientIndex = 0;

	// 재료 정보 표시
	for (const auto& ingredient : recipe) {
		if (ingredientIndex >= 2) break; // 최대 2개만 표시

		GameObjectID ingredientID = static_cast<GameObjectID>(ingredient.first);
		UINT count = ingredient.second;

		// 재료 이미지 업데이트
		if (ingredientIndex < static_cast<int>(m_ingredientImages.size())) {
			UIImage* image = m_ingredientImages[ingredientIndex];
			if (image) {
				std::wstring imagePath = GetIngredientImagePath(ingredientID);
				if (!imagePath.empty()) {
					image->LoadSprite(imagePath);
					image->SetActive(true);
				}
			}
		}

		// 재료 텍스트 업데이트
		if (ingredientIndex < static_cast<int>(m_ingredientTexts.size())) {
			UIText* text = m_ingredientTexts[ingredientIndex];
			if (text) {
				std::wstring textStr = std::to_wstring(count) + L"x ";
				// 재료 이름 추가 (임시로 ID 사용)
				textStr += L"재료 " + std::to_wstring(ingredientID);
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

void CraftingUI::LoadCraftingRecipes()
{
	m_craftingRecipes.clear();

	// Struct.h의 CraftingRecipeTable에서 레시피 로드
	for (size_t i = 0; i < ResourcePathUtils::CraftingRecipeCount; ++i) {
		const auto& recipe = ResourcePathUtils::CraftingRecipeTable[i];
		
		std::map<UINT, UINT> ingredientMap;
		ingredientMap[recipe.ingredient1ID] = recipe.ingredient1Count;
		if (recipe.ingredient2ID != GOID_NONE) {
			ingredientMap[recipe.ingredient2ID] = recipe.ingredient2Count;
		}

		m_craftingRecipes[recipe.toolID] = ingredientMap;
	}
}

std::wstring CraftingUI::GetIngredientImagePath(GameObjectID ingredientID)
{
	// ObjectResourceTable에서 재료 이미지 경로 찾기
	for (size_t i = 0; i < ResourcePathUtils::ObjectResourceCount; ++i) {
		const auto& entry = ResourcePathUtils::ObjectResourceTable[i];
		if (entry.id == ingredientID) {
			std::wstring fullPath = entry.baseDir;
			if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
				fullPath += L"\\";
			}
			fullPath += entry.imageName;
			return fullPath;
		}
	}
	return L"";
}

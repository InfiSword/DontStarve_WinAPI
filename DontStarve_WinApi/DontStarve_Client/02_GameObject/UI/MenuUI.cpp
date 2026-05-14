#include "99_Default/pch.h"
#include "MenuUI.h"
#include "CraftingRecipe.h"
#include "UIImage.h"
#include "UIButton.h"
#include "UIText.h"
#include "../Entity/Combatant.h"
#include "../Entity/Player/Player.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../01_Manager/SceneManager/SceneManager.h"

MenuUI::MenuUI()
    : UIElement(GOID_UI_MENU, L"", L"", true, false)
{
    // 데이터 초기화
    for (size_t i = 0; i < CraftingRecipeCount; ++i) {
        GameObjectID id = CraftingRecipeTable[i].toolID;
        if (id >= GOID_TOOL_GOLDEN_PICKAXE && id <= GOID_TOOL_HAMMER)
            m_availableTools.push_back(id);
    }
    m_availableCreateItems = { GOID_ITEM_CUT_NORMAL_STONE, GOID_ITEM_ROPE, GOID_ITEM_WOOD_2 };
    m_availableCookItems = { GOID_ITEM_COOKED_MONSTER_MEAT, GOID_ITEM_COOKED_SMALL_MEAT, GOID_ITEM_COOKED_MEAT };

    // 레이아웃 상수 계산
    const float menuIconStartY = 140.0f;
    const float menuIconSpacing = 72.0f;
    for (int i = 0; i < 5; ++i) m_menuIconY[i] = menuIconStartY - i * menuIconSpacing;

    const float palettePadding = 48.0f;
    const int maxRows = (static_cast<int>(m_availableTools.size()) + m_columnsPerRow - 1) / m_columnsPerRow;
    const float toolPanelH = maxRows * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing + palettePadding * 2.0f;
    const float toolButtonStartY = -(m_craftBarHeight * 0.5f);
    const float toolPanelCY = toolButtonStartY + toolPanelH * 0.5f - palettePadding;
    const float paletteBgBottomY = toolPanelCY + toolPanelH * 0.5f - 30.0f;

    m_ingredientStartY = paletteBgBottomY + m_ingredientImageSize * 0.5f + 20.0f;
    m_ingredientPanelCenterX = m_craftBarWidth + (m_columnsPerRow * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing) * 0.5f + 45.0f;
}

MenuUI::~MenuUI() { Release(); }

void MenuUI::Init() { InitializeCraftingUI(); }

void MenuUI::Update(float deltaTime) { GameObject::Update(deltaTime); }

void MenuUI::Release()
{
    ObjectManager* objMgr = ObjectManager::GetInstance();
    for (auto* obj : m_managedObjects) {
        if (obj) {
            objMgr->RemoveGameObject(obj);
        }
    }
    m_managedObjects.clear();
    UIElement::Release();
}

void MenuUI::InitializeCraftingUI()
{
    CreateMenuBar();
    CreatePalettes();
    CreateIngredientUI();
    CreateBossChallengeUI();
    CreateEditUI();
    ClearAllPanels();
}

void MenuUI::CreateMenuBar()
{
    const float iconX = 40.0f;
    auto* objMgr = ObjectManager::GetInstance();

    // 배경을 먼저 생성해 비최적화 즉시 렌더에서도 아이콘이 위에 오도록 한다.
	m_managedObjects.push_back(objMgr->CreateImage(GOID_UI_MENU, m_craftBarWidth, m_craftBarHeight, LAYER_UI_BACKGROUND, L"Resource/UI/CraftBar.png", 0.0f, 0.0f, 0.5f, 0.0f, 0.5f, 50.0f, 0.0f));

	m_managedObjects.push_back(objMgr->CreateButton(GOID_UI_BUTTON, m_iconSize, m_iconSize, L"Resource/UI/CraftIcon.png", L"Resource/UI/CraftIcon.png", 0.0f, 0.5f, 0.0f, 0.5f, iconX, m_menuIconY[4], [this]() { ToggleToolList(); }));
	m_managedObjects.push_back(objMgr->CreateButton(GOID_UI_BUTTON, m_iconSize + 20, m_iconSize + 20, L"Resource/UI/CreateIcon.png", L"Resource/UI/CreateIcon.png", 0.0f, 0.5f, 0.0f, 0.5f, iconX, m_menuIconY[3], [this]() { ToggleCreateList(); }));
	m_managedObjects.push_back(objMgr->CreateButton(GOID_UI_BUTTON, m_iconSize, m_iconSize, L"Resource/UI/CookIcon.png", L"Resource/UI/CookIcon.png", 0.0f, 0.5f, 0.0f, 0.5f, iconX, m_menuIconY[2], [this]() { ToggleCookList(); }));
	m_managedObjects.push_back(objMgr->CreateButton(GOID_UI_BUTTON, m_iconSize, m_iconSize, L"Resource/UI/BattleIcon.png", L"Resource/UI/BattleIcon.png", 0.0f, 0.5f, 0.0f, 0.5f, iconX, m_menuIconY[1], [this]() { ToggleBossPanel(); }));
    m_managedObjects.push_back(objMgr->CreateButton(GOID_UI_BUTTON, m_iconSize, m_iconSize, L"Resource/UI/EditIcon.png", L"Resource/UI/EditIcon.png", 0.0f, 0.5f, 0.0f, 0.5f, iconX, m_menuIconY[0], [this]() { ToggleEditPanel(); }));
}

void MenuUI::CreatePalettes()
{
    const float startX = m_craftBarWidth + m_toolButtonSize * 0.5f + 45.0f;
    const float startY = -(m_craftBarHeight * 0.5f);
    auto* objMgr = ObjectManager::GetInstance();

  const float palettePadding = 48.0f;
  const int maxRows = (static_cast<int>(m_availableTools.size()) + m_columnsPerRow - 1) / m_columnsPerRow;
  const float toolPanelW = m_columnsPerRow * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing + palettePadding * 2.0f;
  const float toolPanelH = maxRows * (m_toolButtonSize + m_toolButtonSpacing) - m_toolButtonSpacing + palettePadding * 2.0f;
  const float toolButtonStartY = -(m_craftBarHeight * 0.5f);
  const float toolPanelCX = m_craftBarWidth + toolPanelW * 0.5f - palettePadding;
  const float toolPanelCY = toolButtonStartY + toolPanelH * 0.5f - palettePadding;

    m_toolPanelBg = AddManaged(objMgr->CreateImage(GOID_UI_IMAGE, toolPanelW, toolPanelH, LAYER_UI_BACKGROUND, L"Resource/UI/CraftUI_PaletteBG.png", -0.05f, 0.0f, 0.5f, 0.0f, 0.5f, toolPanelCX + 20.0f, toolPanelCY - 30.0f));

    auto CreateGrid = [&](const std::vector<GameObjectID>& items, std::vector<GameObject*>& group) {
        for (size_t i = 0; i < items.size(); ++i) {
            float x = startX + (i % m_columnsPerRow) * (m_toolButtonSize + m_toolButtonSpacing);
            float y = startY + (i / m_columnsPerRow) * (m_toolButtonSize + m_toolButtonSpacing);
            std::wstring path = ResourceUtils::GetResourceImagePath(items[i]);
            UIButton* btn = AddManaged(objMgr->CreateButton(GOID_UI_BUTTON, m_toolButtonSize, m_toolButtonSize, path, path, 0.0f, 0.5f, 0.0f, 0.5f, x, y, [this, id = items[i]]() { SelectCraftItem(id); }));
            if (btn) {
                if (auto* img = btn->GetImageComponent()) img->SetDisplaySizeProportional(m_toolButtonSize);
                group.push_back(btn);
            }
        }
    };

    CreateGrid(m_availableTools, m_toolGroup);
    CreateGrid(m_availableCreateItems, m_createGroup);
    CreateGrid(m_availableCookItems, m_cookGroup);
}

void MenuUI::CreateIngredientUI()
{
    const float ingPadding = 16.0f;
    const float ingPanelW = 2.0f * m_ingredientImageSize + m_ingredientSpacing + ingPadding * 2.0f + 50.0f;
    const float craftButtonY = m_ingredientStartY + m_ingredientImageSize * 0.5f + 48.0f;
    const float ingTop = m_ingredientStartY - m_ingredientImageSize * 0.5f - ingPadding;
    const float ingBot = craftButtonY + 20.0f + ingPadding;
    const float ingPanelH = ingBot - ingTop;
    auto* objMgr = ObjectManager::GetInstance();

    m_ingredientPanelBg = AddManaged(objMgr->CreateImage(GOID_UI_IMAGE, ingPanelW, ingPanelH, LAYER_UI_BACKGROUND, L"Resource/UI/CraftUI_IngredientBG.png", -0.05f, 0.0f, 0.5f, 0.0f, 0.5f, m_ingredientPanelCenterX, ingTop + ingPanelH * 0.5f));
    
    m_craftingItemNameText = AddManaged(objMgr->CreateText(GOID_UI_TEXT, 250.0f, 30.0f, L"", Gdiplus::Color::White, 18.0f, Gdiplus::FontStyleRegular, 0.0f, 0.5f, 0.0f, 0.5f, m_ingredientPanelCenterX, m_ingredientStartY - m_ingredientImageSize * 0.5f));
    m_ingredientGroup.push_back(m_craftingItemNameText);

    for (int i = 0; i < 2; ++i) {
        float x = m_ingredientPanelCenterX + (i - 0.5f) * (m_ingredientImageSize + m_ingredientSpacing);
        m_ingredientSlots[i].image = AddManaged(objMgr->CreateImage(GOID_UI_IMAGE, m_ingredientImageSize, m_ingredientImageSize, LAYER_UI_FOREGROUND, L"", 0.1f, 0.0f, 0.5f, 0.0f, 0.5f, x, m_ingredientStartY));
        m_ingredientSlots[i].text = AddManaged(objMgr->CreateText(GOID_UI_TEXT, 150.0f, 28.0f, L"", Gdiplus::Color::White, 13.0f, Gdiplus::FontStyleRegular, 0.0f, 0.5f, 0.0f, 0.5f, x, m_ingredientStartY + m_ingredientImageSize * 0.5f + 14.0f));
        m_ingredientGroup.push_back(m_ingredientSlots[i].image);
        m_ingredientGroup.push_back(m_ingredientSlots[i].text);
    }

    m_craftButton = AddManaged(objMgr->CreateButton(GOID_UI_BUTTON, 120.0f, 40.0f, L"Resource/UI/frontscreen.png", L"Resource/UI/HighLight_frontscreen.png", 0.0f, 0.5f, 0.0f, 0.5f, m_ingredientPanelCenterX, craftButtonY, [this]() {
        Player* player = ObjectManager::GetInstance()->GetPlayer();
        if (player) TryCraftSelectedItem(player);
    }));
    m_craftButtonText = AddManaged(objMgr->CreateText(GOID_UI_TEXT, 120.0f, 40.0f, L"제작하기", Gdiplus::Color::Black, 14.0f, Gdiplus::FontStyleRegular, 0.0f, 0.5f, 0.0f, 0.5f, m_ingredientPanelCenterX, craftButtonY, 100.0f));
    m_ingredientGroup.push_back(m_craftButton);
    m_ingredientGroup.push_back(m_craftButtonText);
}

void MenuUI::CreateBossChallengeUI()
{
    auto* objMgr = ObjectManager::GetInstance();
    m_bossOverlay = AddManaged(objMgr->CreateImage(GOID_UI_IMAGE, 1920.0f, 1080.0f, LAYER_UI_BACKGROUND, L"Resource/UI/frontscreen.png", -0.1f, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 0.0f));
    m_bossOverlay->SetTintColor(0, 0, 0, 180);
    m_bossGroup.push_back(m_bossOverlay);

    auto SetupBossPanel = [&](int idx, GameObjectID bossID, const std::wstring& spritePath, float x) {
        m_bossPanels[idx].id = bossID;
        m_bossPanels[idx].panel = AddManaged(objMgr->CreateButton(GOID_UI_BUTTON, 300.0f, 400.0f, spritePath, spritePath, 0.5f, 0.5f, 0.5f, 0.5f, x, 0.0f, [this, bossID]() { SelectBoss(bossID); }));
        m_bossPanels[idx].clearText = AddManaged(objMgr->CreateText(GOID_UI_TEXT, 300.0f, 60.0f, L"CLEAR", Gdiplus::Color::Red, 48.0f, Gdiplus::FontStyleRegular, 0.5f, 0.5f, 0.5f, 0.5f, x, 0.0f));
        m_bossGroup.push_back(m_bossPanels[idx].panel);
        m_bossGroup.push_back(m_bossPanels[idx].clearText);
    };

    SetupBossPanel(0, GOID_MONSTER_HOUNDDOG, L"Resource/UI/BossBattleHound.png", -200.0f);
    SetupBossPanel(1, GOID_MONSTER_QUEEN_SPIDER, L"Resource/UI/BossBattleSpiderQueen.png", 200.0f);

    m_bossChallengeButton = AddManaged(objMgr->CreateButton(GOID_UI_BUTTON, 200.0f, 60.0f, L"Resource/UI/frontscreen.png", L"Resource/UI/HighLight_frontscreen.png", 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 280.0f, [this]() { TryChallengeBoss(); }));
    m_bossChallengeButtonText = AddManaged(objMgr->CreateText(GOID_UI_TEXT, 200.0f, 60.0f, L"보스 도전", Gdiplus::Color::Black, 18.0f, Gdiplus::FontStyleRegular, 0.5f, 0.5f, 0.5f, 0.5f, 0.0f, 280.0f, 100.0f));
    m_bossGroup.push_back(m_bossChallengeButton);
    m_bossGroup.push_back(m_bossChallengeButtonText);
}

void MenuUI::ClearAllPanels()
{
    m_currentPanel = PanelType::NONE;
    for (auto* obj : m_toolGroup) obj->SetActive(false);
    for (auto* obj : m_createGroup) obj->SetActive(false);
    for (auto* obj : m_cookGroup) obj->SetActive(false);
    for (auto* obj : m_bossGroup) obj->SetActive(false);
    for (auto* obj : m_ingredientGroup) obj->SetActive(false);
    for (auto* obj : m_editGroup) obj->SetActive(false);
    if (m_toolPanelBg) m_toolPanelBg->SetActive(false);
    if (m_ingredientPanelBg) m_ingredientPanelBg->SetActive(false);
    m_selectedCraftItemID = GOID_NONE;
    m_selectedBossID = GOID_NONE;
}

void MenuUI::ToggleToolList() 
{
	bool open = (m_currentPanel != PanelType::TOOL); 
	ClearAllPanels();
	if (open) 
	{ 
		m_currentPanel = PanelType::TOOL; 
		for (auto* obj : m_toolGroup) obj->SetActive(true); 
		m_toolPanelBg->SetActive(true); 
	} 
}
void MenuUI::ToggleCreateList() { bool open = (m_currentPanel != PanelType::CREATE); ClearAllPanels(); if (open) { m_currentPanel = PanelType::CREATE; for (auto* obj : m_createGroup) obj->SetActive(true); m_toolPanelBg->SetActive(true); } }
void MenuUI::ToggleCookList() { bool open = (m_currentPanel != PanelType::COOK); ClearAllPanels(); if (open) { m_currentPanel = PanelType::COOK; for (auto* obj : m_cookGroup) obj->SetActive(true); m_toolPanelBg->SetActive(true); } }
void MenuUI::ToggleBossPanel() { bool open = (m_currentPanel != PanelType::BOSS); ClearAllPanels(); if (open) { m_currentPanel = PanelType::BOSS; for (auto* obj : m_bossGroup) obj->SetActive(true); UpdateBossPanelHighlight(); } }

void MenuUI::ToggleEditPanel()
{
	bool open = (m_currentPanel != PanelType::Editor);
	ClearAllPanels();
	if (open)
	{
		m_currentPanel = PanelType::Editor;
		for (auto* obj : m_editGroup) obj->SetActive(true);
	}
}

void MenuUI::SelectCraftItem(GameObjectID itemID)
{
    if (m_selectedCraftItemID == itemID) { m_selectedCraftItemID = GOID_NONE; m_ingredientPanelBg->SetActive(false); for (auto* obj : m_ingredientGroup) obj->SetActive(false); return; }
    m_selectedCraftItemID = itemID;
    m_ingredientPanelBg->SetActive(true);
    for (auto* obj : m_ingredientGroup) obj->SetActive(true);
    UpdateIngredientDisplay();
}

void MenuUI::UpdateIngredientDisplay()
{
    if (m_selectedCraftItemID == GOID_NONE) return;
    m_craftingItemNameText->SetText(ResourceUtils::GetResourceDisplayName(m_selectedCraftItemID));
    
    auto* invMgr = InventoryManager::GetInstance();
    const auto* recipe = invMgr ? invMgr->GetCraftingRecipe(m_selectedCraftItemID) : nullptr;
    int idx = 0;
    if (recipe) {
        for (const auto& item : *recipe) {
            if (idx >= 2) break;
            m_ingredientSlots[idx].image->LoadSprite(ResourceUtils::GetResourceImagePath(static_cast<GameObjectID>(item.first)));
            if (auto* imgComp = m_ingredientSlots[idx].image->GetImageComponent()) {
                imgComp->SetDisplaySizeProportional(m_ingredientImageSize);
            }
            m_ingredientSlots[idx].image->SetActive(true);
            m_ingredientSlots[idx].text->SetText(std::to_wstring(item.second) + L" x " + ResourceUtils::GetResourceDisplayName(static_cast<GameObjectID>(item.first)));
            m_ingredientSlots[idx].text->SetActive(true);
            idx++;
        }
    }
    for (; idx < 2; ++idx) { m_ingredientSlots[idx].image->SetActive(false); m_ingredientSlots[idx].text->SetActive(false); }
}

bool MenuUI::TryCraftSelectedItem(Player* player)
{
    if (m_selectedCraftItemID == GOID_NONE || !player) return false;
    bool success = InventoryManager::GetInstance()->TryCraftItem(player, m_selectedCraftItemID);
    if (success) UpdateIngredientDisplay();
    return success;
}

void MenuUI::SelectBoss(GameObjectID bossID)
{
    if (bossID == GOID_MONSTER_QUEEN_SPIDER && !GameProgressManager::GetInstance()->IsSceneCleared(SCENE_GAME_HOUND_FOREST)) return;
    m_selectedBossID = bossID;
    UpdateBossPanelHighlight();
}

void MenuUI::TryChallengeBoss()
{
    if (m_selectedBossID == GOID_NONE) 
        m_selectedBossID = GameProgressManager::GetInstance()->IsSceneCleared(SCENE_GAME_HOUND_FOREST) ? GOID_MONSTER_QUEEN_SPIDER : GOID_MONSTER_HOUNDDOG;
    
    if (m_selectedBossID == GOID_MONSTER_QUEEN_SPIDER && !GameProgressManager::GetInstance()->IsSceneCleared(SCENE_GAME_HOUND_FOREST)) return;

    Player* player = ObjectManager::GetInstance()->GetPlayer();
    GameObjectID charID = player ? player->GetID() : GOID_NONE;
    SceneManager::GetInstance()->LoadGameScene(m_selectedBossID == GOID_MONSTER_HOUNDDOG ? SCENE_GAME_HOUND_FOREST : SCENE_GAME_SPIDER_QUEEN_HOUSE, charID);
    ClearAllPanels();
}

void MenuUI::UpdateBossPanelHighlight()
{
    bool cleared[2] = { 
        GameProgressManager::GetInstance()->IsSceneCleared(SCENE_GAME_HOUND_FOREST), 
        GameProgressManager::GetInstance()->IsSceneCleared(SCENE_GAME_SPIDER_QUEEN_HOUSE) 
    };

    for (int i = 0; i < 2; ++i) {
        bool locked = (i == 1 && !cleared[0]);
        m_bossPanels[i].panel->SetDisabled(cleared[i] || locked);
        m_bossPanels[i].clearText->SetActive(cleared[i]);
        if (auto* img = const_cast<ComponentElement::Image*>(m_bossPanels[i].panel->GetImageComponent())) {
            if (m_selectedBossID == m_bossPanels[i].id) img->SetTintColor(255, 180, 180, 255);
            else img->SetTintColor(cleared[i] ? 100 : (locked ? 120 : 200), cleared[i] ? 100 : (locked ? 120 : 200), cleared[i] ? 100 : (locked ? 120 : 200), 255);
        }
        m_bossPanels[i].panel->GetRectTransform()->SetScale(m_selectedBossID == m_bossPanels[i].id ? 1.1f : 1.0f, m_selectedBossID == m_bossPanels[i].id ? 1.1f : 1.0f);
    }
}

void MenuUI::CreateEditUI()
{
    auto* objMgr = ObjectManager::GetInstance();
    
    // Edit 메뉴 배경 (화면 중앙)
    float bgX = 0.0f;
    float bgY = 0.0f;
    UIImage* editBg = AddManaged(objMgr->CreateImage(GOID_UI_IMAGE, 400.0f, 500.0f, LAYER_UI_BACKGROUND, L"Resource/UI/EditorMenu.png", -0.05f, 0.5f, 0.5f, 0.5f, 0.5f, bgX, bgY));
    m_editGroup.push_back(editBg);

    float btnW = 260.0f;
    float btnH = 70.0f;
    float startY = 100.0f;
    float spacingY = 100.0f;

    // 타이틀 화면으로
    UIButton* titleBtn = AddManaged(objMgr->CreateButton(GOID_UI_BUTTON, btnW, btnH, L"Resource/UI/frontscreen.png", L"Resource/UI/HighLight_frontscreen.png", 0.5f, 0.5f, 0.5f, 0.5f, bgX, startY + 50.f, []() {
        SceneManager::GetInstance()->LoadTitleScene();
    }));
    UIText* titleText = AddManaged(objMgr->CreateText(GOID_UI_TEXT, btnW, btnH, L"타이틀 화면으로", Gdiplus::Color::Black, 20.0f, Gdiplus::FontStyleRegular, 0.5f, 0.5f, 0.5f, 0.5f, bgX, startY + 50.f, 100.0f));
    m_editGroup.push_back(titleBtn);
    m_editGroup.push_back(titleText);

    // 적 디버그 표시 체크박스
    UIButton* debugBtn = AddManaged(objMgr->CreateButton(GOID_UI_BUTTON, btnW, btnH, L"Resource/UI/frontscreen.png", L"Resource/UI/HighLight_frontscreen.png", 0.5f, 0.5f, 0.5f, 0.5f, bgX, startY - spacingY, [this]() {
        Combatant::s_bShowAttackGizmo = !Combatant::s_bShowAttackGizmo;
        if (m_debugToggleStatusText) {
            m_debugToggleStatusText->SetText(Combatant::s_bShowAttackGizmo ? L"On" : L"Off");
            m_debugToggleStatusText->SetColor(Combatant::s_bShowAttackGizmo ? Gdiplus::Color::Green : Gdiplus::Color::Red);
        }
    }));
    UIText* debugText = AddManaged(objMgr->CreateText(GOID_UI_TEXT, btnW, btnH, L"공격 범위 표시 ", Gdiplus::Color::Black, 18.0f, Gdiplus::FontStyleRegular, 0.5f, 0.5f, 0.5f, 0.5f, bgX, startY - spacingY, 100.0f));
    
    // On/Off 상태 텍스트
	m_debugToggleStatusBG = AddManaged(objMgr->CreateImage(GOID_UI_IMAGE, 60.0f, 30.0f, LAYER_UI_FOREGROUND, L"Resource/UI/frontscreen.png", 0.1f, 0.5f, 0.5f, 0.5f, 0.5f, bgX, startY - spacingY + 50.f));
    m_debugToggleStatusText = AddManaged(objMgr->CreateText(GOID_UI_TEXT, 80.0f, btnH, Combatant::s_bShowAttackGizmo ? L"On" : L"Off", Combatant::s_bShowAttackGizmo ? Gdiplus::Color::Green : Gdiplus::Color::Red, 20.0f, Gdiplus::FontStyleRegular, 0.5f, 0.5f, 0.5f, 0.5f, bgX, startY - spacingY + 50.f, 100.0f));
    
    m_editGroup.push_back(debugBtn);
    m_editGroup.push_back(debugText);
	m_editGroup.push_back(m_debugToggleStatusText);
	m_editGroup.push_back(m_debugToggleStatusBG);

    // 닫기 버튼
    UIButton* closeBtn = AddManaged(objMgr->CreateButton(GOID_UI_BUTTON, btnW/2, btnH/2, L"Resource/UI/frontscreen.png", L"Resource/UI/HighLight_frontscreen.png", 0.5f, 0.5f, 0.5f, 0.5f, bgX, startY - spacingY * 2, [this]() {
        ToggleEditPanel();
    }));
    UIText* closeText = AddManaged(objMgr->CreateText(GOID_UI_TEXT, btnW/2, btnH/2, L"닫기", Gdiplus::Color::Black, 20.0f, Gdiplus::FontStyleRegular, 0.5f, 0.5f, 0.5f, 0.5f, bgX, startY - spacingY * 2, 100.0f));
    m_editGroup.push_back(closeBtn);
    m_editGroup.push_back(closeText);
}

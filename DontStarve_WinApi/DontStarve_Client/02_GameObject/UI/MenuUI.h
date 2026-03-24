#pragma once

#include "UIElement.h"
#include <vector>
#include <map>
#include <memory>
#include <functional>

class UIImage;
class UIButton;
class UIText;
class Player;
class Sprite;
class ObjectManager;

class MenuUI : public UIElement
{
public:
    MenuUI();
    virtual ~MenuUI();

    virtual void Init() override;
    virtual void LateInit() override {}
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override {}
    virtual void Render() override {}
    virtual void Release() override;

    // UI 컨트롤
    void ToggleToolList();
    void ToggleCreateList();
    void ToggleCookList();
    void ToggleBossPanel();
	void ToggleEditPanel();

    void SelectCraftItem(GameObjectID itemID);
    void SelectBoss(GameObjectID bossID);
    void UpdateIngredientDisplay();
    bool TryCraftSelectedItem(Player* player);
    void TryChallengeBoss();

private:
    // Lifecycle & Managed Objects
    template<typename T>
    T* AddManaged(T* pObj) {
        if (pObj) {
            m_managedObjects.push_back(pObj);
        }
        return pObj;
    }

    void ClearAllPanels();

    // UI 생성 헬퍼
    void InitializeCraftingUI();
    void CreateMenuBar();
    void CreatePalettes();
    void CreateIngredientUI();
    void CreateBossChallengeUI();
    void CreateEditUI();

    void UpdateBossPanelHighlight();

private:
    // UI 요소 관리 (전체 관리 및 그룹 관리)
    std::vector<GameObject*> m_managedObjects;
    
    // 개별 접근이 필요한 특정 UI들
    UIImage*  m_toolPanelBg          = nullptr;
    UIImage*  m_ingredientPanelBg    = nullptr;
    UIButton* m_craftButton          = nullptr;
    UIText*   m_craftButtonText      = nullptr;
    UIText*   m_craftingItemNameText = nullptr;

    // 그룹별 관리 (가시성 제어용)
    std::vector<GameObject*> m_toolGroup;
    std::vector<GameObject*> m_createGroup;
    std::vector<GameObject*> m_cookGroup;
    std::vector<GameObject*> m_bossGroup;
	std::vector<GameObject*> m_editGroup;
    std::vector<GameObject*> m_ingredientGroup;

    // 재료 UI 슬롯 (최대 2개 고정)
    struct IngredientSlot {
        UIImage* image;
        UIText*  text;
    } m_ingredientSlots[2];

    // 보스 UI 슬롯
    struct BossPanelSlot {
        UIButton* panel;
        UIText*   clearText;
        GameObjectID id;
    } m_bossPanels[2];

    UIButton* m_bossChallengeButton     = nullptr;
    UIText*   m_bossChallengeButtonText = nullptr;
    UIImage*  m_bossOverlay             = nullptr;
    
	UIImage* m_debugToggleStatusBG = nullptr;
	UIText*   m_debugToggleStatusText   = nullptr;

    // 데이터
    GameObjectID m_selectedCraftItemID = GOID_NONE;
    GameObjectID m_selectedBossID = GOID_NONE;
    
    std::vector<GameObjectID> m_availableTools;
    std::vector<GameObjectID> m_availableCreateItems;
    std::vector<GameObjectID> m_availableCookItems;

    // 상태
    enum class PanelType { NONE, TOOL, CREATE, COOK, BOSS, Editor };
    PanelType m_currentPanel = PanelType::NONE;

    // 레이아웃 상수 (필요한 것만 보존)
    const float m_craftBarWidth         = 125.0f;
    const float m_craftBarHeight        = 400.0f;
    const float m_iconSize              = 64.0f;
    const float m_toolButtonSize        = 64.0f;
    const float m_toolButtonSpacing     = 8.0f;
    const int   m_columnsPerRow         = 3;
    const float m_ingredientImageSize   = 60.0f;
    const float m_ingredientSpacing     = 50.f;
    
    float m_ingredientStartY            = 0.0f;
    float m_ingredientPanelCenterX      = 0.0f;
    float m_menuIconY[5]                = { 0, };
};

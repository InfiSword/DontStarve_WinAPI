#pragma once

#include "UIElement.h"
#include <vector>
#include <map>

class UIImage;
class UIButton;
class UIText;
class Player;

class CraftingUI : public UIElement
{
private:
	// UI 요소들
	UIImage* m_craftBar;                              // 크래프팅 바 배경
	UIButton* m_craftIcon;                            // 크래프팅 아이콘 버튼 (도구 목록 토글)
	UIButton* m_menuCreateIcon;                       // 메뉴바: Create 아이콘
	UIButton* m_menuEditIcon;                         // 메뉴바: Edit 아이콘
	UIButton* m_menuBattleIcon;                       // 메뉴바: Battle 아이콘
	UIButton* m_menuCookIcon;                         // 메뉴바: Cook 아이콘
	std::vector<UIButton*> m_toolButtons;             // 도구 버튼들
	UIButton* m_craftButton;                          // 크래프팅 실행 버튼
	UIText* m_craftButtonText;                        // "제작하기" 버튼 텍스트
	std::vector<UIImage*> m_ingredientImages;         // 재료 이미지들
	std::vector<UIText*> m_ingredientTexts;           // 재료 설명 텍스트들 (최대 2개)
	
	// 상태 관리
	bool m_isToolListVisible;                        // 도구 목록 표시 여부
	bool m_isCreateListVisible;                     // 제작 재료 목록 표시 여부 (CreateIcon)
	bool m_isCookListVisible;                       // 요리 목록 표시 여부 (CookIcon)
	GameObjectID m_selectedToolID;                   // 선택된 도구/재료/요리 ID
	std::vector<GameObjectID> m_availableTools;      // 제작 가능한 도구 목록
	std::vector<GameObjectID> m_availableCreateItems; // 제작 가능한 재료 목록 (석재, 밧줄, 나무판자)
	std::vector<GameObjectID> m_availableCookItems;  // 제작 가능한 요리 목록 (몬스터/작은/일반 고기 요리)
	std::vector<UIButton*> m_createItemButtons;      // Create 패널 재료 버튼들
	std::vector<UIButton*> m_cookItemButtons;        // Cook 패널 요리 버튼들

	// UI 레이아웃 상수들
	float m_craftBarWidth;
	float m_craftBarHeight;
	float m_iconSize;
	float m_toolButtonSize;
	float m_toolButtonSpacing;
	float m_toolPanelOffsetX;
	float m_toolButtonStartY;
	int m_columnsPerRow;
	float m_craftButtonWidth;
	float m_craftButtonHeight;
	float m_ingredientImageSize;
	float m_ingredientTextHeight;
	float m_ingredientSpacing;
	float m_ingredientStartY;
	float m_ingredientToolGap;                        // 도구 패널과 재료 행 간격
	float m_toolPanelBottomY;                         // 도구 버튼 영역 하단 Y
	float m_ingredientPanelCenterX;                   // 재료/도구 패널 가로 중앙 X
	float m_menuIconX;                                // 메뉴바 아이콘 고정 X
	float m_menuIconStartY;                           // 메뉴바 맨 위 아이콘 Y
	float m_menuIconSpacing;                          // 메뉴바 아이콘 세로 간격 (아래로 한 칸마다)
	float m_menuIconY0;                               // 계산됨: 1번째 Y
	float m_menuIconY1;                               // 계산됨: 2번째 Y
	float m_menuIconY2;                               // 계산됨: 3번째 Y
	float m_menuIconY3;                               // 계산됨: 4번째 Y
	float m_menuIconY4;                               // 계산됨: 5번째 Y

public:
	CraftingUI();
	virtual ~CraftingUI();

	virtual void Init() override;
	virtual void LateInit() override {}
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override {}
	virtual void Render() override {}
	virtual void Release() override;

	// 크래프팅 UI 초기화
	void InitializeCraftingUI();

	// 도구 목록 토글
	void ToggleToolList();
	// 제작 재료 목록 토글 (CreateIcon)
	void ToggleCreateList();
	// 요리 목록 토글 (CookIcon)
	void ToggleCookList();

	// 도구 선택
	void SelectTool(GameObjectID toolID);

	// 선택된 도구의 재료 정보 업데이트
	void UpdateIngredientDisplay();

	// 크래프팅 시도
	bool TryCraftSelectedTool(Player* player);

private:
	// UI 요소 생성 헬퍼 함수들
	void CreateCraftBar();
	void CreateMenuBarIcons();
	void CreateToolButtons();
	void CreateCreateItemButtons();
	void CreateCookItemButtons();
	void CreateCraftButton();
	void CreateIngredientDisplay();

	// 재료 이미지 경로 가져오기
	std::wstring GetIngredientImagePath(GameObjectID ingredientID);
	// 재료 표시 이름 가져오기 (수량과 함께 텍스트에 사용)
	std::wstring GetIngredientDisplayName(GameObjectID ingredientID);
};

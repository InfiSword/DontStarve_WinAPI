#pragma once

#include "UIElement.h"
#include <vector>
#include <map>

class UIImage;
class UIButton;
class UIText;
class Player;

class MenuUI : public UIElement
{
private:
	// UI 요소들
	UIImage* m_menuBar;                              // 크래프팅 바 배경
	UIImage* m_toolPanelBg;                           // 도구 팔레트 배경
	UIImage* m_ingredientPanelBg;                     // 재료 표시 배경
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
	UIText* m_craftingItemNameText;                   // 제작하려는 아이템 이름 텍스트
	
	// 보스 전투 UI 요소들
	UIImage* m_bossOverlay;                          // 반투명 검은색 오버레이
	UIButton* m_houndBossPanel;                      // 하운드 보스 패널 (클릭 가능)
	UIButton* m_spiderQueenBossPanel;                // 스파이더 퀸 보스 패널 (클릭 가능)
	UIText* m_houndClearText;                        // "CLEAR" 텍스트 (하운드)
	UIText* m_spiderQueenClearText;                  // "CLEAR" 텍스트 (스파이더 퀸)
	UIButton* m_bossChallengeButton;                 // 보스 도전 버튼
	UIText* m_bossChallengeButtonText;               // 보스 도전 버튼 텍스트
	bool m_isBossPanelVisible;                       // 보스 패널 표시 여부
	GameObjectID m_selectedBossID;                   // 선택된 보스 ID
	
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
	int   m_columnsPerRow;
	float m_craftButtonWidth;
	float m_craftButtonHeight;
	float m_ingredientImageSize;
	float m_ingredientSpacing;
	float m_ingredientStartY;                         // 재료 행 중심 Y (계산됨)
	float m_ingredientPanelCenterX;                   // 재료/도구 패널 가로 중앙 X (계산됨)
	float m_ingredientPanelOffsetX;                   // 재료 패널 전체 X 추가 오프셋
	float m_ingredientPanelOffsetY;                   // 재료 패널 전체 Y 추가 오프셋 (양수=아래)
	float m_menuIconX;                                // 메뉴바 아이콘 고정 X
	float m_menuIconStartY;                           // 메뉴바 맨 위 아이콘 Y
	float m_menuIconSpacing;                          // 메뉴바 아이콘 세로 간격
	float m_menuIconY[5];                             // 계산됨: 메뉴바 아이콘 Y 위치 [0]=맨위 ~ [4]=맨아래
	float m_paletteBgOffsetX;                         // 팔레트 BG X 추가 오프셋 (버튼 그리드와 정렬)
	float m_paletteBgOffsetY;                         // 팔레트 BG Y 추가 오프셋 (버튼 그리드와 정렬)
	float m_toolButtonOffsetX;                        // 도구 버튼 그리드 X 추가 오프셋 (BG와 정렬)
	
public:
	MenuUI();
	virtual ~MenuUI();

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
	// 보스 패널 토글
	void ToggleBossPanel();

	// 도구 선택
	void SelectTool(GameObjectID toolID);
	// 보스 선택
	void SelectBoss(GameObjectID bossID);

	// 선택된 도구의 재료 정보 업데이트
	void UpdateIngredientDisplay();

	// 크래프팅 시도
	bool TryCraftSelectedTool(Player* player);
	// 보스 도전 시도
	void TryChallengeBoss();

private:
	// 모든 패널 UI 닫기 (공통 로직)
	void ClearAllPanels();

	// UI 요소 생성 헬퍼 함수들
	void CreateCraftBar();
	void CreatePanelBackgrounds();                    // 도구/재료 패널 배경 생성
	void CreateMenuBarIcons();
	void CreateToolButtons();
	void CreateCreateItemButtons();
	void CreateCookItemButtons();
	void CreateCraftButton();
	void CreateIngredientDisplay();
	void CreateBossUI();                             // 보스 UI 생성
	
	// 보스 클리어 상태 확인 헬퍼
	bool IsHoundBossCleared() const;
	bool IsSpiderQueenBossCleared() const;
	void UpdateBossPanelHighlight();
};

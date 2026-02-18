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
	std::vector<UIButton*> m_toolButtons;             // 도구 버튼들
	UIButton* m_craftButton;                          // 크래프팅 실행 버튼
	std::vector<UIImage*> m_ingredientImages;         // 재료 이미지들
	std::vector<UIText*> m_ingredientTexts;            // 재료 설명 텍스트들
	
	// 상태 관리
	bool m_isToolListVisible;                        // 도구 목록 표시 여부
	GameObjectID m_selectedToolID;                    // 선택된 도구 ID
	std::vector<GameObjectID> m_availableTools;       // 제작 가능한 도구 목록
	
	// 크래프팅 레시피 맵 (도구ID -> 재료 맵)
	std::map<GameObjectID, std::map<UINT, UINT>> m_craftingRecipes;

	// UI 레이아웃 상수들
	float m_craftBarWidth;
	float m_craftBarHeight;
	float m_iconSize;
	float m_iconOffsetFromTop;
	float m_toolButtonSize;
	float m_toolButtonSpacing;
	float m_toolPanelOffsetX;
	float m_toolButtonStartY;
	int m_columnsPerRow;
	float m_craftButtonWidth;
	float m_craftButtonHeight;
	float m_craftButtonOffsetFromTop;
	float m_ingredientImageSize;
	float m_ingredientTextHeight;
	float m_ingredientSpacing;
	float m_ingredientStartY;

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

	// 도구 선택
	void SelectTool(GameObjectID toolID);

	// 선택된 도구의 재료 정보 업데이트
	void UpdateIngredientDisplay();

	// 크래프팅 시도
	bool TryCraftSelectedTool(Player* player);

	// 크래프팅 레시피 로드
	void LoadCraftingRecipes();

private:
	// UI 요소 생성 헬퍼 함수들
	void CreateCraftBar();
	void CreateCraftIcon();
	void CreateToolButtons();
	void CreateCraftButton();
	void CreateIngredientDisplay();

	// 재료 이미지 경로 가져오기
	std::wstring GetIngredientImagePath(GameObjectID ingredientID);
};

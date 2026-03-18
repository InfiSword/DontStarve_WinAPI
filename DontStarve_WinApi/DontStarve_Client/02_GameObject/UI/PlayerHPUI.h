#pragma once

class Player;
class Sprite;
class UIText;
class UIButton;

// 플레이어 HP UI (우측 상단 아이콘 + 게이지바)
class PlayerHPUI
{
public:
	PlayerHPUI();
	~PlayerHPUI();

	void Init();
	void Update(float deltaTime);
	void Render();
	void Release();

private:
	void UpdateHPDisplay(int currentHp, int maxHp);

	// HP 바 (우측 상단)
	std::shared_ptr<Sprite> m_hpIconSprite;
	UIText* m_hpText;  // "max/hp" 텍스트 (게이지 바 안, UIManager 등록)

	// HP 텍스트 갱신 시 할당 방지: 마지막으로 표시한 값과 같으면 SetText 호출 안 함
	int m_lastDisplayedHp;
	int m_lastDisplayedMaxHp;

	// 레이아웃 상수
	static const float MARGIN;
	static const float BAR_WIDTH;
	static const float BAR_HEIGHT;
	static const float ICON_SIZE;
	static const float GAP;
	static const float GAME_OVER_SORT_KEY;
};

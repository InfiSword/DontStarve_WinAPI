#pragma once
#include "UIElement.h"

class Entity;
class Sprite;
class UIText;

// 공통 HP UI (플레이어 및 보스용)
class HPUI : public UIElement
{
public:
	HPUI(Entity* target, 
		 const std::wstring& name,
		 float barWidth, float barHeight,
		 Gdiplus::Color bgColor, Gdiplus::Color fillColor, Gdiplus::Color nameColor,
		 float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY,
		 float anchoredPosX, float anchoredPosY,
		 float barSortKey = 10.1f, float textSortKey = 10.2f,
		 bool showIcon = true, bool numericValue = true,
		 float margin = 20.0f, float gap = 10.0f, float iconSize = 48.0f);
	
	virtual ~HPUI();

	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void Render() override;
	virtual void Release() override;

private:
	void UpdateHPDisplay();

	Entity* m_target;
	std::wstring m_name;
	bool m_showIcon;
	bool m_numericValue; 

	std::shared_ptr<Sprite> m_hpIconSprite;
	UIText* m_nameText;  

	int m_lastHp;
	int m_lastMaxHp;

	float m_barWidth;
	float m_barHeight;
	float m_margin;
	float m_gap;
	float m_iconSize;

	Gdiplus::Color m_bgColor;
	Gdiplus::Color m_fillColor;
	Gdiplus::Color m_nameColor;

	float m_barSortKey;
	float m_textSortKey;
};

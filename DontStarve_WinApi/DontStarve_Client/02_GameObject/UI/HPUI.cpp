#include "99_Default/pch.h"
#include "HPUI.h"
#include "../Entity/Entity.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Sprite.h"
#include "UIText.h"

HPUI::HPUI(Entity* target, const std::wstring& name,
	float barWidth, float barHeight,
	Gdiplus::Color bgColor, Gdiplus::Color fillColor, Gdiplus::Color nameColor,
	float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY,
	float anchoredPosX, float anchoredPosY,
	float barSortKey, float textSortKey,
	bool showIcon, bool numericValue,
	float margin, float gap, float iconSize)
	: UIElement(GOID_NONE, L"", L"", true, false)
	, m_target(target), m_name(name)
	, m_barWidth(barWidth), m_barHeight(barHeight)
	, m_bgColor(bgColor), m_fillColor(fillColor), m_nameColor(nameColor)
	, m_barSortKey(barSortKey), m_textSortKey(textSortKey)
	, m_showIcon(showIcon), m_numericValue(numericValue)
	, m_margin(margin), m_gap(gap), m_iconSize(iconSize)
	, m_nameText(nullptr)
	, m_lastHp(-1), m_lastMaxHp(-1)
{
	if (m_rectTransform) {
		m_rectTransform->SetAnchorMin(anchorMinX, anchorMinY);
		m_rectTransform->SetAnchorMax(anchorMaxX, anchorMaxY);
		m_rectTransform->SetAnchoredPosition(anchoredPosX, anchoredPosY);
		m_rectTransform->SetPivot(0.5f, 0.5f);
	}
}

HPUI::~HPUI()
{
	Release();
}

void HPUI::Init()
{
	UIElement::Init();

	if (m_showIcon) {
		m_hpIconSprite = ResourceManager::GetInstance()->LoadSprite(L"Resource\\UI\\HP.png");
	}

	float textPosX = m_rectTransform->GetAnchoredPosition().X;
	float textPosY = m_rectTransform->GetAnchoredPosition().Y;

	// 보스용인 경우 텍스트를 바 위로 올림
	if (!m_numericValue) {
		textPosY -= (m_barHeight * 0.5f + 25.0f);
	}

	m_nameText = new UIText(
		GOID_NONE,
		m_barWidth,
		m_barHeight,
		m_name,
		m_nameColor,
		LAYER_UI_FOREGROUND,
		m_textSortKey,
		L"Arial", 16.0f,
		Gdiplus::StringAlignmentCenter, Gdiplus::StringAlignmentCenter,
		m_rectTransform->GetAnchorMin().X, m_rectTransform->GetAnchorMin().Y,
		m_rectTransform->GetAnchorMax().X, m_rectTransform->GetAnchorMax().Y,
		textPosX, textPosY
	);

	m_nameText->Init();
	ObjectManager::GetInstance()->AddGameObject(m_nameText);

	UpdateHPDisplay();
}

void HPUI::Update(float deltaTime)
{
	UIElement::Update(deltaTime);

	if (m_nameText && m_nameText->IsEnabled() != IsEnabled())
		m_nameText->SetActive(IsEnabled());

	if (m_target && m_target->IsEnabled()) {
		UpdateHPDisplay();
	}
	else if (m_target && !m_target->IsEnabled()) {
		SetActive(false);
	}
}

void HPUI::UpdateHPDisplay()
{
	if (!m_target || !m_nameText) return;

	int currentHp = m_target->GetHp();
	int maxHp = m_target->GetMaxHp();

	if (currentHp == m_lastHp && maxHp == m_lastMaxHp)
		return;

	m_lastHp = currentHp;
	m_lastMaxHp = maxHp;

	if (m_numericValue) {
		std::wstring str = std::to_wstring(currentHp) + L"/" + std::to_wstring(maxHp);
		m_nameText->SetText(str);
	}
	else {
		m_nameText->SetText(m_name);
	}
}

void HPUI::Render()
{
	if (!IsEnabled()) return;

	RenderManager* pRM = RenderManager::GetInstance();
	if (!pRM) return;

	float barX = m_rectTransform->GetX();
	float barY = m_rectTransform->GetY();
	float barW = m_barWidth;
	float barH = m_barHeight;

	float barLeft = barX - barW * m_rectTransform->GetPivotX();
	float barTop = barY - barH * m_rectTransform->GetPivotY();

	if (m_showIcon && m_hpIconSprite && m_hpIconSprite->bitmap) {
		float iconX = barLeft - m_gap - m_iconSize;
		float iconY = barTop + (barH - m_iconSize) * 0.5f;
		
		Gdiplus::Bitmap* bmp = m_hpIconSprite->bitmap.get();
		pRM->AddDrawCommand(
			bmp,
			Gdiplus::RectF(iconX, iconY, m_iconSize, m_iconSize),
			Gdiplus::RectF(0, 0, (float)bmp->GetWidth(), (float)bmp->GetHeight()),
			Gdiplus::UnitPixel,
			Gdiplus::PointF(iconX + m_iconSize * 0.5f, iconY + m_iconSize * 0.5f),
			LAYER_UI_FOREGROUND,
			10.0f,
			DIR_DOWN
		);
	}

	pRM->AddFillRectangleCommand(Gdiplus::RectF(barLeft, barTop, barW, barH), m_bgColor, LAYER_UI_FOREGROUND, m_barSortKey);

	if (m_target) {
		int hp = m_target->GetHp();
		int maxHp = m_target->GetMaxHp();
		float ratio = (maxHp > 0) ? (static_cast<float>(hp) / static_cast<float>(maxHp)) : 0.0f;
		if (ratio < 0.0f) ratio = 0.0f;
		if (ratio > 1.0f) ratio = 1.0f;

		float fillWidth = barW * ratio;
		if (fillWidth > 0.01f) {
			pRM->AddFillRectangleCommand(Gdiplus::RectF(barLeft, barTop, fillWidth, barH), m_fillColor, LAYER_UI_FOREGROUND, m_barSortKey + 0.1f);
		}
	}
}

void HPUI::Release()
{
	if (m_nameText) {
		ObjectManager::GetInstance()->RemoveGameObject(m_nameText);
		m_nameText = nullptr;
	}
	UIElement::Release();
}

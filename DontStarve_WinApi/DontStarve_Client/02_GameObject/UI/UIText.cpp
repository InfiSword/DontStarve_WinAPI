#include "99_Default/pch.h"
#include "UIText.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Text/Text.h"
#include "../../01_Manager/RenderManager/RenderManager.h"

UIText::UIText(GameObjectID id, float width, float height,
               const std::wstring& text, const Gdiplus::Color& color,
               RenderLayer layer, float sortKey,
               const std::wstring& fontName, float fontSize,
               Gdiplus::StringAlignment hAlign, Gdiplus::StringAlignment vAlign,
               float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY,
               float anchoredPosX, float anchoredPosY)
	: UIElement(id, L"", L"", true, false),
	m_text(nullptr)
{
	// UIElement에서 이미 RectTransform이 생성되었으므로 GetRectTransform() 사용
	RectTransform* rectTransform = GetRectTransform();
	rectTransform->SetAnchorMin(anchorMinX, anchorMinY);
	rectTransform->SetAnchorMax(anchorMaxX, anchorMaxY);
	rectTransform->SetAnchoredPosition(anchoredPosX, anchoredPosY);
	rectTransform->SetPivot(0.5f, 0.5f);
	
	// Text 컴포넌트 추가
	m_text = AddComponent<Text>(
		text,
		color,
		width,
		height,
		layer,
		sortKey,
		fontName,
		fontSize,
		hAlign,
		vAlign
	);
}

UIText::~UIText()
{
	Release();
}

void UIText::Update(float deltaTime)
{
	// GameObject::Update() 호출하여 모든 컴포넌트 업데이트
	GameObject::Update(deltaTime);
}

void UIText::Render()
{
	if (!IsEnabled() || !m_text || !m_rectTransform) return;
	
	RenderManager::GetInstance()->RenderText(m_rectTransform, m_text);
}

void UIText::SetText(const std::wstring& text)
{
	if (m_text) {
		m_text->SetText(text);
	}
}

void UIText::SetColor(const Gdiplus::Color& color)
{
	if (m_text) {
		m_text->SetColor(color);
	}
}

void UIText::Release()
{
	// UIText 전용 정리 작업
	m_text = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	UIElement::Release();
}

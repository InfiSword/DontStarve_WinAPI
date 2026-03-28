#include "99_Default/pch.h"
#include "UIButton.h"
#include "../../01_Manager/InputManager/InputManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../Component/Transform/RectTransform.h"
#include "../Component/Sprite/Image.h"
#include "../Component/Sprite/Sprite.h"
#include "../Component/Button/Button.h"

UIButton::UIButton(GameObjectID id, float width, float height,
	const std::shared_ptr<Sprite>& normalSprite, const std::shared_ptr<Sprite>& hoverSprite,
	float anchorMinX, float anchorMinY, float anchorMaxX, float anchorMaxY,
	float anchoredPosX, float anchoredPosY)
	: UIElement(id, L"", L"", true, false),
	m_image(nullptr),
	m_buttonComp(nullptr),
	m_normalSprite(nullptr),
	m_hoverSprite(nullptr)
{
	m_image = AddComponent<ComponentElement::Image>();
	m_image->SetLayer(LAYER_UI_FOREGROUND);
	m_image->SetSortKey(0.0f);
	m_image->SetSprite(normalSprite);
	m_normalSprite = normalSprite;
	m_hoverSprite = hoverSprite;
	
	RectTransform* rectTransform = GetRectTransform();
	rectTransform->SetAnchorMin(anchorMinX, anchorMinY);
	rectTransform->SetAnchorMax(anchorMaxX, anchorMaxY);
	rectTransform->SetAnchoredPosition(anchoredPosX, anchoredPosY);
	m_image->SetPivot(0.5f, 0.5f);

	Gdiplus::Bitmap* bitmap = m_image->GetSprite();
	if (bitmap) {
		float bw = static_cast<float>(bitmap->GetWidth());
		float bh = static_cast<float>(bitmap->GetHeight());
		m_rectTransform->SetSize(bw, bh);
		m_rectTransform->SetScale(width / bw, height / bh);		
	}
	else
	{
		rectTransform->SetSize(width, height);
		rectTransform->SetScale(1.0f, 1.0f);
	}

	ButtonStateStyle normalStyle{ Gdiplus::Color(255, 255, 255, 255), LAYER_UI_FOREGROUND, 0.0f };
	ButtonStateStyle hoverStyle{ Gdiplus::Color(200, 200, 200, 255), LAYER_UI_FOREGROUND, 0.0f };
	ButtonStateStyle clickedStyle{ Gdiplus::Color(200, 200, 200, 255), LAYER_UI_FOREGROUND, 0.0f };
	ButtonStateStyle disabledStyle{ Gdiplus::Color(160, 160, 160, 255), LAYER_UI_FOREGROUND, 0.0f };
	m_buttonComp = AddComponent<Button>(normalStyle, hoverStyle, clickedStyle, disabledStyle);
}

UIButton::~UIButton()
{
	Release();
}

void UIButton::Update(float deltaTime)
{
	if (!m_buttonComp || !m_image || !m_rectTransform) return;

	m_image->Update(deltaTime);
	Button::State previousState = m_buttonComp->GetState();
	bool callbackInvoked = m_buttonComp->UpdateState(m_rectTransform, m_image);
	if (callbackInvoked) return;
	if (!m_buttonComp || !m_image) return;

	Button::State currentState = m_buttonComp->GetState();
	if (previousState != currentState) {
		if (currentState == Button::State::HOVER && m_hoverSprite)
			m_image->SetSprite(m_hoverSprite);
		else if (m_normalSprite)
			m_image->SetSprite(m_normalSprite);
	}
}

void UIButton::Render()
{
	if (!IsEnabled() || !m_image) return;
	
	m_image->Render();
}

void UIButton::RenderDisabled()
{
	if (!m_image) return;
	
	m_image->Render();
}

Gdiplus::Bitmap* UIButton::GetBitmap() const
{
	return m_image->GetSprite();
}

void UIButton::SetOnClickCallback(std::function<void()> callback)
{
	m_buttonComp->SetOnClickCallback(std::move(callback));
}

void UIButton::SetDisabled(bool disabled)
{
	m_buttonComp->SetDisabled(disabled);
}

void UIButton::SetHoverColor(const Gdiplus::Color& color)
{
	m_buttonComp->SetHoverColor(color);
}

void UIButton::SetNormalColor(const Gdiplus::Color& color)
{
	m_buttonComp->SetNormalColor(color);
}

void UIButton::SetClickedColor(const Gdiplus::Color& color)
{
	m_buttonComp->SetClickedColor(color);
}

void UIButton::SetDisabledColor(const Gdiplus::Color& color)
{
	m_buttonComp->SetDisabledColor(color);
}

void UIButton::UpdateHoverStateImmediate()
{
	if (!m_buttonComp || !m_image || !m_rectTransform) return;
	Button::State previousState = m_buttonComp->GetState();
	bool callbackInvoked = m_buttonComp->UpdateState(m_rectTransform, m_image);
	if (callbackInvoked || !m_buttonComp || !m_image) return;
	Button::State currentState = m_buttonComp->GetState();
	if (previousState != currentState) {
		if (currentState == Button::State::HOVER && m_hoverSprite)
			m_image->SetSprite(m_hoverSprite);
		else if (m_normalSprite)
			m_image->SetSprite(m_normalSprite);
	}
}

void UIButton::SetSortKey(float sortKey)
{
	if (m_image)
		m_image->SetSortKey(sortKey);
}

void UIButton::Release()
{
	m_image = nullptr;
	m_buttonComp = nullptr;
	m_normalSprite = nullptr;
	m_hoverSprite = nullptr;
	UIElement::Release();
}

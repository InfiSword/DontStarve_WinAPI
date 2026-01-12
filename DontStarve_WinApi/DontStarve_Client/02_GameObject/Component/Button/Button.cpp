#include "99_Default/pch.h"
#include "Button.h"
#include "../Transform/RectTransform.h"
#include "../Sprite/Image.h"
#include "../Text/Text.h"
#include "../../GameObject.h"
#include "../../../01_Manager/InputManager/InputManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"

Button::Button(GameObject* owner,
	const ButtonVisualState& normalState,
	const ButtonVisualState& hoverState,
	const ButtonVisualState& disabledState)
	: Component(owner),
	m_buttonState(ButtonState::NORMAL),
	m_isMouseOver(false),
	m_isDisabled(false),
	m_normal(normalState),
	m_hover(hoverState),
	m_disabled(disabledState)
{
}

Button::~Button()
{
	Release();
}

void Button::Init() {}

void Button::Release()
{
	m_onClickCallback = nullptr;
}

void Button::SetOnClickCallback(std::function<void()> callback)
{
	m_onClickCallback = std::move(callback);
}

void Button::SetDisabled(bool disabled)
{
	m_isDisabled = disabled;
	m_buttonState = disabled ? ButtonState::DISABLED : ButtonState::NORMAL;
}

bool Button::IsPointInside(const RectTransform* rectTransform, ComponentElement::Image* image, float x, float y) const
{
	if (!rectTransform || !image || !image->GetSprite()) return false;

	Gdiplus::Bitmap* bitmap = image->GetSprite();
	float bitmapWidth = static_cast<float>(bitmap->GetWidth());
	float bitmapHeight = static_cast<float>(bitmap->GetHeight());
	float width = bitmapWidth * rectTransform->GetScaleX();
	float height = bitmapHeight * rectTransform->GetScaleY();

	float rectX = rectTransform->GetX();
	float rectY = rectTransform->GetY();
	float pivotX = rectTransform->GetPivotX();
	float pivotY = rectTransform->GetPivotY();

	float left = rectX - (width * pivotX);
	float top = rectY - (height * pivotY);
	float right = left + width;
	float bottom = top + height;

	return (x >= left && x <= right && y >= top && y <= bottom);
}

const ButtonVisualState& Button::GetVisualState(ButtonState state) const
{
	switch (state) {
	case ButtonState::HOVER:
	case ButtonState::CLICKED:
		return m_hover;
	case ButtonState::DISABLED:
		return m_disabled;
	default:
		return m_normal;
	}
}

void Button::UpdateState(const RectTransform* rectTransform, ComponentElement::Image* image)
{
	if (!rectTransform || !image) return;

	POINT mousePos = InputManager::GetInstance()->GetMousePos();
	float mouseX = static_cast<float>(mousePos.x);
	float mouseY = static_cast<float>(mousePos.y);

	bool inside = IsPointInside(rectTransform, image, mouseX, mouseY);
	m_isMouseOver = inside && !m_isDisabled;

	if (m_isDisabled) {
		m_buttonState = ButtonState::DISABLED;
	}
	else if (inside && InputManager::GetInstance()->IsLButtonClicked()) {
		m_buttonState = ButtonState::CLICKED;
		// 렌더가 상위(UI)에서 수행되므로 콜백만 트리거
		if (m_onClickCallback) {
			m_onClickCallback();
		}
	}
	else if (inside) {
		m_buttonState = ButtonState::HOVER;
	}
	else {
		m_buttonState = ButtonState::NORMAL;
	}

}

ButtonRenderParams Button::GetRenderParams(const RectTransform* rectTransform, ComponentElement::Image* image) const
{
	ButtonRenderParams params{};
	if (!rectTransform || !image) return params;

	const ButtonVisualState& state = GetVisualState(m_buttonState);

	params.overrideSpritePath = state.spritePath;
	Gdiplus::Bitmap* currentBitmap = image->GetSprite();
	if (!currentBitmap) return params;

	float bitmapWidth = static_cast<float>(currentBitmap->GetWidth());
	float bitmapHeight = static_cast<float>(currentBitmap->GetHeight());
	params.targetWidth = (state.width > 0.0f) ? state.width : bitmapWidth * rectTransform->GetScaleX();
	params.targetHeight = (state.height > 0.0f) ? state.height : bitmapHeight * rectTransform->GetScaleY();

	params.bitmap = currentBitmap;
	params.pivotX = rectTransform->GetPivotX();
	params.pivotY = rectTransform->GetPivotY();
	params.layer = state.layer;
	params.sortKey = state.sortKeyOffset + static_cast<float>(m_buttonState);
	params.textColor = state.textColor;
	params.textSortKey = state.sortKeyOffset + 0.1f + static_cast<float>(m_buttonState);

	return params;
}
#include "99_Default/pch.h"
#include "Button.h"
#include "../Transform/RectTransform.h"
#include "../Sprite/Image.h"
#include "../../GameObject.h"
#include "../../../01_Manager/InputManager/InputManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"

Button::Button(GameObject* owner,
	const ButtonStateStyle& normalStyle,
	const ButtonStateStyle& hoverStyle,
	const ButtonStateStyle& clickedStyle,
	const ButtonStateStyle& disabledStyle)
	: Component(owner),
	m_buttonState(ButtonState::NORMAL),
	m_isMouseOver(false),
	m_isDisabled(false),
	m_normal(normalStyle),
	m_click(clickedStyle),
	m_hover(hoverStyle),
	m_disabled(disabledStyle)
{
}

Button::~Button()
{
	Release();
}

void Button::Init() 
{
	// 초기 상태 적용
	ComponentElement::Image* image = GetOwner()->GetComponent<ComponentElement::Image>();
	if (image) {
		ApplyVisualState(image);
	}
}

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

const ButtonStateStyle& Button::GetStateStyle(ButtonState state) const
{
	switch (state) {
	case ButtonState::HOVER:
		return m_hover;
	case ButtonState::CLICKED:
		return m_click;
	case ButtonState::DISABLED:
		return m_disabled;
	default:
		return m_normal;
	}
}

void Button::UpdateState(const RectTransform* rectTransform, ComponentElement::Image* image)
{
	if (!rectTransform || !image) return;

	ButtonState previousState = m_buttonState;

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
		if (m_onClickCallback) {
			m_onClickCallback();
		}
	}
	else if (m_buttonState == ButtonState::CLICKED && !InputManager::GetInstance()->IsLButtonDown()) {
		// 클릭 상태에서 마우스 버튼이 떼어지면 hover 또는 normal로 전환
		m_buttonState = inside ? ButtonState::HOVER : ButtonState::NORMAL;
	}
	else if (inside) {
		m_buttonState = ButtonState::HOVER;
	}
	else {
		m_buttonState = ButtonState::NORMAL;
	}

	if (previousState != m_buttonState) {
		ApplyVisualState(image);
	}
}

void Button::ApplyVisualState(ComponentElement::Image* image)
{
	if (!image) return;

	const ButtonStateStyle& style = GetStateStyle(m_buttonState);

	// Image 컴포넌트 스타일 적용
	ComponentElement::ImageStyle imgStyle{
		style.layer,
		style.sortKeyOffset + static_cast<float>(m_buttonState)
	};
	image->ApplyStyle(imgStyle);

	// Sprite 색상 적용은 UIButton에서 렌더링 시점에 처리하도록 변경
	// (여러 버튼이 같은 스프라이트를 공유할 수 있으므로 직접 변경하지 않음)
	// std::shared_ptr<Sprite> sprite = image->GetSpriteHandle();
	// if (sprite) {
	//     sprite->tintColor = style.spriteColor;
	// }
}
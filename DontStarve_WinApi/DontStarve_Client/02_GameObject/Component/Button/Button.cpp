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
	m_buttonState(State::NORMAL),
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
	m_buttonState = disabled ? State::DISABLED : State::NORMAL;
}

bool Button::IsPointInside(const RectTransform* rectTransform, ComponentElement::Image* image, float x, float y) const
{
	if (!rectTransform) return false;

	float width, height;
	
	// 스프라이트가 있으면 스프라이트 크기 사용, 없으면 RectTransform의 width/height 사용
	if (image && image->GetSprite()) 
	{
		Gdiplus::Bitmap* bitmap = image->GetSprite();
		float bitmapWidth = static_cast<float>(bitmap->GetWidth());
		float bitmapHeight = static_cast<float>(bitmap->GetHeight());
		width = bitmapWidth * rectTransform->GetScaleX();
		height = bitmapHeight * rectTransform->GetScaleY();
	}
	else {
		// 스프라이트가 없는 경우 RectTransform의 width/height와 scale 사용
		width = rectTransform->GetWidth() * rectTransform->GetScaleX();
		height = rectTransform->GetHeight() * rectTransform->GetScaleY();
	}

	float rectX = rectTransform->GetX();
	float rectY = rectTransform->GetY();
	float pivotX = image ? image->GetPivotX() : 0.5f;
	float pivotY = image ? image->GetPivotY() : 0.5f;

	float left = rectX - (width * pivotX);
	float top = rectY - (height * pivotY);
	float right = left + width;
	float bottom = top + height;

	return (x >= left && x <= right && y >= top && y <= bottom);
}

const ButtonStateStyle& Button::GetStateStyle(State state) const
{
	switch (state) {
	case State::HOVER:
		return m_hover;
	case State::CLICKED:
		return m_click;
	case State::DISABLED:
		return m_disabled;
	default:
		return m_normal;
	}
}

bool Button::UpdateState(const RectTransform* rectTransform, ComponentElement::Image* image)
{
	if (!rectTransform || !image) return false;

	State previousState = m_buttonState;

	// 마우스 위치 가져오기
	InputManager* inputManager = InputManager::GetInstance();
	if (!inputManager) return false;
	
	POINT mousePos = inputManager->GetMousePos();
	float mouseX = static_cast<float>(mousePos.x);
	float mouseY = static_cast<float>(mousePos.y);
	bool inside = IsPointInside(rectTransform, image, mouseX, mouseY);
	m_isMouseOver = inside && !m_isDisabled;

	// 비활성화 상태 처리
	if (m_isDisabled) {
		m_buttonState = State::DISABLED;
		if (previousState != m_buttonState) {
			ApplyVisualState(image);
		}
		return false;
	}

	// 클릭 처리 (가장 우선순위)
	if (inside && inputManager->IsLButtonClicked()) {
		m_buttonState = State::CLICKED;
		if (previousState != m_buttonState) {
			ApplyVisualState(image);
		}
		// 콜백 호출
		if (m_onClickCallback) {
			m_onClickCallback();
			return true; 
		}
		return false;
	}

	// 클릭 상태에서 버튼을 떼었을 때 처리
	if (m_buttonState == State::CLICKED && !inputManager->IsLButtonDown()) {
		m_buttonState = inside ? State::HOVER : State::NORMAL;
		if (previousState != m_buttonState) {
			ApplyVisualState(image);
		}
		return false;
	}

	// hover/normal 상태 처리
	if (m_buttonState != State::CLICKED && m_buttonState != State::DISABLED) {
		m_buttonState = inside ? State::HOVER : State::NORMAL;
		if (previousState != m_buttonState) {
			ApplyVisualState(image);
		}
	}

	return false;
}


void Button::ApplyVisualState(ComponentElement::Image* image)
{
	if (!image) return;

	const ButtonStateStyle& style = GetStateStyle(m_buttonState);

	ComponentElement::ImageStyle imgStyle{
		style.layer,
		style.sortKeyOffset + static_cast<float>(m_buttonState)
	};
	image->ApplyStyle(imgStyle);

	image->SetTintColor(style.spriteColor);
}

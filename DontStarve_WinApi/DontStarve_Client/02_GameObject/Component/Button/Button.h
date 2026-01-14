#pragma once

#include "../Component.h"

class RectTransform;
namespace ComponentElement { class Image; }

// 버튼 상태별 시각적 설정
struct ButtonStateStyle {
	Gdiplus::Color spriteColor;  // 스프라이트 틴트 색상 (기본값: 흰색)
	RenderLayer layer;
	float sortKeyOffset;
};

class Button : public Component
{
public:
	Button(GameObject* owner,
		const ButtonStateStyle& normalStyle,
		const ButtonStateStyle& hoverStyle,
		const ButtonStateStyle& clickedStyle,
		const ButtonStateStyle& disabledStyle);
	virtual ~Button();

	virtual void Init() override;
	virtual void Release() override;

	void SetOnClickCallback(std::function<void()> callback);
	void SetDisabled(bool disabled);
	ButtonState GetState() const { return m_buttonState; }
	bool IsDisabled() const { return m_isDisabled; }

	const ButtonStateStyle& GetStateStyle(ButtonState state) const;
	void UpdateState(const RectTransform* rectTransform, ComponentElement::Image* image);
	void ApplyVisualState(ComponentElement::Image* image);

private:
	ButtonState m_buttonState;
	bool m_isMouseOver;
	bool m_isDisabled;
	std::function<void()> m_onClickCallback;

	ButtonStateStyle m_normal;
	ButtonStateStyle m_click;
	ButtonStateStyle m_hover;
	ButtonStateStyle m_disabled;

	bool IsPointInside(const RectTransform* rectTransform, ComponentElement::Image* image, float x, float y) const;
};

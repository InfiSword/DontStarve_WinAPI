#pragma once

#include "../Component.h"

class RectTransform;
namespace ComponentElement { class Image; }

// 버튼 상태별 시각적 설정
struct ButtonStateStyle {
	Gdiplus::Color spriteColor; 
	RenderLayer layer;
	float sortKeyOffset;
};

class Button : public Component
{
public:
	enum class State
	{
		NORMAL,
		HOVER,
		CLICKED,
		DISABLED
	};

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
	State GetState() const { return m_buttonState; }
	bool IsDisabled() const { return m_isDisabled; }

	const ButtonStateStyle& GetStateStyle(State state) const;

	bool UpdateState(const RectTransform* rectTransform, ComponentElement::Image* image);
	void ApplyVisualState(ComponentElement::Image* image);
	
	// 색상만 변경하는 편의 메서드
	void SetNormalColor(const Gdiplus::Color& color) { m_normal.spriteColor = color; }
	void SetHoverColor(const Gdiplus::Color& color) { m_hover.spriteColor = color; }
	void SetClickedColor(const Gdiplus::Color& color) { m_click.spriteColor = color; }
	void SetDisabledColor(const Gdiplus::Color& color) { m_disabled.spriteColor = color; }

private:
	State m_buttonState;
	bool m_isMouseOver;
	bool m_isDisabled;
	std::function<void()> m_onClickCallback;

	ButtonStateStyle m_normal;
	ButtonStateStyle m_click;
	ButtonStateStyle m_hover;
	ButtonStateStyle m_disabled;

	bool IsPointInside(const RectTransform* rectTransform, ComponentElement::Image* image, float x, float y) const;
};

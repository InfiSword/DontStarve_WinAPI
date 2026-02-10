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
	/// true 반환 시 클릭 콜백이 호출됐으므로 호출부에서는 이 버튼/UI를 더 이상 참조하면 안 됨 (씬 전환 등으로 파괴되었을 수 있음)
	bool UpdateState(const RectTransform* rectTransform, ComponentElement::Image* image);
	void ApplyVisualState(ComponentElement::Image* image);
	
	// 스타일 설정 메서드
	//void SetNormalStyle(const ButtonStateStyle& style) { m_normal = style; }
	//void SetHoverStyle(const ButtonStateStyle& style) { m_hover = style; }
	//void SetClickedStyle(const ButtonStateStyle& style) { m_click = style; }
	//void SetDisabledStyle(const ButtonStateStyle& style) { m_disabled = style; }
	
	// 색상만 변경하는 편의 메서드
	void SetNormalColor(const Gdiplus::Color& color) { m_normal.spriteColor = color; }
	void SetHoverColor(const Gdiplus::Color& color) { m_hover.spriteColor = color; }
	void SetClickedColor(const Gdiplus::Color& color) { m_click.spriteColor = color; }
	void SetDisabledColor(const Gdiplus::Color& color) { m_disabled.spriteColor = color; }

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

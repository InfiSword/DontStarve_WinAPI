#pragma once

#include "../Component.h"

class RectTransform;
namespace ComponentElement { class Image; }

struct ButtonVisualState {
	Gdiplus::Color textColor;
	RenderLayer layer;
	float sortKeyOffset;
	std::wstring spritePath; 
	float width;             // 0이면 기존 스케일 기반
	float height;            // 0이면 기존 스케일 기반
};

struct ButtonRenderParams {
	Gdiplus::Bitmap* bitmap = nullptr;
	std::wstring overrideSpritePath;
	float targetWidth = 0.0f;
	float targetHeight = 0.0f;
	float pivotX = 0.5f;
	float pivotY = 0.5f;
	RenderLayer layer = LAYER_UI_FOREGROUND;
	float sortKey = 0.0f;
	Gdiplus::Color textColor = Gdiplus::Color::Black;
	float textSortKey = 0.0f;
};

class Button : public Component
{
public:
	Button(GameObject* owner,
		const ButtonVisualState& normalState,
		const ButtonVisualState& hoverState,
		const ButtonVisualState& disabledState);
	virtual ~Button();

	virtual void Init() override;
	virtual void Release() override;

	void SetOnClickCallback(std::function<void()> callback);
	void SetDisabled(bool disabled);
	ButtonState GetState() const { return m_buttonState; }
	bool IsDisabled() const { return m_isDisabled; }

	const ButtonVisualState& GetVisualState(ButtonState state) const;
	void UpdateState(const RectTransform* rectTransform, ComponentElement::Image* image);
	ButtonRenderParams GetRenderParams(const RectTransform* rectTransform, ComponentElement::Image* image) const;

private:
	ButtonState m_buttonState;
	bool m_isMouseOver;
	bool m_isDisabled;
	std::function<void()> m_onClickCallback;

	ButtonVisualState m_normal;
	ButtonVisualState m_hover;
	ButtonVisualState m_disabled;

	bool IsPointInside(const RectTransform* rectTransform, ComponentElement::Image* image, float x, float y) const;
};

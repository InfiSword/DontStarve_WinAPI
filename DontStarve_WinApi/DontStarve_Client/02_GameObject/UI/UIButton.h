#pragma once
#include "UIElement.h"

class RectTransform;
class Button;
class Sprite;

namespace ComponentElement { class Image; }

class UIButton : public UIElement
{
private:
	ComponentElement::Image* m_image;
	Button* m_buttonComp;
	std::shared_ptr<Sprite> m_normalSprite;  // normal 상태 스프라이트 저장
	std::shared_ptr<Sprite> m_hoverSprite;   // hover 상태 스프라이트 저장

	// 이전 상태 추적 (불필요한 스타일 재적용 방지)
	ButtonState m_previousState;

public:
	UIButton(GameObjectID id, float width, float height,
		const std::shared_ptr<Sprite>& normalSprite, const std::shared_ptr<Sprite>& hoverSprite,
		float anchorMinX = 0.5f, float anchorMinY = 0.5f,
		float anchorMaxX = 0.5f, float anchorMaxY = 0.5f,
		float anchoredPosX = 0.0f, float anchoredPosY = 0.0f);
	virtual ~UIButton();

	virtual void Init() override {}
	virtual void LateInit() override {}
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override {}
	virtual void Render() override;
	virtual void Release() override;

	Gdiplus::Bitmap* GetBitmap() const;
	ButtonState GetButtonState() const;
	const ComponentElement::Image* GetImageComponent() const { return m_image; }

	// 버튼 이벤트
	void SetOnClickCallback(std::function<void()> callback);

	// 비활성화 관련 메서드
	void SetDisabled(bool disabled);
	void RenderDisabled();

	// 버튼 스타일 설정 메서드
	void SetHoverColor(const Gdiplus::Color& color);
	void SetNormalColor(const Gdiplus::Color& color);
	void SetClickedColor(const Gdiplus::Color& color);
	void SetDisabledColor(const Gdiplus::Color& color);

	// 마우스 이동 시 즉시 hover 상태 업데이트 (반응 속도 개선)
	void UpdateHoverStateImmediate();
};
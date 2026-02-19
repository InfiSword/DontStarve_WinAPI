#pragma once

#include "UIElement.h"

class Sprite;
class RectTransform;
namespace ComponentElement { class Image; }

class UIImage : public UIElement
{

public:
    UIImage(GameObjectID id, float width, float height, RenderLayer layer, const std::wstring& imagePath, float sortKey,
            float anchorMinX = 0.5f, float anchorMinY = 0.5f,
            float anchorMaxX = 0.5f, float anchorMaxY = 0.5f,
            float anchoredPosX = 0.0f, float anchoredPosY = 0.0f);
    virtual ~UIImage();

    virtual void Init() override {}
    virtual void LateInit() override {}
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override {}
    virtual void Render() override;
    virtual void Release() override;

    Gdiplus::Bitmap* GetBitmap() const;
	ComponentElement::Image* GetImageComponent() { return m_image; }
	const ComponentElement::Image* GetImageComponent() const { return m_image; }
	void SetSprite(const std::shared_ptr<Sprite>& sprite);
	void LoadSprite(const std::wstring& imagePath);
	
	// 색상 설정 메서드
	void SetTintColor(const Gdiplus::Color& color);
	void SetTintColor(BYTE r, BYTE g, BYTE b, BYTE a = 255);
	void SetAlpha(BYTE alpha);
	Gdiplus::Color GetTintColor() const;
	
private:
    RectTransform* m_rectTransform;
    ComponentElement::Image* m_image;
};

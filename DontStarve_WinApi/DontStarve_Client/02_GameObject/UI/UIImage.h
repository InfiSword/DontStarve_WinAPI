#pragma once

#include "../GameObject.h"

class Sprite;
class RectTransform;
namespace ComponentElement { class Image; }

class UIImage : public GameObject
{

public:
    UIImage(GameObjectID id, float x, float y, float width, float height, RenderLayer layer, const std::wstring& imagePath, float sortKey);
    virtual ~UIImage();

    virtual void Init() override {}
    virtual void LateInit() override {}
    virtual void Update(float deltaTime) override;
    virtual void LateUpdate() override {}
    virtual void Render();
    virtual void Release() override;

    Gdiplus::Bitmap* GetBitmap() const;
	const ComponentElement::Image* GetImageComponent() const { return m_image; }
	void SetSprite(const std::shared_ptr<Sprite>& sprite);
private:
    RectTransform* m_rectTransform;
    ComponentElement::Image* m_image;
}; 

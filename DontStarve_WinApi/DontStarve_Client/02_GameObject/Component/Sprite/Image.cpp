#include "99_Default/pch.h"
#include "Image.h"
#include "Sprite.h"
#include "../../GameObject.h"
#include "../Transform/RectTransform.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"

namespace ComponentElement {

Image::Image(GameObject* owner, RenderLayer layer, float sortKey)
	: Component(owner), m_sprite(nullptr), m_layer(layer), m_sortKey(sortKey),
	m_tintColor(255, 255, 255, 255)  
{
}

Image::~Image()
{
	Release();
}

void Image::Init()
{
}

void Image::Release()
{
	m_sprite.reset();
}

void Image::Render()
{
	if (!m_sprite || !m_sprite->bitmap) return;

	RectTransform* rt = GetOwner()->GetComponent<RectTransform>();
	if (!rt) return;

	Gdiplus::RectF destRect = rt->GetScreenBoundingBox();

	RenderManager::GetInstance()->AddDrawCommand(
		m_sprite->bitmap.get(),
		destRect,
		m_sprite->sourceRect,
		Gdiplus::UnitPixel,
		Gdiplus::PointF(rt->GetX(), rt->GetY()),
		m_layer,
		0.0f, // yPos (UI는 보통 0)
		m_sortKey,
		DIR_NONE,
		m_tintColor,
		(m_tintColor.GetValue() != Gdiplus::Color::MakeARGB(255, 255, 255, 255)),
		false,
		rt->GetRotation()
	);
}

void Image::LoadSprite(const std::wstring& fullPath)
{
	if (fullPath.empty()) {
		m_sprite = nullptr;
		return;
	}

	m_sprite = ResourceManager::GetInstance()->LoadSprite(fullPath);
}

void Image::SetDisplaySize(float width, float height) const
{
	if (!m_sprite) return;
	RectTransform* rectTransform = GetOwner() ? GetOwner()->GetComponent<RectTransform>() : nullptr;
	if (!rectTransform) return;
	Gdiplus::RectF srcRect = m_sprite->sourceRect;
	if (srcRect.Width > 0 && srcRect.Height > 0) {
		rectTransform->SetScale(width / srcRect.Width, height / srcRect.Height);
	}
}

void Image::SetDisplaySizeProportional(float maxSize) const
{
	if (!m_sprite || maxSize <= 0.0f) return;
	RectTransform* rectTransform =GetOwner()->GetComponent<RectTransform>();	
	if (!rectTransform) return;

	Gdiplus::RectF srcRect = m_sprite->sourceRect;
	
	if (srcRect.Width <= 0 || srcRect.Height <= 0) return;
	float scale = maxSize / (srcRect.Width >= srcRect.Height ? srcRect.Width : srcRect.Height);
	rectTransform->SetScale(scale, scale);
}

}

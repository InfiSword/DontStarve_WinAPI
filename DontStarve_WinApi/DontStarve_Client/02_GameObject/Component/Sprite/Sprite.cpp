#include "99_Default/pch.h"
#include "Sprite.h"

Sprite::Sprite(std::shared_ptr<Gdiplus::Bitmap> bmp,
	const Gdiplus::RectF& srcRect,
	float px,
	float py,
	const std::wstring& k,
	const Gdiplus::Color& tint)
	: bitmap(std::move(bmp)), sourceRect(srcRect), pivotX(px), pivotY(py), key(k), tintColor(tint)
{

}

Sprite::~Sprite()
{
	std::wstring().swap(key);
}

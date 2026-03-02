#include "99_Default/pch.h"
#include "Sprite.h"

Sprite::Sprite(std::shared_ptr<Gdiplus::Bitmap> bmp,
	const Gdiplus::RectF& srcRect,
	const std::wstring& k,
	const Gdiplus::Color& tint)
	: bitmap(std::move(bmp)), sourceRect(srcRect), key(k), tintColor(tint)
{

}

Sprite::~Sprite()
{
	std::wstring().swap(key);
}

std::unique_ptr<Sprite> Sprite::CreateFromFile(const std::wstring& path)
{
	Gdiplus::Bitmap* pBitmap = Gdiplus::Bitmap::FromFile(path.c_str());
	if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) {
		if (pBitmap) delete pBitmap;
		return nullptr;
	}

	std::shared_ptr<Gdiplus::Bitmap> sharedBitmap(pBitmap);
	Gdiplus::RectF srcRect(0.0f, 0.0f, (float)pBitmap->GetWidth(), (float)pBitmap->GetHeight());

	return std::make_unique<Sprite>(sharedBitmap, srcRect, path);
}

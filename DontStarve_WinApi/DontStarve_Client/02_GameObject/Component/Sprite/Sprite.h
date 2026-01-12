#pragma once

// 스프라이트 데이터
class Sprite
{
public:
	std::shared_ptr<Gdiplus::Bitmap> bitmap;
	Gdiplus::RectF sourceRect;
	float pivotX;
	float pivotY;
	std::wstring key;      
	bool isAtlas;

	Sprite(std::shared_ptr<Gdiplus::Bitmap> bmp,
		const Gdiplus::RectF& srcRect,
		float px,
		float py,
		const std::wstring& k,
		bool atlas)
		: bitmap(std::move(bmp)), sourceRect(srcRect), pivotX(px), pivotY(py), key(k), isAtlas(atlas) {}
};

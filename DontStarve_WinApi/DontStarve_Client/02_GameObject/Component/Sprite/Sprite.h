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
	Gdiplus::Color tintColor;  // 색상 틴트 (기본값: 흰색, 덮어쓰기 방식 - Unity Sprite 스타일)

	Sprite(std::shared_ptr<Gdiplus::Bitmap> bmp,
		const Gdiplus::RectF& srcRect,
		float px,
		float py,
		const std::wstring& k,
		bool atlas,
		const Gdiplus::Color& tint = Gdiplus::Color(255, 255, 255, 255))
		: bitmap(std::move(bmp)), sourceRect(srcRect), pivotX(px), pivotY(py), key(k), isAtlas(atlas), tintColor(tint) {}
};

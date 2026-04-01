#pragma once

class Sprite
{
public:
	std::shared_ptr<Gdiplus::Bitmap> bitmap;
	Gdiplus::RectF sourceRect;
	Gdiplus::PointF pivot;     
	std::wstring key;      
	Gdiplus::Color tintColor; 

	Sprite(std::shared_ptr<Gdiplus::Bitmap> bmp,
		const Gdiplus::RectF& srcRect,
		const Gdiplus::PointF& pvt,
		const std::wstring& k = L"",
		const Gdiplus::Color& tint = Gdiplus::Color(255, 255, 255, 255));
	
	~Sprite();

	// 파일로부터 Sprite를 생성하는 팩토리 메서드
	static std::shared_ptr<Sprite> CreateFromFile(const std::wstring& path, const Gdiplus::PointF& pvt);
};

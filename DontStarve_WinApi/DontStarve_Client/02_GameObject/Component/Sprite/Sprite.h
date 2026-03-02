#pragma once

// 스프라이트 데이터
class Sprite
{
public:
	std::shared_ptr<Gdiplus::Bitmap> bitmap;
	Gdiplus::RectF sourceRect;
	std::wstring key;      
	Gdiplus::Color tintColor;  // 색상 틴트 (기본값: 흰색, 덮어쓰기 방식 - Unity Sprite 스타일)

	Sprite(std::shared_ptr<Gdiplus::Bitmap> bmp,
		const Gdiplus::RectF& srcRect,
		const std::wstring& k,
		const Gdiplus::Color& tint = Gdiplus::Color(255, 255, 255, 255));
	
	~Sprite();

	// 파일로부터 Sprite를 생성하는 팩토리 메서드
	static std::unique_ptr<Sprite> CreateFromFile(const std::wstring& path);
};

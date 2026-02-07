#include "99_Default/pch.h"
#include "Image.h"
#include "Sprite.h"
#include "../../GameObject.h"

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

void Image::LoadSprite(const std::wstring& fullPath)
{
	if (fullPath.empty()) {
		OutputDebugStringW(L"Image: LoadSprite 실패 - 경로가 비어있음\n");
		m_sprite = nullptr;
		return;
	}

	OutputDebugStringW((L"Image: LoadSprite - 전체 경로: " + fullPath + L"\n").c_str());

	// 비트맵 로드
	auto bmp = std::make_shared<Gdiplus::Bitmap>(fullPath.c_str());
	if (bmp && bmp->GetLastStatus() == Gdiplus::Ok) {
		Gdiplus::RectF src(0, 0, static_cast<float>(bmp->GetWidth()), static_cast<float>(bmp->GetHeight()));
		m_sprite = std::make_shared<Sprite>(bmp, src, 0.5f, 0.5f, fullPath, false);
		OutputDebugStringW(L"Image: LoadSprite 성공\n");
	}
	else {
		OutputDebugStringW(L"Image: LoadSprite 실패 - 비트맵 파일 로드 실패\n");
		m_sprite.reset();
	}
}

}

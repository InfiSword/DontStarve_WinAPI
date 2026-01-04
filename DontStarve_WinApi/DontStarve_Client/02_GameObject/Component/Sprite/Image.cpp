#include "../../../99_Default/pch.h"
#include "Image.h"

::Image::Image(GameObject* owner, RenderLayer layer, float sortKey)
	: Component(owner), m_sprite(nullptr), m_layer(layer), m_sortKey(sortKey)
{
}

::Image::~Image()
{
	Release();
}

void ::Image::Init()
{
}

void ::Image::Release()
{
	SafeDelete(m_sprite);
}

void ::Image::LoadSprite(const std::wstring& fullPath)
{
	if (fullPath.empty()) {
		OutputDebugStringW(L"Image: LoadSprite 실패 - 경로가 비어있음\n");
		m_sprite = nullptr;
		return;
	}

	OutputDebugStringW((L"Image: LoadSprite - 전체 경로: " + fullPath + L"\n").c_str());

	// 비트맵 로드
	m_sprite = new Gdiplus::Bitmap(fullPath.c_str());
	if (m_sprite && m_sprite->GetLastStatus() != Gdiplus::Ok) {
		OutputDebugStringW(L"Image: LoadSprite 실패 - 비트맵 파일 로드 실패\n");
		delete m_sprite;
		m_sprite = nullptr;
	}
	else if (m_sprite) {
		OutputDebugStringW(L"Image: LoadSprite 성공\n");
	}
	else {
		OutputDebugStringW(L"Image: LoadSprite 실패 - 비트맵 생성 실패\n");
	}
}


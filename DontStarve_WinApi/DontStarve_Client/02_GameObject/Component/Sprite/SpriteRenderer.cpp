#include "../../../99_Default/pch.h"
#include "SpriteRenderer.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../03_Animation/Animator.h"
#include "../Transform/Transform.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Header/Struct.h"

SpriteRenderer::SpriteRenderer(GameObject* owner, RenderLayer layer)
	: Component(owner), m_sprite(nullptr), m_layer(layer), m_sortKey(0.0f),
	m_animator(nullptr), m_transform(nullptr)
{
}

SpriteRenderer::~SpriteRenderer()
{
	Release();
}

void SpriteRenderer::Init()
{
	// 컴포넌트 참조 캐싱
	m_animator = GetOwner()->GetComponent<Animator>();
	m_transform = GetOwner()->GetComponent<Transform>();
}

void SpriteRenderer::Release()
{
	SafeDelete(m_sprite);
}

void SpriteRenderer::LoadSprite(GameObjectID id, const std::wstring& imageName)
{
	if (imageName.empty()) {
		OutputDebugStringW((L"SpriteRenderer: LoadSprite 실패 - 이미지명이 비어있음 (ID: " + std::to_wstring(id) + L")\n").c_str());
		m_sprite = nullptr;
		return;
	}

	std::wstring fullPath = ResourceManager::GetInstance()->BuildObjectResourcePath(id, L"", imageName);
	LoadSprite(fullPath);
}

void SpriteRenderer::LoadSprite(const std::wstring& fullPath)
{
	if (fullPath.empty()) {
		OutputDebugStringW(L"SpriteRenderer: LoadSprite 실패 - 경로가 비어있음\n");
		m_sprite = nullptr;
		return;
	}

	OutputDebugStringW((L"SpriteRenderer: LoadSprite - 전체 경로: " + fullPath + L"\n").c_str());

	// 비트맵 로드
	m_sprite = new Gdiplus::Bitmap(fullPath.c_str());
	if (m_sprite && m_sprite->GetLastStatus() != Gdiplus::Ok) {
		OutputDebugStringW(L"SpriteRenderer: LoadSprite 실패 - 비트맵 파일 로드 실패\n");
		delete m_sprite;
		m_sprite = nullptr;
	}
	else if (m_sprite) {
		OutputDebugStringW(L"SpriteRenderer: LoadSprite 성공\n");
	}
	else {
		OutputDebugStringW(L"SpriteRenderer: LoadSprite 실패 - 비트맵 생성 실패\n");
	}
}


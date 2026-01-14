#include "../../../99_Default/pch.h"
#include "SpriteRenderer.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"

SpriteRenderer::SpriteRenderer(GameObject* owner, RenderLayer layer)
	: Component(owner)
	, m_sprite(nullptr)
	, m_layer(layer)
	, m_sortKey(0.0f)
	, m_sourceRect(0.0f, 0.0f, 0.0f, 0.0f)
	, m_pivotX(0.5f)
	, m_pivotY(0.5f)
{
}

SpriteRenderer::~SpriteRenderer()
{
	Release();
}

void SpriteRenderer::Init()
{
	// 기본값 유지 (비트맵 로드/Animator 연동 시 갱신)
}

void SpriteRenderer::Release()
{
	m_sprite = nullptr;
	m_sourceRect = Gdiplus::RectF(0.0f, 0.0f, 0.0f, 0.0f);
	m_pivotX = 0.5f;
	m_pivotY = 0.5f;
}

void SpriteRenderer::SetSprite(Gdiplus::Bitmap* sprite, const Gdiplus::RectF& sourceRect, float pivotX, float pivotY)
{
	// Animator 등에서 전달된 비트맵/프레임 정보를 반영 (소유권 없음)
	m_sprite = sprite;
	m_sourceRect = sourceRect;
	m_pivotX = pivotX;
	m_pivotY = pivotY;
}


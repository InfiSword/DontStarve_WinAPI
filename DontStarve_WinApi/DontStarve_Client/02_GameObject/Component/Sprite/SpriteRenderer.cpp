#include "99_Default/pch.h"
#include "SpriteRenderer.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../02_GameObject/GameObject.h"
#include "../../../02_GameObject/Component/Transform/Transform.h"

SpriteRenderer::SpriteRenderer(GameObject* owner, RenderLayer layer)
	: Component(owner), m_sprite(nullptr), m_layer(layer), m_preFlipped(false)
{
}

SpriteRenderer::~SpriteRenderer()
{
	Release();
}

void SpriteRenderer::Init()
{

}

void SpriteRenderer::Render()
{
	auto pTransform = m_owner->GetComponent<Transform>();
	if (!pTransform || !m_sprite || !m_sprite->bitmap) return;

	CameraManager* pCam = CameraManager::GetInstance();
	Gdiplus::PointF screenPos = pCam->WorldToScreen(pTransform->GetX(), pTransform->GetY());

	RenderLayer layer = m_layer;
	float yPos = pTransform->GetY();
	Direction dir = pTransform->GetDirection();

	// 스케일 적용하여 렌더링 크기 계산
	float width = m_sprite->sourceRect.Width * pTransform->GetScaleX();
	float height = m_sprite->sourceRect.Height * pTransform->GetScaleY();
	float x = screenPos.X - width * m_sprite->pivot.X;
	float y = screenPos.Y - height * m_sprite->pivot.Y;

	// Y-Sorting: 항상 스프라이트의 최하단(발밑) 좌표를 기준으로 정렬
	float sortingY = yPos + (1.0f - m_sprite->pivot.Y) * height;


	Gdiplus::Color tintColor = m_sprite->tintColor;
	bool hasTint = (tintColor.GetValue() != Gdiplus::Color::MakeARGB(255, 255, 255, 255));

	RenderManager::GetInstance()->AddDrawCommand(m_sprite->bitmap.get(), Gdiplus::RectF(x, y, width, height),
		m_sprite->sourceRect, Gdiplus::UnitPixel, screenPos,
		layer, sortingY, 0, dir, tintColor, hasTint, m_preFlipped);
}

void SpriteRenderer::Release()
{
	m_sprite.reset();
}

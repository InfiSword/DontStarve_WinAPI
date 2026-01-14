#include "99_Default/pch.h"
#include "RectTransform.h"
#include "../Sprite/Image.h"
#include "../Sprite/SpriteRenderer.h"
#include "../../GameObject.h"

RectTransform::RectTransform(GameObject* owner, float x, float y,
	float scaleX, float scaleY, float anchorX, float anchorY,
	float pivotX, float pivotY)
	: Component(owner), m_x(x), m_y(y), m_scaleX(scaleX), m_scaleY(scaleY),
	m_anchorX(anchorX), m_anchorY(anchorY), m_pivotX(pivotX), m_pivotY(pivotY),
	m_anchorMin(anchorX, anchorY), m_anchorMax(anchorX, anchorY),
	m_anchoredPosition(x, y), m_sizeDelta(0.0f, 0.0f),
	m_parentWidth(static_cast<float>(WINCX)), m_parentHeight(static_cast<float>(WINCY))
{
}

RectTransform::~RectTransform()
{
}

void RectTransform::SetAnchorMin(float x, float y)
{
	m_anchorMin.X = x;
	m_anchorMin.Y = y;
	m_anchorX = x; // 호환성 유지
	UpdatePositionFromAnchors();
}

void RectTransform::SetAnchorMax(float x, float y)
{
	m_anchorMax.X = x;
	m_anchorMax.Y = y;
	m_anchorY = y; // 호환성 유지
	UpdatePositionFromAnchors();
}

void RectTransform::SetAnchoredPosition(float x, float y)
{
	m_anchoredPosition.X = x;
	m_anchoredPosition.Y = y;
	UpdatePositionFromAnchors();
}

void RectTransform::SetSizeDelta(float width, float height)
{
	m_sizeDelta.Width = width;
	m_sizeDelta.Height = height;
	UpdatePositionFromAnchors();
}

Gdiplus::PointF RectTransform::GetOffsetMin() const
{
	// 왼쪽 하단 오프셋 계산
	float anchorX = m_anchorMin.X;
	float anchorY = m_anchorMin.Y;
	
	float anchorWorldX = anchorX * m_parentWidth;
	float anchorWorldY = anchorY * m_parentHeight;
	
	float left = m_x - anchorWorldX;
	float bottom = m_y - anchorWorldY;
	
	return Gdiplus::PointF(left, bottom);
}

Gdiplus::PointF RectTransform::GetOffsetMax() const
{
	// 오른쪽 상단 오프셋 계산
	float anchorX = m_anchorMax.X;
	float anchorY = m_anchorMax.Y;
	
	float anchorWorldX = anchorX * m_parentWidth;
	float anchorWorldY = anchorY * m_parentHeight;
	
	// 실제 크기 계산
	float width = 0.0f;
	float height = 0.0f;
	
	if (m_owner) {
		Gdiplus::Bitmap* bitmap = nullptr;
		ComponentElement::Image* image = m_owner->GetComponent<ComponentElement::Image>();
		if (image && image->GetSprite()) {
			bitmap = image->GetSprite();
		}
		else {
			SpriteRenderer* spriteRenderer = m_owner->GetComponent<SpriteRenderer>();
			if (spriteRenderer && spriteRenderer->GetSprite()) {
				bitmap = spriteRenderer->GetSprite();
			}
		}
		
		if (bitmap) {
			width = static_cast<float>(bitmap->GetWidth()) * m_scaleX;
			height = static_cast<float>(bitmap->GetHeight()) * m_scaleY;
		}
	}
	
	if (width == 0.0f || height == 0.0f) {
		width = 32.0f * m_scaleX;
		height = 32.0f * m_scaleY;
	}
	
	float right = m_x + width * (1.0f - m_pivotX) - anchorWorldX;
	float top = m_y + height * (1.0f - m_pivotY) - anchorWorldY;
	
	return Gdiplus::PointF(right, top);
}

void RectTransform::SetOffsetMin(float left, float bottom)
{
	float anchorX = m_anchorMin.X;
	float anchorY = m_anchorMin.Y;
	
	float anchorWorldX = anchorX * m_parentWidth;
	float anchorWorldY = anchorY * m_parentHeight;
	
	m_x = anchorWorldX + left;
	m_y = anchorWorldY + bottom;
	
	m_anchoredPosition.X = m_x;
	m_anchoredPosition.Y = m_y;
}

void RectTransform::SetOffsetMax(float right, float top)
{
	float anchorX = m_anchorMax.X;
	float anchorY = m_anchorMax.Y;
	
	float anchorWorldX = anchorX * m_parentWidth;
	float anchorWorldY = anchorY * m_parentHeight;
	
	// 실제 크기 계산
	float width = 0.0f;
	float height = 0.0f;
	
	if (m_owner) {
		Gdiplus::Bitmap* bitmap = nullptr;
		ComponentElement::Image* image = m_owner->GetComponent<ComponentElement::Image>();
		if (image && image->GetSprite()) {
			bitmap = image->GetSprite();
		}
		else {
			SpriteRenderer* spriteRenderer = m_owner->GetComponent<SpriteRenderer>();
			if (spriteRenderer && spriteRenderer->GetSprite()) {
				bitmap = spriteRenderer->GetSprite();
			}
		}
		
		if (bitmap) {
			width = static_cast<float>(bitmap->GetWidth()) * m_scaleX;
			height = static_cast<float>(bitmap->GetHeight()) * m_scaleY;
		}
	}
	
	if (width == 0.0f || height == 0.0f) {
		width = 32.0f * m_scaleX;
		height = 32.0f * m_scaleY;
	}
	
	m_x = anchorWorldX + right - width * (1.0f - m_pivotX);
	m_y = anchorWorldY + top - height * (1.0f - m_pivotY);
	
	m_anchoredPosition.X = m_x;
	m_anchoredPosition.Y = m_y;
}

void RectTransform::UpdatePositionFromAnchors()
{
	// 앵커 기반 위치 계산
	float anchorMinX = m_anchorMin.X * m_parentWidth;
	float anchorMinY = m_anchorMin.Y * m_parentHeight;
	float anchorMaxX = m_anchorMax.X * m_parentWidth;
	float anchorMaxY = m_anchorMax.Y * m_parentHeight;
	
	// 앵커 중심 계산
	float anchorCenterX = (anchorMinX + anchorMaxX) * 0.5f;
	float anchorCenterY = (anchorMinY + anchorMaxY) * 0.5f;
	
	// anchoredPosition을 기준으로 최종 위치 계산
	m_x = anchorCenterX + m_anchoredPosition.X;
	m_y = anchorCenterY + m_anchoredPosition.Y;
	
	// 기존 anchorX, anchorY 업데이트 (호환성 유지)
	m_anchorX = m_anchorMin.X;
	m_anchorY = m_anchorMin.Y;
}

Gdiplus::RectF RectTransform::GetScreenBoundingBox() const
{
	// Sprite 크기 * scale 계산
	float width = 0.0f;
	float height = 0.0f;
	
	// Image 또는 SpriteRenderer 컴포넌트에서 비트맵 크기 가져오기
	if (m_owner) {
		Gdiplus::Bitmap* bitmap = nullptr;
		
		// Image 컴포넌트 우선 확인 (UI)
		ComponentElement::Image* image = m_owner->GetComponent<ComponentElement::Image>();
		if (image && image->GetSprite()) {
			bitmap = image->GetSprite();
		}
		// SpriteRenderer 컴포넌트 확인 (월드 오브젝트)
		else {
			SpriteRenderer* spriteRenderer = m_owner->GetComponent<SpriteRenderer>();
			if (spriteRenderer && spriteRenderer->GetSprite()) {
				bitmap = spriteRenderer->GetSprite();
			}
		}
		
		if (bitmap) {
			width = static_cast<float>(bitmap->GetWidth()) * m_scaleX;
			height = static_cast<float>(bitmap->GetHeight()) * m_scaleY;
		}
	}
	
	// 비트맵이 없으면 기본값 사용
	if (width == 0.0f || height == 0.0f) {
		width = 32.0f * m_scaleX;
		height = 32.0f * m_scaleY;
	}
	
	return Gdiplus::RectF(
		m_x - width * m_pivotX,
		m_y - height * m_pivotY,
		width,
		height
	);
}

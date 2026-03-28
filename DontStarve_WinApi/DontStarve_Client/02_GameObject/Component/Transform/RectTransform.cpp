#include "99_Default/pch.h"
#include "RectTransform.h"
#include "../Sprite/Image.h"
#include "../Sprite/SpriteRenderer.h"
#include "../../GameObject.h"

RectTransform::RectTransform(GameObject* owner, float x, float y,
	float width, float height,
	float scaleX, float scaleY, float anchorX, float anchorY,
	float rotation)
	: Component(owner), m_x(x), m_y(y), 
	m_width(width), m_height(height),
	m_scaleX(scaleX), m_scaleY(scaleY),
	m_rotation(rotation),
	m_anchorX(anchorX), m_anchorY(anchorY),
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
	
	// 실제 크기 계산 (width * scaleX, height * scaleY)
	float actualWidth = m_width * m_scaleX;
	float actualHeight = m_height * m_scaleY;
	
	float pivotX = 0.5f;
	float pivotY = 0.5f;

	if (m_owner) {
		ComponentElement::Image* image = m_owner->GetComponent<ComponentElement::Image>();
		if (image && image->GetSpriteHandle()) {
			pivotX = image->GetPivotX();
			pivotY = image->GetPivotY();
		}
	}

	float right = m_x + actualWidth * (1.0f - pivotX) - anchorWorldX;
	float top = m_y + actualHeight * (1.0f - pivotY) - anchorWorldY;
	
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
	
	// 실제 크기 계산 (width * scaleX, height * scaleY)
	float actualWidth = m_width * m_scaleX;
	float actualHeight = m_height * m_scaleY;
	
	float pivotX = 0.5f;
	float pivotY = 0.5f;

	if (m_owner) {
		ComponentElement::Image* image = m_owner->GetComponent<ComponentElement::Image>();
		if (image && image->GetSpriteHandle()) {
			pivotX = image->GetPivotX();
			pivotY = image->GetPivotY();
		}
	}

	m_x = anchorWorldX + right - actualWidth * (1.0f - pivotX);
	m_y = anchorWorldY + top - actualHeight * (1.0f - pivotY);
	
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
	// 기본 크기는 m_width, m_height 사용
	float actualWidth = m_width * m_scaleX;
	float actualHeight = m_height * m_scaleY;
	
	float pivotX = 0.5f;
	float pivotY = 0.5f;

	// Sprite가 있으면 그 크기를 우선 사용 (기존 동작 유지)
	if (m_owner) {
		Gdiplus::Bitmap* bitmap = nullptr;
		
		// Image 컴포넌트 우선 확인 (UI)
		ComponentElement::Image* image = m_owner->GetComponent<ComponentElement::Image>();
		if (image && image->GetSpriteHandle()) {
			bitmap = image->GetSpriteHandle()->bitmap.get();
			pivotX = image->GetPivotX();
			pivotY = image->GetPivotY();
		}
		// SpriteRenderer 컴포넌트 확인 (월드 오브젝트)
		else {
			SpriteRenderer* spriteRenderer = m_owner->GetComponent<SpriteRenderer>();
			if (spriteRenderer && spriteRenderer->GetSpriteHandle()) {
				bitmap = spriteRenderer->GetSpriteHandle()->bitmap.get();
				pivotX = spriteRenderer->GetPivotX();
				pivotY = spriteRenderer->GetPivotY();
			}
		}
		
		if (bitmap) {
			actualWidth = static_cast<float>(bitmap->GetWidth()) * m_scaleX;
			actualHeight = static_cast<float>(bitmap->GetHeight()) * m_scaleY;
		}
	}
	
	return Gdiplus::RectF(
		m_x - actualWidth * pivotX,
		m_y - actualHeight * pivotY,
		actualWidth,
		actualHeight
	);
}

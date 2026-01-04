#include "../../../99_Default/pch.h"
#include "RectTransform.h"
#include "../Sprite/Image.h"
#include "../../GameObject.h"

RectTransform::RectTransform(GameObject* owner, float x, float y,
	float scaleX, float scaleY, float anchorX, float anchorY,
	float pivotX, float pivotY)
	: Component(owner), m_x(x), m_y(y), m_scaleX(scaleX), m_scaleY(scaleY),
	m_anchorX(anchorX), m_anchorY(anchorY), m_pivotX(pivotX), m_pivotY(pivotY)
{
}

RectTransform::~RectTransform()
{
}

Gdiplus::RectF RectTransform::GetScreenBoundingBox() const
{
	// Sprite 크기 * scale 계산
	float width = 0.0f;
	float height = 0.0f;
	
	// Image 컴포넌트에서 비트맵 크기 가져오기
	if (m_owner) {
		::Image* image = m_owner->GetComponent<::Image>();
		if (image && image->GetSprite()) {
			Gdiplus::Bitmap* bitmap = image->GetSprite();
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

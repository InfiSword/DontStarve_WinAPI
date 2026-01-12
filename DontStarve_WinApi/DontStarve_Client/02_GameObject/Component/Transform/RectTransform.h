#pragma once

#include "../Component.h"

// UI 오브젝트의 화면 좌표 기반 위치를 관리하는 컴포넌트
class RectTransform : public Component
{
protected:
	float m_x, m_y;				// 화면 좌표
	float m_scaleX, m_scaleY;	// 스케일 (기본값 1.0f)
	float m_anchorX, m_anchorY;	// 앵커 (0.0 ~ 1.0, 기본 0.0)
	float m_pivotX, m_pivotY;	// 피벗 (0.0 ~ 1.0, 기본 0.5)

public:
	RectTransform(GameObject* owner, float x = 0.0f, float y = 0.0f,
		float scaleX = 1.0f, float scaleY = 1.0f,
		float anchorX = 0.0f, float anchorY = 0.0f,
		float pivotX = 0.5f, float pivotY = 0.5f);
	virtual ~RectTransform();

	// 위치 Getter/Setter
	float GetX() const { return m_x; }
	float GetY() const { return m_y; }
	void SetX(float x) { m_x = x; }
	void SetY(float y) { m_y = y; }
	void SetPosition(float x, float y) { m_x = x; m_y = y; }

	// 스케일 Getter/Setter
	float GetScaleX() const { return m_scaleX; }
	float GetScaleY() const { return m_scaleY; }
	void SetScaleX(float scaleX) { m_scaleX = scaleX; }
	void SetScaleY(float scaleY) { m_scaleY = scaleY; }
	void SetScale(float scaleX, float scaleY) { m_scaleX = scaleX; m_scaleY = scaleY; }
	void SetScale(float scale) { m_scaleX = scale; m_scaleY = scale; }

	// 앵커 Getter/Setter
	float GetAnchorX() const { return m_anchorX; }
	float GetAnchorY() const { return m_anchorY; }
	void SetAnchorX(float anchorX) { m_anchorX = anchorX; }
	void SetAnchorY(float anchorY) { m_anchorY = anchorY; }
	void SetAnchor(float anchorX, float anchorY) { m_anchorX = anchorX; m_anchorY = anchorY; }

	// 피벗 Getter/Setter
	float GetPivotX() const { return m_pivotX; }
	float GetPivotY() const { return m_pivotY; }
	void SetPivotX(float pivotX) { m_pivotX = pivotX; }
	void SetPivotY(float pivotY) { m_pivotY = pivotY; }
	void SetPivot(float pivotX, float pivotY) { m_pivotX = pivotX; m_pivotY = pivotY; }

	// 바운딩 박스 계산 (화면 좌표 기준)
	// 주의: sprite 크기 * scale을 사용하므로, Image 또는 SpriteRenderer 컴포넌트가 필요합니다
	Gdiplus::RectF GetScreenBoundingBox() const;
};

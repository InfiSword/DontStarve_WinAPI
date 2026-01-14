#pragma once

#include "../Component.h"

// UI 오브젝트의 화면 좌표 기반 위치를 관리하는 컴포넌트
class RectTransform : public Component
{
protected:
	float m_x, m_y;				// 화면 좌표
	float m_scaleX, m_scaleY;	// 스케일 (기본값 1.0f)
	float m_anchorX, m_anchorY;	// 앵커 (0.0 ~ 1.0, 기본 0.0) - 호환성 유지용
	float m_pivotX, m_pivotY;	// 피벗 (0.0 ~ 1.0, 기본 0.5)
	
	// Unity 스타일 앵커 시스템
	Gdiplus::PointF m_anchorMin;		// 앵커 최소값 (0.0 ~ 1.0, 기본 0.0, 0.0)
	Gdiplus::PointF m_anchorMax;		// 앵커 최대값 (0.0 ~ 1.0, 기본 0.0, 0.0)
	Gdiplus::PointF m_anchoredPosition;	// 앵커 기준 위치
	Gdiplus::SizeF m_sizeDelta;			// 앵커에 따른 크기 오프셋
	
	// 부모 또는 화면 크기 (앵커 계산용)
	float m_parentWidth;	// 부모 너비 (기본값: 화면 너비)
	float m_parentHeight;	// 부모 높이 (기본값: 화면 높이)

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

	// Unity 스타일 앵커 Getter/Setter
	Gdiplus::PointF GetAnchorMin() const { return m_anchorMin; }
	Gdiplus::PointF GetAnchorMax() const { return m_anchorMax; }
	void SetAnchorMin(float x, float y);
	void SetAnchorMax(float x, float y);
	void SetAnchorMin(const Gdiplus::PointF& anchorMin) { SetAnchorMin(anchorMin.X, anchorMin.Y); }
	void SetAnchorMax(const Gdiplus::PointF& anchorMax) { SetAnchorMax(anchorMax.X, anchorMax.Y); }

	// Unity 스타일 위치/크기 Getter/Setter
	Gdiplus::PointF GetAnchoredPosition() const { return m_anchoredPosition; }
	void SetAnchoredPosition(float x, float y);
	void SetAnchoredPosition(const Gdiplus::PointF& pos) { SetAnchoredPosition(pos.X, pos.Y); }

	Gdiplus::SizeF GetSizeDelta() const { return m_sizeDelta; }
	void SetSizeDelta(float width, float height);
	void SetSizeDelta(const Gdiplus::SizeF& size) { SetSizeDelta(size.Width, size.Height); }

	// 오프셋 계산
	Gdiplus::PointF GetOffsetMin() const;
	Gdiplus::PointF GetOffsetMax() const;
	void SetOffsetMin(float left, float bottom);
	void SetOffsetMax(float right, float top);

	// 부모 크기 설정 (앵커 계산용)
	void SetParentSize(float width, float height) { m_parentWidth = width; m_parentHeight = height; UpdatePositionFromAnchors(); }
	float GetParentWidth() const { return m_parentWidth; }
	float GetParentHeight() const { return m_parentHeight; }

	// 앵커 기반 위치 업데이트
	void UpdatePositionFromAnchors();

	// 바운딩 박스 계산 (화면 좌표 기준)
	// 주의: sprite 크기 * scale을 사용하므로, Image 또는 SpriteRenderer 컴포넌트가 필요합니다
	Gdiplus::RectF GetScreenBoundingBox() const;
};

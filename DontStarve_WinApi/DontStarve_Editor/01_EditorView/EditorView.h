#pragma once

#include <windows.h>
#include <gdiplus.h>

class EditorView
{
public:
	EditorView();
	~EditorView() = default;

	// Map offset (camera position)
	const POINT& GetMapOffset() const { return m_mapOffset; }
	void SetMapOffset(int x, int y);
	void SetMapOffset(POINT p) { SetMapOffset(p.x, p.y); }

	// Zoom
	float GetZoomFactor() const { return m_zoomFactor; }
	void SetZoomFactor(float zoom);
	float GetMinZoom() const { return m_minZoom; }
	float GetMaxZoom() const { return m_maxZoom; }
	float GetZoomStep() const { return m_zoomStep; }
	void ZoomIn();   // Increase zoom (call from WM_MOUSEWHEEL WHEEL_DELTA positive)
	void ZoomOut();  // Decrease zoom

	// Coordinate conversion (client size for view rect)
	Gdiplus::RectF GetViewWorldRect(int clientWidth, int clientHeight, float cullingMargin = 0.0f) const;
	Gdiplus::PointF WorldToScreen(Gdiplus::PointF worldPos) const;
	Gdiplus::RectF WorldToScreen(Gdiplus::RectF worldRect) const;
	Gdiplus::PointF ScreenToWorld(Gdiplus::PointF screenPos) const;

private:
	POINT m_mapOffset;
	float m_zoomFactor;
	const float m_minZoom;
	const float m_maxZoom;
	const float m_zoomStep;
};

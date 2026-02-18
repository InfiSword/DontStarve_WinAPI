#include "../pch.h"
#include "EditorView.h"

EditorView::EditorView()
	: m_mapOffset({ 0, 0 }),
	m_zoomFactor(1.0f),
	m_minZoom(0.25f),
	m_maxZoom(2.0f),
	m_zoomStep(0.25f)
{
}

void EditorView::SetMapOffset(int x, int y)
{
	m_mapOffset.x = x;
	m_mapOffset.y = y;
}

void EditorView::SetMapOffsetClamped(int x, int y, int clientWidth, int clientHeight, int mapWidthTiles, int mapHeightTiles)
{
	const float mapWorldW = (float)(mapWidthTiles * TILE_SIZE);
	const float mapWorldH = (float)(mapHeightTiles * TILE_SIZE);
	const float viewW = (float)clientWidth / m_zoomFactor;
	const float viewH = (float)clientHeight / m_zoomFactor;

	// 오프셋 범위: 뷰포트가 맵 [0, mapWorldW] x [0, mapWorldH] 안에 들어오도록
	// viewWorldLeft = -offset.x/zoom >= 0 => offset.x <= 0
	// viewWorldRight = -offset.x/zoom + viewW <= mapWorldW => offset.x >= clientWidth - mapWorldW*zoom
	int minOffsetX = (int)(clientWidth - mapWorldW * m_zoomFactor);
	int maxOffsetX = 0;
	int minOffsetY = (int)(clientHeight - mapWorldH * m_zoomFactor);
	int maxOffsetY = 0;

	// 맵이 화면보다 작을 때(줌 아웃 시): 범위가 역전되므로 제한 없이 자유롭게 이동 가능
	if (minOffsetX > maxOffsetX) {
		// 맵이 화면보다 작은 경우: 자유롭게 이동 (클램핑 없음)
		m_mapOffset.x = x;
	}
	else {
		// 맵이 화면보다 큰 경우: 맵 경계 내로 제한
		m_mapOffset.x = min(maxOffsetX, max(minOffsetX, x));
	}

	if (minOffsetY > maxOffsetY) {
		// 맵이 화면보다 작은 경우: 자유롭게 이동 (클램핑 없음)
		m_mapOffset.y = y;
	}
	else {
		// 맵이 화면보다 큰 경우: 맵 경계 내로 제한
		m_mapOffset.y = min(maxOffsetY, max(minOffsetY, y));
	}
}

void EditorView::SetZoomFactor(float zoom)
{
	if (zoom < m_minZoom) m_zoomFactor = m_minZoom;
	else if (zoom > m_maxZoom) m_zoomFactor = m_maxZoom;
	else m_zoomFactor = zoom;
}

void EditorView::ZoomIn()
{
	SetZoomFactor(m_zoomFactor + m_zoomStep);
}

void EditorView::ZoomOut()
{
	SetZoomFactor(m_zoomFactor - m_zoomStep);
}

Gdiplus::RectF EditorView::GetViewWorldRect(int clientWidth, int clientHeight, float cullingMargin) const
{
	float viewWorldX = -(Gdiplus::REAL)m_mapOffset.x / m_zoomFactor;
	float viewWorldY = -(Gdiplus::REAL)m_mapOffset.y / m_zoomFactor;
	float viewWorldWidth = (Gdiplus::REAL)clientWidth / m_zoomFactor;
	float viewWorldHeight = (Gdiplus::REAL)clientHeight / m_zoomFactor;

	if (cullingMargin > 0.0f) {
		viewWorldX -= cullingMargin;
		viewWorldY -= cullingMargin;
		viewWorldWidth += 2 * cullingMargin;
		viewWorldHeight += 2 * cullingMargin;
	}

	return Gdiplus::RectF(viewWorldX, viewWorldY, viewWorldWidth, viewWorldHeight);
}

Gdiplus::PointF EditorView::WorldToScreen(Gdiplus::PointF worldPos) const
{
	return Gdiplus::PointF(
		worldPos.X * m_zoomFactor + (Gdiplus::REAL)m_mapOffset.x,
		worldPos.Y * m_zoomFactor + (Gdiplus::REAL)m_mapOffset.y
	);
}

Gdiplus::RectF EditorView::WorldToScreen(Gdiplus::RectF worldRect) const
{
	Gdiplus::PointF screenTopLeft = WorldToScreen(Gdiplus::PointF(worldRect.X, worldRect.Y));
	return Gdiplus::RectF(
		screenTopLeft.X,
		screenTopLeft.Y,
		worldRect.Width * m_zoomFactor,
		worldRect.Height * m_zoomFactor
	);
}

Gdiplus::PointF EditorView::ScreenToWorld(Gdiplus::PointF screenPos) const
{
	return Gdiplus::PointF(
		(screenPos.X - (Gdiplus::REAL)m_mapOffset.x) / m_zoomFactor,
		(screenPos.Y - (Gdiplus::REAL)m_mapOffset.y) / m_zoomFactor
	);
}

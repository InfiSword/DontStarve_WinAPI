#include "../pch.h"
#include "EditorWalkableEditor.h"
#include "../01_EditorView/EditorView.h"
#include "../00_MainEditor/DontStarve_EditorMain.h"
#include "Struct.h"
#include <algorithm>

void EditorWalkableEditor::SetDependencies(EditorView* pView, DontStarve_EditorMain* pMain) {
	m_pView = pView;
	m_pMain = pMain;
}

void EditorWalkableEditor::StartWalkableEdit() {
	m_isWalkableEditMode = true;
	m_isDraggingWalkable = false;
}

void EditorWalkableEditor::EndWalkableEdit() {
	m_isWalkableEditMode = false;
	m_isDraggingWalkable = false;
}

void EditorWalkableEditor::ToggleWalkableEditMode() {
	m_isWalkableEditMode = !m_isWalkableEditMode;
	if (!m_isWalkableEditMode) {
		m_isDraggingWalkable = false;
	}
}

void EditorWalkableEditor::OnLeftButtonDown(POINT clickPoint, HWND hWnd) {
	if (!m_isWalkableEditMode || !m_pView || !m_pMain) return;

	m_isDraggingWalkable = true;
	m_walkableDragStart = clickPoint;
	m_walkableDragEnd = clickPoint;
	SetCapture(hWnd);
}

void EditorWalkableEditor::OnLeftButtonUp() {
	if (!m_isDraggingWalkable || !m_pView || !m_pMain) return;

	// Main의 m_walkableAreaMap에 friend로 접근
	bool(*walkableAreaMap)[MAP_WIDTH] = m_pMain->m_walkableAreaMap;

	Gdiplus::PointF startWorldPos = m_pView->ScreenToWorld(Gdiplus::PointF((float)m_walkableDragStart.x, (float)m_walkableDragStart.y));
	Gdiplus::PointF endWorldPos = m_pView->ScreenToWorld(Gdiplus::PointF((float)m_walkableDragEnd.x, (float)m_walkableDragEnd.y));

	int startTileX = max(0, min(MAP_WIDTH - 1, (int)floor(startWorldPos.X / TILE_SIZE)));
	int startTileY = max(0, min(MAP_HEIGHT - 1, (int)floor(startWorldPos.Y / TILE_SIZE)));
	int endTileX = max(0, min(MAP_WIDTH - 1, (int)floor(endWorldPos.X / TILE_SIZE)));
	int endTileY = max(0, min(MAP_HEIGHT - 1, (int)floor(endWorldPos.Y / TILE_SIZE)));

	int minTileX = min(startTileX, endTileX);
	int maxTileX = max(startTileX, endTileX);
	int minTileY = min(startTileY, endTileY);
	int maxTileY = max(startTileY, endTileY);

	bool newWalkableState = !walkableAreaMap[minTileY][minTileX];
	for (int y = minTileY; y <= maxTileY; ++y) {
		for (int x = minTileX; x <= maxTileX; ++x) {
			walkableAreaMap[y][x] = newWalkableState;
		}
	}

	m_isDraggingWalkable = false;
	ReleaseCapture();

	std::wstringstream debugSS;
	debugSS << L"Walkable area set: (" << minTileX << L"," << minTileY << L") to ("
		<< maxTileX << L"," << maxTileY << L") = " << (newWalkableState ? L"WALKABLE" : L"BLOCKED") << L"\n";
	OutputDebugStringW(debugSS.str().c_str());
}

void EditorWalkableEditor::OnMouseMove(POINT mousePos, HWND hWnd) {
	if (m_isDraggingWalkable && m_isWalkableEditMode) {
		m_walkableDragEnd = mousePos;
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void EditorWalkableEditor::DrawWalkableAreas(Gdiplus::Graphics* pGraphics) const {
	if (!pGraphics || !m_isWalkableEditMode || !m_pView || !m_pMain) return;

	bool(*walkableAreaMap)[MAP_WIDTH] = m_pMain->m_walkableAreaMap;

	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	Gdiplus::PointF viewTopLeft = m_pView->ScreenToWorld(Gdiplus::PointF(0, 0));
	Gdiplus::PointF viewBottomRight = m_pView->ScreenToWorld(Gdiplus::PointF((float)clientRect.right, (float)clientRect.bottom));

	int startX = max(0, (int)floor(viewTopLeft.X / TILE_SIZE));
	int endX = min(MAP_WIDTH, (int)ceil(viewBottomRight.X / TILE_SIZE));
	int startY = max(0, (int)floor(viewTopLeft.Y / TILE_SIZE));
	int endY = min(MAP_HEIGHT, (int)ceil(viewBottomRight.Y / TILE_SIZE));

	Gdiplus::SolidBrush blockedBrush(Gdiplus::Color(100, 255, 0, 0));
	Gdiplus::SolidBrush walkableBrush(Gdiplus::Color(50, 0, 255, 0));
	Gdiplus::Pen gridPen(Gdiplus::Color(150, 255, 255, 255), 1.0f);

	float screenTileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();

	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			float worldX = (float)x * TILE_SIZE;
			float worldY = (float)y * TILE_SIZE;

			float screenX = (worldX - viewTopLeft.X) * m_pView->GetZoomFactor();
			float screenY = (worldY - viewTopLeft.Y) * m_pView->GetZoomFactor();

			if (screenX + screenTileSize < 0 || screenX > clientRect.right ||
				screenY + screenTileSize < 0 || screenY > clientRect.bottom) continue;

			Gdiplus::RectF tileRect(screenX, screenY, screenTileSize, screenTileSize);

			if (walkableAreaMap[y][x]) {
				pGraphics->FillRectangle(&walkableBrush, tileRect);
			}
			else {
				pGraphics->FillRectangle(&blockedBrush, tileRect);
			}

			pGraphics->DrawRectangle(&gridPen, tileRect);
		}
	}

	if (m_isDraggingWalkable) {
		int minX = min(m_walkableDragStart.x, m_walkableDragEnd.x);
		int maxX = max(m_walkableDragStart.x, m_walkableDragEnd.x);
		int minY = min(m_walkableDragStart.y, m_walkableDragEnd.y);
		int maxY = max(m_walkableDragStart.y, m_walkableDragEnd.y);

		Gdiplus::RectF dragRect((float)minX, (float)minY, (float)(maxX - minX), (float)(maxY - minY));
		Gdiplus::Pen dragPen(Gdiplus::Color(255, 255, 255, 0), 3.0f);
		Gdiplus::SolidBrush dragBrush(Gdiplus::Color(50, 255, 255, 0));

		pGraphics->FillRectangle(&dragBrush, dragRect);
		pGraphics->DrawRectangle(&dragPen, dragRect);
	}

	Gdiplus::Font font(L"Arial", 14, Gdiplus::FontStyleBold);
	Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush backgroundBrush(Gdiplus::Color(150, 0, 0, 0));
	Gdiplus::RectF textBgRect(10, 10, 300, 30);
	pGraphics->FillRectangle(&backgroundBrush, textBgRect);
	pGraphics->DrawString(L"Walkable Area Edit Mode - Drag to toggle", -1, &font,
		Gdiplus::PointF(15, 15), &textBrush);
}

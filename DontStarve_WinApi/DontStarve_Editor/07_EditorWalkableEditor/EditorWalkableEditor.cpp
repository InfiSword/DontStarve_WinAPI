#include "../pch.h"
#include "EditorWalkableEditor.h"
#include "../01_EditorView/EditorView.h"
#include "../00_MainEditor/MapEditor.h"

void EditorWalkableEditor::SetDependencies(EditorView* pView, MapEditor* pMain) {
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

void EditorWalkableEditor::SetAllMapBlocked() {
	if (!m_pMain) return;
	bool(*walkableAreaMap)[MAP_WIDTH] = m_pMain->m_walkableAreaMap;
	int w = m_pMain->GetMapWidth();
	int h = m_pMain->GetMapHeight();
	for (int y = 0; y < h; ++y)
		for (int x = 0; x < w; ++x)
			walkableAreaMap[y][x] = false;
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

	bool(*walkableAreaMap)[MAP_WIDTH] = m_pMain->m_walkableAreaMap;
	Gdiplus::PointF startWorldPos = m_pView->ScreenToWorld(Gdiplus::PointF((float)m_walkableDragStart.x, (float)m_walkableDragStart.y));
	Gdiplus::PointF endWorldPos = m_pView->ScreenToWorld(Gdiplus::PointF((float)m_walkableDragEnd.x, (float)m_walkableDragEnd.y));

	int startTileX = max(0, min(m_pMain->GetMapWidth() - 1, (int)floor(startWorldPos.X / TILE_SIZE)));
	int startTileY = max(0, min(m_pMain->GetMapHeight() - 1, (int)floor(startWorldPos.Y / TILE_SIZE)));
	int endTileX = max(0, min(m_pMain->GetMapWidth() - 1, (int)floor(endWorldPos.X / TILE_SIZE)));
	int endTileY = max(0, min(m_pMain->GetMapHeight() - 1, (int)floor(endWorldPos.Y / TILE_SIZE)));

	int minTileX = min(startTileX, endTileX);
	int maxTileX = max(startTileX, endTileX);
	int minTileY = min(startTileY, endTileY);
	int maxTileY = max(startTileY, endTileY);

	bool newWalkableState = (m_paintMode == WalkablePaintMode::PaintWalkable);
	for (int y = minTileY; y <= maxTileY; ++y) {
		for (int x = minTileX; x <= maxTileX; ++x) {
			walkableAreaMap[y][x] = newWalkableState;
		}
	}

	m_isDraggingWalkable = false;
	ReleaseCapture();
	InvalidateRect(g_hWnd, NULL, FALSE);
}

void EditorWalkableEditor::OnMouseMove(POINT mousePos, HWND hWnd) {
	if (m_isDraggingWalkable && m_isWalkableEditMode) {
		m_walkableDragEnd = mousePos;
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

namespace {
	const int kToolbarLeft = 12;
	const int kToolbarTop = 8;
	const int kToolbarBtnW = 100;
	const int kToolbarBtnH = 32;
	const int kToolbarSpacing = 8;
}

bool EditorWalkableEditor::HandleToolbarClick(POINT pt, int clientW, int clientH) {
	(void)clientW;
	(void)clientH;
	Gdiplus::RectF r1((Gdiplus::REAL)kToolbarLeft, (Gdiplus::REAL)kToolbarTop, (Gdiplus::REAL)kToolbarBtnW, (Gdiplus::REAL)kToolbarBtnH);
	Gdiplus::RectF r2((Gdiplus::REAL)(kToolbarLeft + kToolbarBtnW + kToolbarSpacing), (Gdiplus::REAL)kToolbarTop, (Gdiplus::REAL)kToolbarBtnW, (Gdiplus::REAL)kToolbarBtnH);
	Gdiplus::RectF r3((Gdiplus::REAL)(kToolbarLeft + 2 * (kToolbarBtnW + kToolbarSpacing)), (Gdiplus::REAL)kToolbarTop, (Gdiplus::REAL)kToolbarBtnW, (Gdiplus::REAL)kToolbarBtnH);
	if (r1.Contains((Gdiplus::REAL)pt.x, (Gdiplus::REAL)pt.y)) {
		SetAllMapBlocked();
		return true;
	}
	if (r2.Contains((Gdiplus::REAL)pt.x, (Gdiplus::REAL)pt.y)) {
		m_paintMode = WalkablePaintMode::PaintBlocked;
		return true;
	}
	if (r3.Contains((Gdiplus::REAL)pt.x, (Gdiplus::REAL)pt.y)) {
		m_paintMode = WalkablePaintMode::PaintWalkable;
		return true;
	}
	return false;
}

void EditorWalkableEditor::DrawWalkableAreas(Gdiplus::Graphics* pGraphics) const {
	if (!pGraphics || !m_isWalkableEditMode || !m_pView || !m_pMain) return;

	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	bool(*walkableAreaMap)[MAP_WIDTH] = m_pMain->m_walkableAreaMap;
	Gdiplus::PointF viewTopLeft = m_pView->ScreenToWorld(Gdiplus::PointF(0, 0));
	Gdiplus::PointF viewBottomRight = m_pView->ScreenToWorld(Gdiplus::PointF((float)clientRect.right, (float)clientRect.bottom));

	int startX = max(0, (int)floor(viewTopLeft.X / TILE_SIZE));
	int endX = min(m_pMain->GetMapWidth(), (int)ceil(viewBottomRight.X / TILE_SIZE));
	int startY = max(0, (int)floor(viewTopLeft.Y / TILE_SIZE));
	int endY = min(m_pMain->GetMapHeight(), (int)ceil(viewBottomRight.Y / TILE_SIZE));

	Gdiplus::SolidBrush blockedBrush(Gdiplus::Color(100, 255, 0, 0));
	Gdiplus::SolidBrush walkableBrush(Gdiplus::Color(50, 0, 255, 0));
	Gdiplus::Pen gridPen(Gdiplus::Color(150, 255, 255, 255), 1.0f);

	float screenTileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();

	// 1) 그리드(초록=이동가능, 빨강=이동불가) 전체 그림
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

	// 2) 드래그 미리보기
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

	// 3) 상단 툴바 버튼 (그리드보다 위에 그려서 버튼이 항상 보이도록)
	Gdiplus::RectF r1((Gdiplus::REAL)kToolbarLeft, (Gdiplus::REAL)kToolbarTop, (Gdiplus::REAL)kToolbarBtnW, (Gdiplus::REAL)kToolbarBtnH);
	Gdiplus::RectF r2((Gdiplus::REAL)(kToolbarLeft + kToolbarBtnW + kToolbarSpacing), (Gdiplus::REAL)kToolbarTop, (Gdiplus::REAL)kToolbarBtnW, (Gdiplus::REAL)kToolbarBtnH);
	Gdiplus::RectF r3((Gdiplus::REAL)(kToolbarLeft + 2 * (kToolbarBtnW + kToolbarSpacing)), (Gdiplus::REAL)kToolbarTop, (Gdiplus::REAL)kToolbarBtnW, (Gdiplus::REAL)kToolbarBtnH);
	Gdiplus::SolidBrush btnBrush(Gdiplus::Color(255, 70, 130, 180));
	Gdiplus::SolidBrush btnActiveBrush(Gdiplus::Color(255, 100, 160, 220));
	Gdiplus::Pen btnPen(Gdiplus::Color(255, 50, 80, 120), 1.0f);
	Gdiplus::Pen btnActivePen(Gdiplus::Color(255, 255, 255, 255), 2.0f);
	Gdiplus::Font btnFont(L"Malgun Gothic", 10, Gdiplus::FontStyleRegular);
	Gdiplus::SolidBrush btnTextBrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::StringFormat sf;
	sf.SetAlignment(Gdiplus::StringAlignmentCenter);
	sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);

	auto drawButton = [&](const Gdiplus::RectF& rect, const WCHAR* text, bool active) {
		pGraphics->FillRectangle(active ? &btnActiveBrush : &btnBrush, rect);
		pGraphics->DrawRectangle(active ? &btnActivePen : &btnPen, rect);
		pGraphics->DrawString(text, -1, &btnFont, rect, &sf, &btnTextBrush);
	};
	drawButton(r1, L"전체 이동불가", false);
	drawButton(r2, L"이동 불가", m_paintMode == WalkablePaintMode::PaintBlocked);
	drawButton(r3, L"이동 가능", m_paintMode == WalkablePaintMode::PaintWalkable);

	// 4) 힌트 텍스트
	Gdiplus::Font font(L"Arial", 12, Gdiplus::FontStyleBold);
	Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush backgroundBrush(Gdiplus::Color(150, 0, 0, 0));
	Gdiplus::RectF textBgRect(10, (Gdiplus::REAL)(kToolbarTop + kToolbarBtnH + 4), 320, 24);
	pGraphics->FillRectangle(&backgroundBrush, textBgRect);
	const WCHAR* modeStr = (m_paintMode == WalkablePaintMode::PaintWalkable) ? L"이동 가능" : L"이동 불가";
	std::wstring hint = std::wstring(L"Walkable - Drag to paint (") + modeStr + L")";
	pGraphics->DrawString(hint.c_str(), -1, &font,
		Gdiplus::PointF(15, (Gdiplus::REAL)(kToolbarTop + kToolbarBtnH + 6)), &textBrush);
}

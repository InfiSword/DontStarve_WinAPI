#include "../pch.h"
#include "MapEditor.h"
#include "../Resource.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../03_EditorMapFileIO/EditorMapFileIO.h"
#include "../04_EditorPalette/EditorPalette.h"
#include "../07_EditorWalkableEditor/EditorWalkableEditor.h"
#include "../08_EditorDebugPanel/EditorDebugPanel.h"
#include "../09_EditorLayerComposer/EditorLayerComposer.h"

MapEditor::MapEditor()
	: m_pGraphics(nullptr), m_pDoubleBufferBitmap(nullptr),
	m_isPlacingMode(false), m_is3x3Mode(false),
	m_rawMousePos({ 0,0 }), m_snappedPreviewPos(0.0f, 0.0f),
	m_pView(std::make_unique<EditorView>()),
	m_pResources(std::make_unique<EditorResourceManager>()),
	m_pPalette(std::make_unique<EditorPalette>()),
	m_pWalkableEditor(std::make_unique<EditorWalkableEditor>()),
	m_pDebugPanel(std::make_unique<EditorDebugPanel>()),
	m_pLayerComposer(std::make_unique<EditorLayerComposer>()),
	m_objectsDirty(true),
	m_selectedObjectPtr(nullptr),
	m_paletteLayerBitmap(nullptr), m_paletteLayerDirty(true),
	m_hasPlayerSpawn(false), m_playerSpawnPoint(0.0f, 0.0f), m_isPlayerSpawnMode(false),
	m_mapWidth(MAP_WIDTH), m_mapHeight(MAP_HEIGHT)
{
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
		}
	}

	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_walkableAreaMap[y][x] = true;
		}
	}
}


MapEditor::~MapEditor()
{
	Release();
}


void MapEditor::Initialize() {
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
	m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);

	m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
	m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);

	const UINT INIT_LAYER_SIZE = 1024;

	m_pResources->LoadResources();
	m_pLayerComposer->SetDependencies(m_pView.get(), m_pResources.get(), this);
	m_pLayerComposer->ResizeLayerBitmaps(INIT_LAYER_SIZE, INIT_LAYER_SIZE);
	InitPalette();

	const RECT& pr = m_pPalette->GetPaletteRect();
	int paletteW = pr.right - pr.left;
	int paletteH = pr.bottom - pr.top;
	if (paletteW > 0 && paletteH > 0) {
		m_paletteLayerBitmap = new Gdiplus::Bitmap(paletteW, paletteH, PixelFormat32bppARGB);
	}

	m_pDebugPanel->SetDependencies(m_pView.get(), m_pPalette.get(), nullptr, nullptr, m_pWalkableEditor.get(), this);
	m_pWalkableEditor->SetDependencies(m_pView.get(), this);

	float centerX = ((float)((m_mapWidth - 1) / 2) + 0.5f) * TILE_SIZE;
	float centerY = ((float)((m_mapHeight - 1) / 2) + 0.5f) * TILE_SIZE;
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;

	// 화면과 맵의 중앙이 맞도록 초기 카메라 오프셋 설정
	int initialOffsetX = (clientRect.right / 2) - (int)centerX;
	int initialOffsetY = (clientRect.bottom / 2) - (int)centerY;
	m_pView->SetMapOffsetClamped(initialOffsetX, initialOffsetY, clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);

	m_pLayerComposer->ComposeGridLayer();
	m_pLayerComposer->ComposeTileLayer();
	m_pLayerComposer->ComposeObjectLayer();
	m_pPalette->ComposePaletteLayer(m_paletteLayerBitmap, &m_paletteLayerDirty);

	UpdateLauncherButtonRect(clientRect.right, clientRect.bottom);
}

void MapEditor::UpdateLauncherButtonRect(int clientW, int clientH) {
	const int btnW = 120;
	const int btnH = 32;
	const int margin = 12;
	m_rectLauncherButton = Gdiplus::RectF((Gdiplus::REAL)margin, (Gdiplus::REAL)(clientH - margin - btnH), (Gdiplus::REAL)btnW, (Gdiplus::REAL)btnH);
}

bool MapEditor::IsPointInLauncherButton(POINT pt) const {
	return m_rectLauncherButton.Contains((Gdiplus::REAL)pt.x, (Gdiplus::REAL)pt.y) != FALSE;
}

void MapEditor::InitPalette() {
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	m_pPalette->InitPalette(clientRect.right, clientRect.bottom, m_pResources.get());
}

void MapEditor::Update()
{

}

void MapEditor::Render()
{
	if (!m_pGraphics) return;

	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	m_pGraphics->Clear(Gdiplus::Color(255, 255, 255, 255));

	UINT targetWidth = max(512U, min((UINT)clientRect.right + 256, 1536U));
	UINT targetHeight = max(512U, min((UINT)clientRect.bottom + 256, 1536U));

	Gdiplus::Bitmap* tileLayerBitmap = m_pLayerComposer->GetTileLayerBitmap();
	if (!tileLayerBitmap ||
		abs((int)tileLayerBitmap->GetWidth() - (int)targetWidth) > 128 ||
		abs((int)tileLayerBitmap->GetHeight() - (int)targetHeight) > 128) {

		m_pLayerComposer->ResizeLayerBitmaps(targetWidth, targetHeight);
	}

	m_pLayerComposer->ComposeGridLayer();
	m_pLayerComposer->ComposeTileLayer();
	m_pLayerComposer->ComposeObjectLayer();

	m_pPalette->ComposePaletteLayer(m_paletteLayerBitmap, &m_paletteLayerDirty);

	m_pLayerComposer->DrawLayers(m_pGraphics);

	if (m_paletteLayerBitmap) 
	{
		const RECT& pr = m_pPalette->GetPaletteRect();
		m_pGraphics->DrawImage(m_paletteLayerBitmap,
			(Gdiplus::REAL)pr.left, (Gdiplus::REAL)pr.top,
			(Gdiplus::REAL)(pr.right - pr.left),
			(Gdiplus::REAL)(pr.bottom - pr.top));
	}

	DrawPreview(m_pGraphics);
	m_pPalette->DrawSubPalette(m_pGraphics);
	DrawPlayerSpawn(m_pGraphics);
		m_pWalkableEditor->DrawWalkableAreas(m_pGraphics);

	if (m_pDebugPanel->IsVisible()) {
		m_pDebugPanel->DrawDebugInfo(m_pGraphics);
	}

	{
		Gdiplus::SolidBrush btnBrush(Gdiplus::Color(255, 70, 130, 180));
		Gdiplus::Pen btnPen(Gdiplus::Color(255, 50, 80, 120), 2.0f);
		m_pGraphics->FillRectangle(&btnBrush, m_rectLauncherButton);
		m_pGraphics->DrawRectangle(&btnPen, m_rectLauncherButton);
		Gdiplus::Font font(L"Malgun Gothic", 12, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
		Gdiplus::StringFormat sf;
		sf.SetAlignment(Gdiplus::StringAlignmentCenter);
		sf.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		Gdiplus::RectF textRect(m_rectLauncherButton.X, m_rectLauncherButton.Y, m_rectLauncherButton.Width, m_rectLauncherButton.Height);
		m_pGraphics->DrawString(L"Launcher", -1, &font, textRect, &sf, &textBrush);
	}

	HDC hdcScreen = GetDC(g_hWnd);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = NULL;
	Gdiplus::Color color(0, 0, 0, 0);
	if (m_pDoubleBufferBitmap->GetHBITMAP(color, &hBitmap) == Gdiplus::Ok && hBitmap) {
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
		BitBlt(hdcScreen, 0, 0, clientRect.right, clientRect.bottom, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
	} else {
		Gdiplus::Graphics screenGraphics(hdcScreen);
		screenGraphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		screenGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		screenGraphics.DrawImage(m_pDoubleBufferBitmap, 0, 0, clientRect.right, clientRect.bottom);
		if (hdcMem) DeleteDC(hdcMem);
	}
	ReleaseDC(g_hWnd, hdcScreen);
}

void MapEditor::Release()
{
	m_pResources->ReleaseResources();

	Utils::SafeDelete(m_pGraphics);
	Utils::SafeDelete(m_pDoubleBufferBitmap);
	Utils::SafeDelete(m_paletteLayerBitmap);
}

EditorScreenSwitch MapEditor::GetRequestedSwitch() {
	EditorScreenSwitch s = m_requestedSwitch;
	m_requestedSwitch = EditorScreenSwitch::None;
	return s;
}

Gdiplus::Bitmap* MapEditor::GetTileLayerBitmap() const {
	return m_pLayerComposer ? m_pLayerComposer->GetTileLayerBitmap() : nullptr;
}

bool MapEditor::GetWalkableAt(int x, int y) const {
	if (x < 0 || x >= m_mapWidth || y < 0 || y >= m_mapHeight) return false;
	return m_walkableAreaMap[y][x];
}

size_t MapEditor::GetBitmapCacheSize() const {
	return m_pResources ? m_pResources->GetBitmapCacheSize() : 0;
}

LRESULT MapEditor::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE: {
		return 0;
	}

	case WM_PAINT: {
		PAINTSTRUCT ps;
		BeginPaint(hWnd, &ps);
		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_SIZE: {
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);

		Utils::SafeDelete(m_pDoubleBufferBitmap);
		Utils::SafeDelete(m_pGraphics);
		m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
		m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
		m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);

		m_pPalette->InitPalette(clientRect.right, clientRect.bottom, m_pResources.get());
		Utils::SafeDelete(m_paletteLayerBitmap);
		const RECT& pr = m_pPalette->GetPaletteRect();
		m_paletteLayerBitmap = new Gdiplus::Bitmap(pr.right - pr.left, pr.bottom - pr.top, PixelFormat32bppARGB);
		m_paletteLayerDirty = true;

		UpdateLauncherButtonRect(clientRect.right, clientRect.bottom);

		m_pLayerComposer->SetGridLayerDirty(true);
		m_pLayerComposer->SetTileLayerDirty(true);
		m_pLayerComposer->SetObjectLayerDirty(true);

		InvalidateRect(hWnd, NULL, FALSE);
		return 0;
	}

	case WM_MOUSEMOVE:
	{
		POINT newMousePos = { LOWORD(lParam), HIWORD(lParam) };

		m_rawMousePos = newMousePos;

		if (m_isDraggingCamera) {
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			int deltaX = m_rawMousePos.x - m_cameraDragStart.x;
			int deltaY = m_rawMousePos.y - m_cameraDragStart.y;

			POINT oldOffset = m_pView->GetMapOffset();
			m_pView->SetMapOffsetClamped(
				m_initialMapOffset.x + deltaX, m_initialMapOffset.y + deltaY,
				clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
			
			m_pLayerComposer->SetGridLayerDirty(true);
			m_pLayerComposer->SetTileLayerDirty(true);
			m_pLayerComposer->SetObjectLayerDirty(true);
			
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (m_pWalkableEditor->IsDraggingWalkable()) {
			m_pWalkableEditor->OnMouseMove(m_rawMousePos, hWnd);
			return 0;
		}

		if (m_isErasingMode) {
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		int selIdx = m_pPalette->GetSelectedPaletteIndex();
		if (selIdx != -1 && m_isPlacingMode) {
			const PaletteItem* pItem = m_pPalette->GetPaletteItem((size_t)selIdx);
			if (pItem) {
				Gdiplus::PointF oldPreviewPos = m_snappedPreviewPos;
				Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)m_rawMousePos.x, (float)m_rawMousePos.y));

				if (pItem->category == CATEGORY_TILE) {
					int snappedMapX = (int)floor(mouseWorldPos.X / TILE_SIZE);
					int snappedMapY = (int)floor(mouseWorldPos.Y / TILE_SIZE);
					snappedMapX = max(0, min(m_mapWidth - 1, snappedMapX));
					snappedMapY = max(0, min(m_mapHeight - 1, snappedMapY));
					m_snappedPreviewPos = Gdiplus::PointF((float)(snappedMapX * TILE_SIZE), (float)(snappedMapY * TILE_SIZE));
				}
				else if (pItem->category == CATEGORY_OBJECT) {
					m_snappedPreviewPos = mouseWorldPos;
				}

				if (oldPreviewPos.X != m_snappedPreviewPos.X || oldPreviewPos.Y != m_snappedPreviewPos.Y) {
					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
		}
	}
	return 0;

	case WM_LBUTTONDOWN: {
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		POINT clickPoint = { mouseX, mouseY };

		if (IsPointInLauncherButton(clickPoint)) {
			m_requestedSwitch = EditorScreenSwitch::BackToLauncher;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (IsPointInDebugPanel(clickPoint)) {
			return 0;
		}

		const RECT& paletteRectFirst = m_pPalette->GetPaletteRect();
		bool clickOnPaletteFirst = PtInRect(&paletteRectFirst, clickPoint) != FALSE;
		
		if (m_isErasingMode && !clickOnPaletteFirst) {
			HandleErasingModeClick(clickPoint, hWnd);
			return 0;
		}
		
		if (m_isPlacingMode && !clickOnPaletteFirst) {
			HandlePlacingModeClick(clickPoint, hWnd);
			return 0;
		}

		if (m_pWalkableEditor->IsWalkableEditMode()) {
			if (m_pWalkableEditor->HandleToolbarClick(clickPoint, clientRect.right, clientRect.bottom)) {
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			m_pWalkableEditor->OnLeftButtonDown(clickPoint, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (m_isPlayerSpawnMode) {
			if (GetKeyState(VK_RBUTTON) & 0x8000) {
				return 0;
			}
			Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

			const float maxX = (float)(m_mapWidth * TILE_SIZE);
			const float maxY = (float)(m_mapHeight * TILE_SIZE);
			const bool isInBounds = (mouseWorldPos.X >= 0 && mouseWorldPos.X <= maxX &&
									 mouseWorldPos.Y >= 0 && mouseWorldPos.Y <= maxY);

			if (isInBounds) {
				m_playerSpawnPoint = mouseWorldPos;
				m_hasPlayerSpawn = true;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;
		}

		auto subResult = m_pPalette->HandleSubPaletteClick(clickPoint);
		if (subResult != EditorPalette::SubPaletteClickResult::NotHandled) {
			if (subResult == EditorPalette::SubPaletteClickResult::ClosedWithSelection) {
				m_isPlacingMode = true;
				m_paletteLayerDirty = true;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			else {
				bool hasSelection = (m_pPalette->GetSelectedPaletteIndex() >= 0 &&
					(m_pPalette->GetSelectedTileVariant() != nullptr || m_pPalette->GetSelectedObjectVariant() != nullptr));
				
				m_isPlacingMode = hasSelection;
				m_paletteLayerDirty = true;
				InvalidateRect(hWnd, NULL, FALSE);
				
				if (!hasSelection) {
					m_selectedObjectPtr = nullptr;
					return 0;
				}
			}
		}

		const RECT& paletteRect = m_pPalette->GetPaletteRect();
		bool clickOnPaletteArea = PtInRect(&paletteRect, clickPoint) != FALSE;
		if (clickOnPaletteArea) {
			if (m_pPalette->HandleMainPaletteClick(clickPoint, clientRect.bottom)) {
				m_isPlacingMode = false;
				m_paletteLayerDirty = true;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		if (m_isPlacingMode && !clickOnPaletteArea) {
			HandlePlacingModeClick(clickPoint, hWnd);
		}
		else if (!m_isPlacingMode) {
			HandleObjectSelectionClick(clickPoint, hWnd);
		}
	}
	return 0;

	case WM_RBUTTONDOWN:
	{
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		POINT clickPoint = { mouseX, mouseY };

		if (IsPointInDebugPanel(clickPoint)) {
			return 0;
		}

		if (m_pPalette->IsSubPaletteOpen()) {
			m_pPalette->CloseSubPalette();
			m_isPlacingMode = false;
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (m_isPlacingMode) {
			m_isPlacingMode = false;
			m_pPalette->ResetSelection();
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		const RECT& pr = m_pPalette->GetPaletteRect();
		if (!PtInRect(&pr, clickPoint)) {
			m_isDraggingCamera = true;
			m_cameraDragStart = clickPoint;
			m_initialMapOffset = m_pView->GetMapOffset();
			SetCapture(hWnd);
		}
	}
	return 0;

	case WM_CAPTURECHANGED:
	{
		if (m_isDraggingCamera) {
			m_isDraggingCamera = false;
		}
		if (m_pWalkableEditor->IsDraggingWalkable()) {
			m_pWalkableEditor->OnLeftButtonUp();
		}
	}
	return 0;

	case WM_RBUTTONUP:
	{
		if (m_isDraggingCamera) {
			int deltaX = m_rawMousePos.x - m_cameraDragStart.x;
			int deltaY = m_rawMousePos.y - m_cameraDragStart.y;
			int dragDistanceSquared = deltaX * deltaX + deltaY * deltaY;

			m_isDraggingCamera = false;
			ReleaseCapture();

			if (dragDistanceSquared <= 25) {
				POINT clickPoint = { m_rawMousePos.x, m_rawMousePos.y };
				HandleObjectSelectionClick(clickPoint, hWnd);
			}
			return 0;
		}
	}
	return 0;

	case WM_MOUSEWHEEL:
	{
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		POINT mouseScreenPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		POINT mouseClientPos = mouseScreenPos;
		ScreenToClient(hWnd, &mouseClientPos);

		if (m_pDebugPanel->IsVisible()) {
			int mx = mouseClientPos.x;
			int my = mouseClientPos.y;
			Gdiplus::RectF vr = m_pDebugPanel->GetViewportRect();
			if (vr.Width > 0 && vr.Height > 0 &&
				(float)mx >= vr.X && (float)mx < vr.X + vr.Width &&
				(float)my >= vr.Y && (float)my < vr.Y + vr.Height) {
				m_pDebugPanel->HandleMouseWheel(zDelta, mx, my);
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		Gdiplus::PointF mouseWorldPos_before_zoom = ScreenToWorld(Gdiplus::PointF((float)mouseScreenPos.x, (float)mouseScreenPos.y));

		float oldZoomFactor = m_pView->GetZoomFactor();

		if (zDelta > 0) {
			m_pView->ZoomIn();
		}
		else {
			m_pView->ZoomOut();
		}

		float newZoomFactor = m_pView->GetZoomFactor();

		if (newZoomFactor != oldZoomFactor) {
			Gdiplus::PointF mouseScreenPos_after_zoom = WorldToScreen(mouseWorldPos_before_zoom);

			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			POINT mo = m_pView->GetMapOffset();
			m_pView->SetMapOffsetClamped(
				mo.x + (LONG)(mouseScreenPos.x - mouseScreenPos_after_zoom.X),
				mo.y + (LONG)(mouseScreenPos.y - mouseScreenPos_after_zoom.Y),
				clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);

			m_pLayerComposer->SetGridLayerDirty(true);
			m_pLayerComposer->SetTileLayerDirty(true);
			m_pLayerComposer->SetObjectLayerDirty(true);

			InvalidateRect(hWnd, NULL, FALSE);
		}
		return 0;
	}

	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE) {
			bool needsRedraw = false;

			if (m_pPalette->IsSubPaletteOpen()) {
				m_pPalette->CloseSubPalette();
				m_isPlacingMode = false;
				m_paletteLayerDirty = true;
				needsRedraw = true;
			}
			else if (m_isErasingMode) {
				m_isErasingMode = false;
				m_is3x3Mode = false;
				needsRedraw = true;
			}
			else if (m_isPlayerSpawnMode) {
				m_isPlayerSpawnMode = false;
				needsRedraw = true;
			}
			else if (m_pWalkableEditor->IsWalkableEditMode()) {
				m_pWalkableEditor->EndWalkableEdit();
				needsRedraw = true;
			}
			else if (m_selectedObjectPtr) {
				DeselectObject(hWnd);
				needsRedraw = true;
			}
			else {
				m_requestedSwitch = EditorScreenSwitch::BackToLauncher;
			}

			if (needsRedraw) {
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;
		}

		if (wParam == VK_SHIFT) {
			if (m_isPlacingMode) {
				int sp = m_pPalette->GetSelectedPaletteIndex();
				const PaletteItem* pi = (sp >= 0) ? m_pPalette->GetPaletteItem((size_t)sp) : nullptr;
				if (pi && pi->category == CATEGORY_TILE) {
					m_is3x3Mode = !m_is3x3Mode;
					InvalidateRect(hWnd, NULL, FALSE);
					return 0;
				}
			} else if (m_isErasingMode) {
				m_is3x3Mode = !m_is3x3Mode;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		if (wParam == 'E') {
			if (m_isErasingMode) {
				m_isErasingMode = false;
				m_is3x3Mode = false;
			} else {
				ExitAllEditModes();
				m_isErasingMode = true;
				m_is3x3Mode = false;
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (wParam == 'P') {
			if (m_isPlayerSpawnMode) {
				m_isPlayerSpawnMode = false;
			} else {
				ExitAllEditModes();
				m_isPlayerSpawnMode = true;
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (wParam == VK_F1) {
			m_pDebugPanel->ToggleVisibility();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (wParam == 'R') {
			if (!m_selectedObjectPtr) {
				MessageBoxW(hWnd, L"삭제할 오브젝트를 먼저 선택해주세요.", L"오브젝트 삭제", MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			if (MessageBoxW(hWnd, L"선택된 오브젝트를 삭제하시겠습니까?", L"오브젝트 삭제", MB_YESNO | MB_ICONQUESTION) == IDYES) {
				RemoveObject(m_selectedObjectPtr);
				m_selectedObjectPtr = nullptr;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;
		}

		if (wParam == 'G') {
			bool wasActive = m_pWalkableEditor->IsWalkableEditMode();
			if (!wasActive)
				ExitAllEditModes();
			m_pWalkableEditor->ToggleWalkableEditMode();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}


		return 0;
	}

	case WM_LBUTTONUP:
	{
		if (m_pWalkableEditor->IsDraggingWalkable()) {
			m_pWalkableEditor->OnLeftButtonUp();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}
	}
	break;

	case WM_DESTROY: {
		PostQuitMessage(0);
		return 0;
	}

	default:
		return DefWindowProc(hWnd, message, wParam, lParam);
	}
	return 0;
}

void MapEditor::NewMap() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
		}
	}
	m_gameObjects.clear();
	ReleaseCapture();
	m_selectedObjectPtr = nullptr;
	m_isPlacingMode = false;
	m_pWalkableEditor->EndWalkableEdit();
	m_isPlayerSpawnMode = false;
	m_isDraggingCamera = false;
	m_pPalette->ResetSelection();
	ReleaseCapture();
	float centerX = ((float)((m_mapWidth - 1) / 2) + 0.5f) * TILE_SIZE;
	float centerY = ((float)((m_mapHeight - 1) / 2) + 0.5f) * TILE_SIZE;
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_walkableAreaMap[y][x] = true;
		}
	}
	m_pView->SetZoomFactor(1.0f);
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	int initialOffsetX = (clientRect.right / 2) - (int)centerX;
	int initialOffsetY = (clientRect.bottom / 2) - (int)centerY;
	m_pView->SetMapOffsetClamped(initialOffsetX, initialOffsetY, clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
	m_pLayerComposer->SetTileLayerDirty(true);
	m_pLayerComposer->SetObjectLayerDirty(true);
	m_objectsDirty = true;
	m_paletteLayerDirty = true;
	m_pLayerComposer->SetGridLayerDirty(true);
}

void MapEditor::SetMapSize(int width, int height) {
	int w = max(1, min(MAP_WIDTH, width));
	int h = max(1, min(MAP_HEIGHT, height));
	if (w == m_mapWidth && h == m_mapHeight) return;
	m_mapWidth = w;
	m_mapHeight = h;
	m_gameObjects.clear();
	m_selectedObjectPtr = nullptr;
	m_isPlacingMode = false;
	m_pWalkableEditor->EndWalkableEdit();
	m_isPlayerSpawnMode = false;
	m_pPalette->ResetSelection();
	float centerX = ((float)((m_mapWidth - 1) / 2) + 0.5f) * TILE_SIZE;
	float centerY = ((float)((m_mapHeight - 1) / 2) + 0.5f) * TILE_SIZE;
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
			m_walkableAreaMap[y][x] = true;
		}
	}
	m_pView->SetZoomFactor(1.0f);
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	int initialOffsetX = (clientRect.right / 2) - (int)centerX;
	int initialOffsetY = (clientRect.bottom / 2) - (int)centerY;
	m_pView->SetMapOffsetClamped(initialOffsetX, initialOffsetY, clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
	m_pLayerComposer->SetTileLayerDirty(true);
	m_pLayerComposer->SetObjectLayerDirty(true);
	m_objectsDirty = true;
	m_paletteLayerDirty = true;
	m_pLayerComposer->SetGridLayerDirty(true);
}

bool MapEditor::SaveMap(const WCHAR* filename) {
	return EditorMapFileIO::SaveMap(this, filename);
}

bool MapEditor::LoadMap(const WCHAR* filename) {
	return EditorMapFileIO::LoadMap(this, filename);
}

bool MapEditor::ShowSaveFileDialog(WCHAR* fileName, DWORD fileNameSize) {
	return EditorMapFileIO::ShowSaveFileDialog(this, fileName, fileNameSize);
}

bool MapEditor::ShowOpenFileDialog(WCHAR* fileName, DWORD fileNameSize) {
	return EditorMapFileIO::ShowOpenFileDialog(this, fileName, fileNameSize);
}

void MapEditor::DrawPreview(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;
	
	// 제거 모드 프리뷰
	if (m_isErasingMode) {
		Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)m_rawMousePos.x, (float)m_rawMousePos.y));
		int mapX = (int)floor(mouseWorldPos.X / TILE_SIZE);
		int mapY = (int)floor(mouseWorldPos.Y / TILE_SIZE);
		mapX = max(0, min(m_mapWidth - 1, mapX));
		mapY = max(0, min(m_mapHeight - 1, mapY));

		Gdiplus::PointF snappedWorldPos((float)(mapX * TILE_SIZE), (float)(mapY * TILE_SIZE));
		Gdiplus::PointF screenPos = WorldToScreen(snappedWorldPos);
		float tileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();

		if (m_is3x3Mode) {
			Gdiplus::Pen erasePen(Gdiplus::Color(200, 255, 0, 0), 2.0f);
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int targetX = mapX + dx;
					int targetY = mapY + dy;
					if (targetX >= 0 && targetX < m_mapWidth && targetY >= 0 && targetY < m_mapHeight) {
						Gdiplus::PointF worldGridPos((float)(targetX * TILE_SIZE), (float)(targetY * TILE_SIZE));
						Gdiplus::PointF screenGridPos = WorldToScreen(worldGridPos);
						Gdiplus::RectF gridRect(screenGridPos.X, screenGridPos.Y, tileSize, tileSize);
						pGraphics->DrawRectangle(&erasePen, gridRect);
						
						Gdiplus::SolidBrush fillBrush(Gdiplus::Color(80, 255, 0, 0));
						pGraphics->FillRectangle(&fillBrush, gridRect);
					}
				}
			}
		} else {
			Gdiplus::Pen erasePen(Gdiplus::Color(200, 255, 0, 0), 2.0f);
			Gdiplus::RectF gridRect(screenPos.X, screenPos.Y, tileSize, tileSize);
			pGraphics->DrawRectangle(&erasePen, gridRect);
			
			Gdiplus::SolidBrush fillBrush(Gdiplus::Color(80, 255, 0, 0));
			pGraphics->FillRectangle(&fillBrush, gridRect);
		}

		// 제거 모드 안내 텍스트
		Gdiplus::Font infoFont(L"Arial", 14, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 0));
		Gdiplus::SolidBrush backBrush(Gdiplus::Color(150, 0, 0, 0));
		
		std::wstring modeText = L"[TILE ERASE MODE] Click to erase";
		if (m_is3x3Mode) modeText += L" (3x3)";
		modeText += L" | E: Exit | Shift: Toggle 3x3";
		
		Gdiplus::RectF textRect(10, 40, 600, 30);
		pGraphics->FillRectangle(&backBrush, textRect);
		pGraphics->DrawString(modeText.c_str(), -1, &infoFont, textRect, nullptr, &textBrush);
		return;
	}
	
	// 기존 배치 모드 프리뷰
	if (!m_isPlacingMode) return;

	int selIdx = m_pPalette->GetSelectedPaletteIndex();
	const PaletteItem* pSelectedItem = (selIdx >= 0) ? m_pPalette->GetPaletteItem((size_t)selIdx) : nullptr;
	if (!pSelectedItem) return;
	const PaletteItem& selectedItem = *pSelectedItem;
	Gdiplus::Bitmap* previewBitmap = nullptr;
	Gdiplus::RectF previewSourceRect;

	if (selectedItem.category == CATEGORY_TILE) {
		const ResourcePathUtils::TileResourceDef* tv = m_pPalette->GetSelectedTileVariant();
		if (tv && !tv->imageName.empty()) {
			std::wstring fullPath = ResourcePathUtils::BuildResourcePath(tv->baseDir, tv->imageName);
			std::shared_ptr<Gdiplus::Bitmap> sharedBitmap = m_pResources->GetCachedBitmap(fullPath);
			if (sharedBitmap) {
				previewBitmap = sharedBitmap.get();
				previewSourceRect = Gdiplus::RectF(0, 0, (float)previewBitmap->GetWidth(), (float)previewBitmap->GetHeight());
			}
		}
	}
	else if (selectedItem.category == CATEGORY_OBJECT) {
		const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
		if (ov && !ov->imageName.empty()) {
			std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
			std::shared_ptr<Gdiplus::Bitmap> sharedBitmap = m_pResources->GetCachedBitmap(fullPath);
			if (sharedBitmap) {
				previewBitmap = sharedBitmap.get();
				previewSourceRect = Gdiplus::RectF(0, 0, (float)previewBitmap->GetWidth(), (float)previewBitmap->GetHeight());
			}
		}
	}

	if (!previewBitmap) return;

	Gdiplus::ColorMatrix colorMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.6f, 0.0f,
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f 
	};

	Gdiplus::ImageAttributes imageAttr;
	imageAttr.SetColorMatrix(&colorMatrix);

	Gdiplus::PointF screenPreviewPos = WorldToScreen(m_snappedPreviewPos);
	float finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight;

	if (selectedItem.category == CATEGORY_TILE) {
		finalRenderWidth = (float)TILE_SIZE * m_pView->GetZoomFactor();
		finalRenderHeight = (float)TILE_SIZE * m_pView->GetZoomFactor();
		finalRenderX = screenPreviewPos.X;
		finalRenderY = screenPreviewPos.Y;

		// 중앙 타일 프리뷰 (항상 표시)
		Gdiplus::Pen previewGridPen(Gdiplus::Color(150, 255, 255, 0), 2.0f);
		Gdiplus::RectF previewGridRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
		pGraphics->DrawRectangle(&previewGridPen, previewGridRect);

		// 3x3 모드일 때 초록색 강조 표시
		if (m_is3x3Mode) {
			// 초록색 테두리와 반투명 초록색 배경으로 강조
			Gdiplus::Pen gridPen3x3(Gdiplus::Color(220, 0, 255, 100), 3.0f);  // 더 두꺼운 초록색 테두리
			Gdiplus::SolidBrush fillBrush3x3(Gdiplus::Color(100, 100, 255, 150));  // 반투명 초록색 배경
			float tileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();

			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					float worldGridX = m_snappedPreviewPos.X + (dx * TILE_SIZE);
					float worldGridY = m_snappedPreviewPos.Y + (dy * TILE_SIZE);
					Gdiplus::PointF screenGridPos = WorldToScreen(Gdiplus::PointF(worldGridX, worldGridY));

					Gdiplus::RectF gridRect(screenGridPos.X, screenGridPos.Y, tileSize, tileSize);
					
					// 반투명 초록색 배경 먼저 그리기
					pGraphics->FillRectangle(&fillBrush3x3, gridRect);
					
					// 초록색 테두리 그리기
					pGraphics->DrawRectangle(&gridPen3x3, gridRect);
				}
			}
			
			// 전체 3x3 영역을 감싸는 외곽 테두리 (더욱 강조)
			Gdiplus::PointF topLeftWorld(m_snappedPreviewPos.X - TILE_SIZE, m_snappedPreviewPos.Y - TILE_SIZE);
			Gdiplus::PointF topLeftScreen = WorldToScreen(topLeftWorld);
			Gdiplus::Pen outerPen(Gdiplus::Color(255, 0, 255, 50), 4.0f);  // 더 두꺼운 외곽선
			Gdiplus::RectF outerRect(topLeftScreen.X, topLeftScreen.Y, tileSize * 3.0f, tileSize * 3.0f);
			pGraphics->DrawRectangle(&outerPen, outerRect);
		}
	}
	else if (selectedItem.category == CATEGORY_OBJECT) {
		const ResourcePathUtils::ObjectResourceDef* ov_preview = m_pPalette->GetSelectedObjectVariant();
		if (!ov_preview) return;

		finalRenderWidth = previewSourceRect.Width * m_pView->GetZoomFactor();
		finalRenderHeight = previewSourceRect.Height * m_pView->GetZoomFactor();
		finalRenderX = screenPreviewPos.X - (ov_preview->pivotX * finalRenderWidth);
		finalRenderY = screenPreviewPos.Y - (ov_preview->pivotY * finalRenderHeight);

		Gdiplus::Pen previewBBoxPen(Gdiplus::Color(150, 0, 255, 255), 1.5f);
		Gdiplus::RectF previewBBoxRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
		pGraphics->DrawRectangle(&previewBBoxPen, previewBBoxRect);

		Gdiplus::SolidBrush pivotBrush(Gdiplus::Color(200, 255, 0, 0));
		Gdiplus::RectF pivotRect(screenPreviewPos.X - 3.0f, screenPreviewPos.Y - 3.0f, 6.0f, 6.0f);
		pGraphics->FillEllipse(&pivotBrush, pivotRect);
	}

	Gdiplus::RectF destRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
	pGraphics->DrawImage(previewBitmap, destRect,
		0, 0, previewSourceRect.Width, previewSourceRect.Height,
		Gdiplus::UnitPixel, &imageAttr);

	Gdiplus::Font infoFont(L"Arial", 14);
	Gdiplus::SolidBrush infoBrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush infoBackBrush(Gdiplus::Color(150, 0, 0, 0));

	std::wstringstream infoSS;
	if (selectedItem.category == CATEGORY_TILE) {
		float screenTileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();
		infoSS << L"Tile Preview: " << (int)screenTileSize << L"px (World: " << TILE_SIZE << L"px)";
		if (m_is3x3Mode) infoSS << L" [3x3 Mode]";
	}
	else {
		float screenWidth = previewSourceRect.Width * m_pView->GetZoomFactor();
		float screenHeight = previewSourceRect.Height * m_pView->GetZoomFactor();
		infoSS << L"Object Preview: " << (int)screenWidth << L"x" << (int)screenHeight
			<< L"px (World: " << (int)previewSourceRect.Width << L"x" << (int)previewSourceRect.Height << L"px)";
	}

	std::wstring infoText = infoSS.str();
	Gdiplus::RectF infoRect(finalRenderX, finalRenderY - 30, 260, 50);

	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	if (infoRect.Y < 0) {
		infoRect.Y = finalRenderY + finalRenderHeight + 5;
	}
	if (infoRect.X + infoRect.Width > clientRect.right) {
		infoRect.X = clientRect.right - infoRect.Width - 10;
	}

	pGraphics->FillRectangle(&infoBackBrush, infoRect);
	pGraphics->DrawString(infoText.c_str(), -1, &infoFont, infoRect, nullptr, &infoBrush);
}

void MapEditor::SetDebugInfoVisible(bool visible) {
	if (m_pDebugPanel && visible != m_pDebugPanel->IsVisible()) {
		m_pDebugPanel->ToggleVisibility();
	}
}

bool MapEditor::IsDebugInfoVisible() const {
	return m_pDebugPanel ? m_pDebugPanel->IsVisible() : false;
}

void MapEditor::AddObject(const ResourcePathUtils::ObjectResourceDef& obj) {
	m_gameObjects.push_back(obj);
	m_objectsDirty = true;
	m_pLayerComposer->SetObjectLayerDirty(true);
}

void MapEditor::RemoveObject(size_t idx) {
	if (idx < m_gameObjects.size()) {
		m_gameObjects.erase(m_gameObjects.begin() + idx);
		m_objectsDirty = true;
		m_pLayerComposer->SetObjectLayerDirty(true);
	}
}

void MapEditor::RemoveObject(ResourcePathUtils::ObjectResourceDef* objToRemove) {
	auto it = std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
		[objToRemove](const ResourcePathUtils::ObjectResourceDef& obj) { return &obj == objToRemove; });
	if (it != m_gameObjects.end()) {
		m_gameObjects.erase(it, m_gameObjects.end());
		m_objectsDirty = true;
		m_pLayerComposer->SetObjectLayerDirty(true);
	}
}

void MapEditor::UpdateObjectPosition(ResourcePathUtils::ObjectResourceDef* obj, int newX, int newY) {
	if (obj) {
		obj->x = (float)newX;
		obj->y = (float)newY;
		m_objectsDirty = true;
		m_pLayerComposer->SetObjectLayerDirty(true);
	}
}

float MapEditor::GetLayerMemoryUsageMB() const {
	Gdiplus::Bitmap* tileLayerBitmap = m_pLayerComposer->GetTileLayerBitmap();
	if (!tileLayerBitmap) return 0.0f;

	UINT totalPixels = tileLayerBitmap->GetWidth() * tileLayerBitmap->GetHeight();
	return (totalPixels * 4) / (1024.0f * 1024.0f);
}

void MapEditor::DrawPlayerSpawn(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;

	if (m_isPlayerSpawnMode) {
		Gdiplus::Font font(L"Arial", 14, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 0));
		Gdiplus::SolidBrush backgroundBrush(Gdiplus::Color(150, 0, 0, 0));

		std::wstring modeText = L"[PLAYER SPAWN MODE] Click to set spawn point (P to exit)";
		Gdiplus::RectF textRect(10, 40, 600, 30);

		pGraphics->FillRectangle(&backgroundBrush, textRect);
		pGraphics->DrawString(modeText.c_str(), -1, &font, textRect, nullptr, &textBrush);
	}

	if (m_hasPlayerSpawn) {
		Gdiplus::PointF screenPos = WorldToScreen(m_playerSpawnPoint);
		float iconRadius = 16.0f;

		int fillAlpha = m_isPlayerSpawnMode ? 200 : 100;
		int textAlpha = m_isPlayerSpawnMode ? 255 : 150;
		float penWidth = m_isPlayerSpawnMode ? 3.0f : 1.5f;

		Gdiplus::SolidBrush spawnBrush(Gdiplus::Color(fillAlpha, 0, 255, 0));
		Gdiplus::Pen spawnPen(Gdiplus::Color(textAlpha, 255, 255, 255), penWidth);

		pGraphics->FillEllipse(&spawnBrush, screenPos.X - iconRadius, screenPos.Y - iconRadius, iconRadius * 2.0f, iconRadius * 2.0f);
		pGraphics->DrawEllipse(&spawnPen, screenPos.X - iconRadius, screenPos.Y - iconRadius, iconRadius * 2.0f, iconRadius * 2.0f);

		Gdiplus::Font playerFont(L"Arial", 12, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush playerTextBrush(Gdiplus::Color(textAlpha, 255, 255, 255));
		Gdiplus::RectF playerTextRect(screenPos.X - 8, screenPos.Y - 8, 16, 16);
		Gdiplus::StringFormat centerFormat;
		centerFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
		centerFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		pGraphics->DrawString(L"P", 1, &playerFont, playerTextRect, &centerFormat, &playerTextBrush);

		if (m_isPlayerSpawnMode) {
			Gdiplus::Font coordFont(L"Arial", 10);
			Gdiplus::SolidBrush coordBrush(Gdiplus::Color(255, 255, 255, 255));
			std::wstringstream coordSS;
			coordSS << L"(" << (int)m_playerSpawnPoint.X << L", " << (int)m_playerSpawnPoint.Y << L")";
			Gdiplus::RectF coordRect(screenPos.X - 40, screenPos.Y + iconRadius + 5, 80, 15);
			pGraphics->DrawString(coordSS.str().c_str(), -1, &coordFont, coordRect, &centerFormat, &coordBrush);
		}
	}
}

Gdiplus::RectF MapEditor::GetViewWorldRect(float cullingMargin) const {
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	return m_pView->GetViewWorldRect(clientRect.right, clientRect.bottom, cullingMargin);
}

Gdiplus::PointF MapEditor::WorldToScreen(Gdiplus::PointF worldPos) const {
	return m_pView->WorldToScreen(worldPos);
}

Gdiplus::RectF MapEditor::WorldToScreen(Gdiplus::RectF worldRect) const {
	return m_pView->WorldToScreen(worldRect);
}

Gdiplus::PointF MapEditor::ScreenToWorld(Gdiplus::PointF screenPos) const {
	return m_pView->ScreenToWorld(screenPos);
}

bool MapEditor::IsPointInDebugPanel(POINT clickPoint) const {
	if (!m_pDebugPanel->IsVisible()) return false;
	
	Gdiplus::RectF r = m_pDebugPanel->GetViewportRect();
	return (r.Width > 0 && r.Height > 0 &&
		(float)clickPoint.x >= r.X && (float)clickPoint.x < r.X + r.Width &&
		(float)clickPoint.y >= r.Y && (float)clickPoint.y < r.Y + r.Height);
}

void MapEditor::HandlePlacingModeClick(POINT clickPoint, HWND hWnd) {
	int selIdx = m_pPalette->GetSelectedPaletteIndex();
	const PaletteItem* pSelectedItem = (selIdx >= 0) ? m_pPalette->GetPaletteItem((size_t)selIdx) : nullptr;
	if (!pSelectedItem) return;

	const PaletteItem& selectedItem = *pSelectedItem;
	Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

	if (selectedItem.category == CATEGORY_TILE) {
		const ResourcePathUtils::TileResourceDef* tv = m_pPalette->GetSelectedTileVariant();
		if (!tv) return;

		int mapX = (int)floor(mouseWorldPos.X / TILE_SIZE);
		int mapY = (int)floor(mouseWorldPos.Y / TILE_SIZE);
		mapX = max(0, min(m_mapWidth - 1, mapX));
		mapY = max(0, min(m_mapHeight - 1, mapY));

		if (m_is3x3Mode) {
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int targetX = mapX + dx;
					int targetY = mapY + dy;
					if (targetX >= 0 && targetX < m_mapWidth && targetY >= 0 && targetY < m_mapHeight) {
						m_tileMap[targetY][targetX] = ResourcePathUtils::TileResourceDef(tv->type, tv->id, tv->baseDir, tv->imageName);
					}
				}
			}
		}
		else {
			m_tileMap[mapY][mapX] = ResourcePathUtils::TileResourceDef(tv->type, tv->id, tv->baseDir, tv->imageName);
		}
		m_pLayerComposer->SetTileLayerDirty(true);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
	} else if (selectedItem.category == CATEGORY_OBJECT) {
		const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
		if (!ov) return;

		GameObjectID selectedObjectID = m_pPalette->GetSelectedGameObjectID();

		float pivotX = ov->pivotX;
		float pivotY = ov->pivotY;
		bool hasCollider = ov->hasCollider;
		ColliderType colliderType = ov->colliderType;
		int colliderOffsetX = ov->colliderOffsetX, colliderOffsetY = ov->colliderOffsetY;
		int colliderWidth = ov->colliderWidth, colliderHeight = ov->colliderHeight;
		float colliderCenterX = ov->colliderCenterX, colliderCenterY = ov->colliderCenterY, colliderRadius = ov->colliderRadius;

		bool needFallback = !hasCollider || (colliderType == COLLIDER_BOX && colliderWidth <= 0 && colliderHeight <= 0) || (colliderType == COLLIDER_CIRCLE && colliderRadius <= 0.0f);
		if (needFallback) {
			const ResourcePathUtils::ObjectResourceDef* sameTypeTemplate = nullptr;
			for (const ResourcePathUtils::ObjectResourceDef& obj : m_gameObjects) {
				if (obj.id == selectedObjectID) {
					sameTypeTemplate = &obj;
					break;
				}
			}
			if (sameTypeTemplate) {
				pivotX = sameTypeTemplate->pivotX;
				pivotY = sameTypeTemplate->pivotY;
				hasCollider = sameTypeTemplate->hasCollider;
				colliderType = sameTypeTemplate->colliderType;
				colliderOffsetX = sameTypeTemplate->colliderOffsetX;
				colliderOffsetY = sameTypeTemplate->colliderOffsetY;
				colliderWidth = sameTypeTemplate->colliderWidth;
				colliderHeight = sameTypeTemplate->colliderHeight;
				colliderCenterX = sameTypeTemplate->colliderCenterX;
				colliderCenterY = sameTypeTemplate->colliderCenterY;
				colliderRadius = sameTypeTemplate->colliderRadius;
			} else {
				int imageWidth = 32, imageHeight = 32;
				if (!ov->imageName.empty()) {
					std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
					std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
					if (pBitmap) {
						imageWidth = pBitmap->GetWidth();
						imageHeight = pBitmap->GetHeight();
					}
				}
				colliderOffsetX = -(int)(ov->pivotX * imageWidth);
				colliderOffsetY = -(int)(ov->pivotY * imageHeight);
				colliderWidth = imageWidth;
				colliderHeight = imageHeight;
				colliderCenterX = imageWidth * (0.5f - ov->pivotX);
				colliderCenterY = imageHeight * (0.5f - ov->pivotY);
				float smallerSize = (imageWidth < imageHeight) ? (float)imageWidth : (float)imageHeight;
				colliderRadius = smallerSize * 0.5f;
			}
		}

		ResourcePathUtils::ObjectResourceDef newObject(selectedObjectID,
			mouseWorldPos.X, mouseWorldPos.Y,
			ov->baseDir, ov->imageName, pivotX, pivotY,
			hasCollider, colliderType,
			colliderOffsetX, colliderOffsetY, colliderWidth, colliderHeight,
			colliderCenterX, colliderCenterY, colliderRadius);
		AddObject(newObject);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd);
	}
}

void MapEditor::HandleObjectSelectionClick(POINT clickPoint, HWND hWnd) {
	Gdiplus::PointF mouseWorldClickPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

	for (int i = (int)m_gameObjects.size() - 1; i >= 0; --i) {
		ResourcePathUtils::ObjectResourceDef& obj = m_gameObjects[i];
		const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(obj.id);
		if (!ov || ov->imageName.empty()) continue;

		std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
		std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
		if (!pBitmap) continue;

		float objWidthWorld = (float)pBitmap->GetWidth();
		float objHeightWorld = (float)pBitmap->GetHeight();
		float objRenderLeftWorld = obj.x - (ov->pivotX * objWidthWorld);
		float objRenderTopWorld = obj.y - (ov->pivotY * objHeightWorld);

		Gdiplus::RectF objWorldRect(objRenderLeftWorld, objRenderTopWorld, objWidthWorld, objHeightWorld);

		if (objWorldRect.Contains(mouseWorldClickPos.X, mouseWorldClickPos.Y)) {
			m_selectedObjectPtr = &obj;
			m_pLayerComposer->SetObjectLayerDirty(true);
			InvalidateRect(hWnd, NULL, FALSE);
			return;
		}
	}

	DeselectObject(hWnd);
}

void MapEditor::DeselectObject(HWND hWnd) {
	if (m_selectedObjectPtr != nullptr) {
		m_selectedObjectPtr = nullptr;
		m_pLayerComposer->SetObjectLayerDirty(true);
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void MapEditor::HandleErasingModeClick(POINT clickPoint, HWND hWnd) {
	Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

	int mapX = (int)floor(mouseWorldPos.X / TILE_SIZE);
	int mapY = (int)floor(mouseWorldPos.Y / TILE_SIZE);
	mapX = max(0, min(m_mapWidth - 1, mapX));
	mapY = max(0, min(m_mapHeight - 1, mapY));

	if (m_is3x3Mode) {
		for (int dy = -1; dy <= 1; ++dy) {
			for (int dx = -1; dx <= 1; ++dx) {
				int targetX = mapX + dx;
				int targetY = mapY + dy;
				if (targetX >= 0 && targetX < m_mapWidth && targetY >= 0 && targetY < m_mapHeight) {
					m_tileMap[targetY][targetX] = ResourcePathUtils::TileResourceDef();
				}
			}
		}
	}
	else {
		m_tileMap[mapY][mapX] = ResourcePathUtils::TileResourceDef();
	}
	m_pLayerComposer->SetTileLayerDirty(true);
	InvalidateRect(hWnd, NULL, FALSE);
	UpdateWindow(hWnd);
}

void MapEditor::ExitAllEditModes() {
	m_pWalkableEditor->EndWalkableEdit();
	m_isPlayerSpawnMode = false;
	m_isPlacingMode = false;
	m_isErasingMode = false;
	m_is3x3Mode = false;
	if (m_pPalette->IsSubPaletteOpen()) {
		m_pPalette->CloseSubPalette();
		m_paletteLayerDirty = true;
	}
}

void MapEditor::ShowMapSizeDialog(HWND parent) {
	EditorMapFileIO::ShowMapSizeDialog(this, parent);
}

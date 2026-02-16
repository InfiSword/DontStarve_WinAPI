#include "../pch.h" 
#include "DontStarve_EditorMain.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../03_EditorMapFileIO/EditorMapFileIO.h"
#include "../04_EditorPalette/EditorPalette.h"
#include "../05_EditorPivotEditor/EditorPivotEditor.h"
#include "../06_EditorColliderEditor/EditorColliderEditor.h"
#include "../07_EditorWalkableEditor/EditorWalkableEditor.h"
#include "../08_EditorDebugPanel/EditorDebugPanel.h"
#include "../../Header/Function.h"
#include <memory>
#include "../09_EditorLayerComposer/EditorLayerComposer.h"
#include <commdlg.h>  // 파일 다이얼로그용

DontStarve_EditorMain::DontStarve_EditorMain()
	: m_pGraphics(nullptr), m_pDoubleBufferBitmap(nullptr),
	m_isPlacingMode(false), m_is3x3Mode(false),
	m_rawMousePos({ 0,0 }), m_snappedPreviewPos(0.0f, 0.0f),
	m_pView(std::make_unique<EditorView>()),
	m_pResources(std::make_unique<EditorResourceManager>()),
	m_pPalette(std::make_unique<EditorPalette>()),
	m_pPivotEditor(std::make_unique<EditorPivotEditor>()),
	m_pColliderEditor(std::make_unique<EditorColliderEditor>()),
	m_pWalkableEditor(std::make_unique<EditorWalkableEditor>()),
	m_pDebugPanel(std::make_unique<EditorDebugPanel>()),
	m_pLayerComposer(std::make_unique<EditorLayerComposer>()),
	m_objectsDirty(true),
	m_selectedObjectPtr(nullptr),
	m_paletteLayerBitmap(nullptr), m_paletteLayerDirty(true),
	m_hasPlayerSpawn(false), m_playerSpawnPoint(0.0f, 0.0f), m_isPlayerSpawnMode(false)
{
	// m_tileMap 초기화 (ResourcePathUtils::TileResourceDef의 기본 생성자 호출)
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
		}
	}

	// walkable area map 초기화 (기본적으로 모든 영역이 walkable)
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_walkableAreaMap[y][x] = true;
		}
	}
}


DontStarve_EditorMain::~DontStarve_EditorMain()
{
	Release();
}


void DontStarve_EditorMain::Initialize() {
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	// Double Buffer Bitmap 및 Graphics 객체 설정
	m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
	m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);

	// GDI+ 렌더링 설정 (Client와 동일: 성능 우선 - NearestNeighbor, HighSpeed)
	m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
	m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);

	// 초기 레이어 비트맵 생성 (화면 크기 기반)
	const UINT INIT_LAYER_SIZE = 1024;  // 초기 1024x1024 크기

	m_pResources->LoadResources();
	m_pPivotEditor->SetDependencies(m_pView.get(), m_pResources.get());
	m_pColliderEditor->SetDependencies(m_pView.get(), m_pResources.get(), this);
	m_pLayerComposer->SetDependencies(m_pView.get(), m_pResources.get(), this);
	m_pLayerComposer->ResizeLayerBitmaps(INIT_LAYER_SIZE, INIT_LAYER_SIZE);
	InitPalette();

	// 플레이어 스폰 포인트를 맵 중앙으로 초기화
	float centerX = (MAP_WIDTH / 2.0f) * TILE_SIZE;  // 25 * 128 = 3200px
	float centerY = (MAP_HEIGHT / 2.0f) * TILE_SIZE; // 25 * 128 = 3200px
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;

	std::wstringstream debugSS;
	debugSS << L"Initial Player Spawn set to map center: (" << (int)centerX << L", " << (int)centerY
		<< L")px = Tile(" << (MAP_WIDTH / 2) << L", " << (MAP_HEIGHT / 2) << L")\n";
	OutputDebugStringW(debugSS.str().c_str());

	m_pLayerComposer->ComposeGridLayer();
	m_pLayerComposer->ComposeTileLayer();
	m_pLayerComposer->ComposeObjectLayer();
	m_pPalette->ComposePaletteLayer(m_paletteLayerBitmap, &m_paletteLayerDirty);
}

void DontStarve_EditorMain::Update()
{

}

void DontStarve_EditorMain::Render()
{
	if (!m_pGraphics) return;

	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	m_pGraphics->Clear(Gdiplus::Color(255, 255, 255, 255));

	// 간단한 적응형 레이어 크기 (성능 vs 메모리 균형)


	UINT targetWidth = max(512U, min((UINT)clientRect.right + 256, 1536U));   // 최대 1536px
	UINT targetHeight = max(512U, min((UINT)clientRect.bottom + 256, 1536U)); // 최대 1536px

	// 레이어 비트맵 재생성 (크기가 크게 다를 때만)
	Gdiplus::Bitmap* tileLayerBitmap = m_pLayerComposer->GetTileLayerBitmap();
	if (!tileLayerBitmap ||
		abs((int)tileLayerBitmap->GetWidth() - (int)targetWidth) > 128 ||
		abs((int)tileLayerBitmap->GetHeight() - (int)targetHeight) > 128) {

		m_pLayerComposer->ResizeLayerBitmaps(targetWidth, targetHeight);
	}

	// 스마트 레이어 렌더링 (필요할 때만)
	m_pLayerComposer->ComposeGridLayer();
	m_pLayerComposer->ComposeTileLayer();
	m_pLayerComposer->ComposeObjectLayer();

	m_pPalette->ComposePaletteLayer(m_paletteLayerBitmap, &m_paletteLayerDirty); // 팔레트는 항상 동일

	// 뷰포트 기반 레이어 렌더링 (성능 최적화)
	m_pLayerComposer->DrawLayers(m_pGraphics);

	// 팔레트는 항상 동일하게 렌더링
	if (m_paletteLayerBitmap) {
		const RECT& pr = m_pPalette->GetPaletteRect();
		m_pGraphics->DrawImage(m_paletteLayerBitmap,
			(Gdiplus::REAL)pr.left, (Gdiplus::REAL)pr.top,
			(Gdiplus::REAL)(pr.right - pr.left),
			(Gdiplus::REAL)(pr.bottom - pr.top));
	}

	DrawPreview(m_pGraphics);
	m_pPalette->DrawSubPalette(m_pGraphics);
		m_pPivotEditor->DrawPivotEditor(m_pGraphics);
		m_pColliderEditor->DrawColliders(m_pGraphics);
	DrawPlayerSpawn(m_pGraphics);
		m_pWalkableEditor->DrawWalkableAreas(m_pGraphics);

	// 디버그 정보 (F1 키로 토글 가능)
	if (m_pDebugPanel->IsVisible()) {
		m_pDebugPanel->DrawDebugInfo(m_pGraphics);
	}

	// Client와 동일: GDI+ DrawImage 대신 BitBlt로 최종 출력 (성능 최적화)
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

void DontStarve_EditorMain::Release()
{
	m_pResources->ReleaseResources(); // 리소스 (아틀라스 비트맵들) 해제

	Utils::SafeDelete(m_pGraphics);
	Utils::SafeDelete(m_pDoubleBufferBitmap);
	Utils::SafeDelete(m_paletteLayerBitmap);
}

LRESULT DontStarve_EditorMain::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) {
	switch (message) {
	case WM_CREATE: {
		// 초기 윈도우 생성 시 처리할 로직 (현재는 Initialize에서 대부분 처리)
		return 0;
	}

	case WM_PAINT: {
		// Render() 함수가 이미 직접 그리기 작업을 수행하고 마지막에 BitBlt을 하므로,
		// WM_PAINT는 InvalidateRect 호출 시에만 발생하고, 여기서는 그릴 내용이 없을 수 있습니다.
		// 다만, 혹시 모를 경우를 대비하여 BeginPaint/EndPaint는 유지합니다.
		// 그러나 핵심 렌더링은 Render() 함수에서 담당합니다.
		PAINTSTRUCT ps;
		HDC hdc = BeginPaint(hWnd, &ps);
		// Gdiplus::Graphics screenGraphics(hdc);
		// screenGraphics.DrawImage(m_pDoubleBufferBitmap, 0, 0, clientRect.right, clientRect.bottom);
		EndPaint(hWnd, &ps);
		return 0;
	}

	case WM_SIZE: {
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);

		// Double buffer 재생성
		Utils::SafeDelete(m_pDoubleBufferBitmap);
		Utils::SafeDelete(m_pGraphics);
		m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
		m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
		m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);

		// 팔레트 재초기화
		m_pPalette->InitPalette(clientRect.right, clientRect.bottom, m_pResources.get());
		Utils::SafeDelete(m_paletteLayerBitmap);
		const RECT& pr = m_pPalette->GetPaletteRect();
		m_paletteLayerBitmap = new Gdiplus::Bitmap(pr.right - pr.left, pr.bottom - pr.top, PixelFormat32bppARGB);
		m_paletteLayerDirty = true;

		// 레이어 재그리기 필요
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

		// 카메라 드래그 모드 (우클릭 드래그) - 단순화된 로직
		if (m_isDraggingCamera) {
			// 시작점 기준으로 직접 계산 (단순하고 직관적)
			int deltaX = m_rawMousePos.x - m_cameraDragStart.x;
			int deltaY = m_rawMousePos.y - m_cameraDragStart.y;

			// 맵 오프셋 업데이트 (드래그 방향과 반대로 이동)
			m_pView->SetMapOffset(m_initialMapOffset.x + deltaX, m_initialMapOffset.y + deltaY);
		
			// 뷰포트 렌더링에서는 맵 오프셋 변경 시 전체 재그리기 필요
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

		if (m_pColliderEditor->IsDraggingCollider()) {
			m_pColliderEditor->OnMouseMove(m_rawMousePos, hWnd);
			m_pLayerComposer->SetObjectLayerDirty(true);
			return 0;
		}

		if (m_pPivotEditor->IsPivotEditMode() && (GetKeyState(VK_LBUTTON) & 0x8000)) {
			m_pPivotEditor->UpdatePivotEdit(m_rawMousePos);
			m_pLayerComposer->SetObjectLayerDirty(true);
			return 0;
		}

		// 배치 모드일 때 프리뷰 위치 계산 (월드 좌표)
		int selIdx = m_pPalette->GetSelectedPaletteIndex();
		if (selIdx != -1 && m_isPlacingMode) {
			const PaletteItem* pItem = m_pPalette->GetPaletteItem((size_t)selIdx);
			if (!pItem) { /* skip */ }
			else {
			const PaletteItem& selectedItem = *pItem;

			Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)m_rawMousePos.x, (float)m_rawMousePos.y));

			if (selectedItem.category == CATEGORY_TILE) {
				// 타일: 그리드에 스냅된 월드 좌표 (좌상단)
				int snappedMapX = (int)floor(mouseWorldPos.X / TILE_SIZE);
				int snappedMapY = (int)floor(mouseWorldPos.Y / TILE_SIZE);

				// 맵 경계 체크
				snappedMapX = max(0, min(MAP_WIDTH - 1, snappedMapX));
				snappedMapY = max(0, min(MAP_HEIGHT - 1, snappedMapY));

				m_snappedPreviewPos.X = (float)(snappedMapX * TILE_SIZE);
				m_snappedPreviewPos.Y = (float)(snappedMapY * TILE_SIZE);
			}
			else if (selectedItem.category == CATEGORY_OBJECT) {
				// 오브젝트: 마우스 위치 그대로 (발 밑 중심)
				m_snappedPreviewPos.X = mouseWorldPos.X;
				m_snappedPreviewPos.Y = mouseWorldPos.Y;
			}
			}
		}
	}
	break;

	case WM_LBUTTONDOWN: {
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		POINT clickPoint = { mouseX, mouseY };

		if (m_pWalkableEditor->IsWalkableEditMode()) {
			m_pWalkableEditor->OnLeftButtonDown(clickPoint, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 2. Player Spawn Mode
		if (m_isPlayerSpawnMode) {
			Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

			// Define.h 값들을 사용한 맵 경계 체크
			const float TOTAL_MAP_WIDTH = (float)(MAP_WIDTH * TILE_SIZE);   // 50 * 128 = 6400px
			const float TOTAL_MAP_HEIGHT = (float)(MAP_HEIGHT * TILE_SIZE); // 50 * 128 = 6400px

			if (mouseWorldPos.X >= 0 && mouseWorldPos.X <= TOTAL_MAP_WIDTH &&
				mouseWorldPos.Y >= 0 && mouseWorldPos.Y <= TOTAL_MAP_HEIGHT) {

				m_playerSpawnPoint = mouseWorldPos;
				m_hasPlayerSpawn = true;

				// 타일 좌표로도 표시
				int tileX = (int)(mouseWorldPos.X / TILE_SIZE);
				int tileY = (int)(mouseWorldPos.Y / TILE_SIZE);

				std::wstringstream debugSS;
				debugSS << L"Player Spawn Set: (" << (int)m_playerSpawnPoint.X << L", " << (int)m_playerSpawnPoint.Y
					<< L")px = Tile(" << tileX << L", " << tileY << L")\n";
				OutputDebugStringW(debugSS.str().c_str());

				InvalidateRect(hWnd, NULL, FALSE);
			}
			else {
				std::wstringstream debugSS;
				debugSS << L"Player spawn point must be within map boundaries! "
					<< L"Valid range: (0,0) to (" << (int)TOTAL_MAP_WIDTH << L"," << (int)TOTAL_MAP_HEIGHT << L")\n";
				OutputDebugStringW(debugSS.str().c_str());
			}
			return 0;
		}

		if (m_pColliderEditor->IsColliderEditMode()) {
			m_pColliderEditor->OnLeftButtonDown(clickPoint, hWnd);
			if (m_pColliderEditor->IsDraggingCollider()) {
				m_pLayerComposer->SetObjectLayerDirty(true);
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		if (m_pPivotEditor->IsPivotEditMode()) {
			m_pPivotEditor->UpdatePivotEdit(clickPoint);
			m_pLayerComposer->SetObjectLayerDirty(true);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 3. Sub-Palette Click Handling
		auto subResult = m_pPalette->HandleSubPaletteClick(clickPoint);
		if (subResult != EditorPalette::SubPaletteClickResult::NotHandled) {
			if (subResult == EditorPalette::SubPaletteClickResult::ClosedWithSelection) {
				m_isPlacingMode = true;
			}
			else {
				m_isPlacingMode = false;
				m_pColliderEditor->EndColliderEdit();
				m_selectedObjectPtr = nullptr;
			}
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 4. Main Palette Click Handling
		if (m_pPalette->HandleMainPaletteClick(clickPoint, clientRect.bottom)) {
			m_isPlacingMode = false;
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 5. Map Click Handling (Placing Mode or Object Selection)
		if (m_isPlacingMode) {
			HandlePlacingModeClick(clickPoint, hWnd);
		}
		else {
			HandleObjectSelectionClick(clickPoint, hWnd);
		}
	}
	break;

	case WM_RBUTTONDOWN:
	{
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		POINT clickPoint = { mouseX, mouseY };

		// 1. 하위 팔레트가 열려 있으면 닫기
		if (m_pPalette->IsSubPaletteOpen()) {
			m_pPalette->CloseSubPalette();
			m_isPlacingMode = false;
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 2. 배치 모드 중이면 배치 모드 해제
		if (m_isPlacingMode) {
			m_isPlacingMode = false;
			m_pPalette->ResetSelection();
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 3. 카메라 드래그 시작 (우클릭) - 팔레트 영역이 아닐 때만
		const RECT& pr = m_pPalette->GetPaletteRect();
		if (!PtInRect(&pr, clickPoint)) {
			m_isDraggingCamera = true;
			m_cameraDragStart = clickPoint;
			m_initialMapOffset = m_pView->GetMapOffset();
			SetCapture(hWnd); // 마우스 캡처
			
			// 디버그 정보 출력
			std::wstringstream debugSS;
			debugSS << L"Camera drag started at (" << clickPoint.x << L", " << clickPoint.y << L")\n";
			OutputDebugStringW(debugSS.str().c_str());
		}
	}
	break;

	case WM_CAPTURECHANGED:
	{
		// 마우스 캡처가 예기치 않게 해제된 경우 드래그 상태 초기화
		if (m_isDraggingCamera) {
			m_isDraggingCamera = false;
			std::wstringstream debugSS;
			debugSS << L"Camera drag cancelled due to capture loss\n";
			OutputDebugStringW(debugSS.str().c_str());
		}
		if (m_pWalkableEditor->IsDraggingWalkable()) {
			m_pWalkableEditor->OnLeftButtonUp();
		}
		if (m_pColliderEditor->IsDraggingCollider()) {
			m_pColliderEditor->OnLeftButtonUp();
		}
	}
	break;

	case WM_RBUTTONUP:
	{
		if (m_isDraggingCamera) {
			// 드래그 거리 계산 (클릭인지 드래그인지 판별)
			int deltaX = m_rawMousePos.x - m_cameraDragStart.x;
			int deltaY = m_rawMousePos.y - m_cameraDragStart.y;
			int dragDistanceSquared = deltaX * deltaX + deltaY * deltaY;

			m_isDraggingCamera = false;
			ReleaseCapture();

			// 디버그 정보 출력
			std::wstringstream debugSS;
			debugSS << L"Camera drag ended - distance: " << sqrt(dragDistanceSquared) << L" pixels\n";
			OutputDebugStringW(debugSS.str().c_str());

			// 드래그 거리가 5픽셀 이하면 클릭으로 간주하여 오브젝트 선택 처리 (5^2 = 25)
			if (dragDistanceSquared <= 25) {
				POINT clickPoint = { m_rawMousePos.x, m_rawMousePos.y };
				HandleObjectSelectionClick(clickPoint, hWnd);
			}
			return 0;
		}
	}
	break;

	// ----------------------------------------------------------------------------------------------------
	// Mouse Wheel (Zoom)
	// ----------------------------------------------------------------------------------------------------
	case WM_MOUSEWHEEL:
	{
		short zDelta = GET_WHEEL_DELTA_WPARAM(wParam);
		POINT mouseScreenPos = { GET_X_LPARAM(lParam), GET_Y_LPARAM(lParam) };
		POINT mouseClientPos = mouseScreenPos;
		ScreenToClient(hWnd, &mouseClientPos);

		// 디버그 info 뷰포트 위에 마우스가 있으면 스크롤 (줌 대신)
		if (m_pDebugPanel->IsVisible()) {
			int mx = mouseClientPos.x;
			int my = mouseClientPos.y;
			m_pDebugPanel->HandleMouseWheel(zDelta, mx, my);
			if (m_pDebugPanel->GetViewportRect().Width > 0 && m_pDebugPanel->GetViewportRect().Height > 0) {
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		Gdiplus::PointF mouseWorldPos_before_zoom = ScreenToWorld(Gdiplus::PointF((float)mouseScreenPos.x, (float)mouseScreenPos.y));

		float oldZoomFactor = m_pView->GetZoomFactor();

		if (zDelta > 0) { // Zoom In
			m_pView->ZoomIn();
		}
		else { // Zoom Out
			m_pView->ZoomOut();
		}

		float newZoomFactor = m_pView->GetZoomFactor();

		// 줌 팩터가 실제로 변경되었을 때만 처리
		if (newZoomFactor != oldZoomFactor) {
			// 변경된 줌 팩터로 마우스 위치에 해당하는 새로운 화면 좌표를 계산
			Gdiplus::PointF mouseScreenPos_after_zoom = WorldToScreen(mouseWorldPos_before_zoom);

			// 새로운 맵 오프셋 계산 (마우스가 클릭한 월드 지점이 화면상에서 같은 위치에 유지되도록)
			POINT mo = m_pView->GetMapOffset();
			m_pView->SetMapOffset(mo.x + (LONG)(mouseScreenPos.x - mouseScreenPos_after_zoom.X), mo.y + (LONG)(mouseScreenPos.y - mouseScreenPos_after_zoom.Y));

			// 줌 변경 시 모든 레이어 재그리기 (화면에서 크기가 변하므로)
			m_pLayerComposer->SetGridLayerDirty(true);
			m_pLayerComposer->SetTileLayerDirty(true);
			m_pLayerComposer->SetObjectLayerDirty(true);

			InvalidateRect(hWnd, NULL, FALSE);
		}
		return 0;
	}

	// ----------------------------------------------------------------------------------------------------
	// Keyboard Input (WASD for scrolling, ESC, Shift)
	// ----------------------------------------------------------------------------------------------------
	case WM_KEYDOWN:
	{
		if (wParam == VK_ESCAPE) {
			// 우선순위: 서브팔레트 > 콜라이더 편집 > 피벗 편집 > 플레이어 스폰 > 워커블 편집 > 오브젝트 선택 해제
			if (m_pPalette->IsSubPaletteOpen()) {
				m_pPalette->CloseSubPalette();
				m_isPlacingMode = false;
				m_paletteLayerDirty = true;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			if (m_pColliderEditor->IsColliderEditMode()) {
				m_pColliderEditor->EndColliderEdit();
				m_pLayerComposer->SetObjectLayerDirty(true);
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			if (m_pPivotEditor->IsPivotEditMode()) {
				m_pPivotEditor->EndPivotEdit();
				m_pLayerComposer->SetObjectLayerDirty(true);
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			if (m_isPlayerSpawnMode) {
				m_isPlayerSpawnMode = false;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			if (m_pWalkableEditor->IsWalkableEditMode()) {
				m_pWalkableEditor->EndWalkableEdit();
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			DeselectObject(hWnd);
			return 0;
		}

		// Shift key for 3x3 mode toggle (only for tiles in placing mode)
		int sp = m_pPalette->GetSelectedPaletteIndex();
		const PaletteItem* pi = (sp >= 0) ? m_pPalette->GetPaletteItem((size_t)sp) : nullptr;
		if (wParam == VK_SHIFT && sp != -1 && m_isPlacingMode && pi && pi->category == CATEGORY_TILE) {
			m_is3x3Mode = !m_is3x3Mode;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// P key for Player Spawn mode toggle
		if (wParam == 'P') {
			m_isPlayerSpawnMode = !m_isPlayerSpawnMode;
			if (m_isPlayerSpawnMode) {
				ExitAllEditModes();
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// F1 key for Debug Info toggle
		if (wParam == VK_F1) {
			m_pDebugPanel->ToggleVisibility();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// V key for Pivot Edit mode toggle
		if (wParam == 'V') {
			if (!m_selectedObjectPtr) {
				MessageBox(hWnd, L"오브젝트를 먼저 선택해주세요.", L"피벗 편집", MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			if (!m_pPivotEditor->IsPivotEditMode()) {
				ExitAllEditModes();
				m_pPivotEditor->StartPivotEdit(m_selectedObjectPtr);
			}
			else {
				m_pPivotEditor->EndPivotEdit();
				m_pLayerComposer->SetObjectLayerDirty(true);
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// C key for Collider Edit mode toggle
		if (wParam == 'C') {
			if (!m_selectedObjectPtr) {
				MessageBox(hWnd, L"오브젝트를 먼저 선택해주세요.", L"콜라이더 편집", MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			if (!m_pColliderEditor->IsColliderEditMode()) {
				ExitAllEditModes();
				m_pColliderEditor->StartColliderEdit(m_selectedObjectPtr);
				m_pLayerComposer->SetObjectLayerDirty(true);
			}
			else {
				m_pColliderEditor->EndColliderEdit();
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// B key for Collider Type toggle (Box <-> Circle)
		if (wParam == 'B' && m_pColliderEditor->IsColliderEditMode()) {
			m_pColliderEditor->ToggleColliderType();
			m_pLayerComposer->SetObjectLayerDirty(true);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// A key: Apply collider to all objects with same type+id (selected object or collider-edit source)
		if (wParam == 'A') {
			if (m_pColliderEditor->IsColliderEditMode()) {
				m_pColliderEditor->ApplyColliderToSameType(nullptr);
				m_pLayerComposer->SetObjectLayerDirty(true);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			else if (m_selectedObjectPtr) {
				m_pColliderEditor->ApplyColliderToSameType(m_selectedObjectPtr);
				m_pLayerComposer->SetObjectLayerDirty(true);
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;
		}

		// R key for Delete selected object
		if (wParam == 'R') {
			if (!m_selectedObjectPtr) {
				MessageBox(hWnd, L"삭제할 오브젝트를 먼저 선택해주세요.", L"오브젝트 삭제", MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			if (MessageBox(hWnd, L"선택된 오브젝트를 삭제하시겠습니까?", L"오브젝트 삭제", MB_YESNO | MB_ICONQUESTION) == IDYES) {
				if (m_pPivotEditor->GetEditingObject() == m_selectedObjectPtr) {
					m_pPivotEditor->EndPivotEdit();
				}
				m_pColliderEditor->EndColliderEdit();
				RemoveObject(m_selectedObjectPtr);
				m_selectedObjectPtr = nullptr;
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;
		}

		if (wParam == 'G') {
			bool wasActive = m_pWalkableEditor->IsWalkableEditMode();
			m_pWalkableEditor->ToggleWalkableEditMode();
			if (m_pWalkableEditor->IsWalkableEditMode() && !wasActive) {
				ExitAllEditModes();
			}
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}


		return 0;
	}

	// ----------------------------------------------------------------------------------------------------
	// 
	// Other Unhandled Messages
	// ----------------------------------------------------------------------------------------------------
	case WM_LBUTTONUP:
	{
		if (m_pWalkableEditor->IsDraggingWalkable()) {
			m_pWalkableEditor->OnLeftButtonUp();
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (m_pColliderEditor->IsDraggingCollider()) {
			m_pColliderEditor->OnLeftButtonUp();
			ReleaseCapture();
			m_pLayerComposer->SetObjectLayerDirty(true);
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
}



void DontStarve_EditorMain::NewMap() {
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
		}
	}
	m_gameObjects.clear();
	m_pColliderEditor->EndColliderEdit();
	ReleaseCapture();
	m_selectedObjectPtr = nullptr;
	m_isPlacingMode = false;
	m_pPivotEditor->EndPivotEdit();
	m_pColliderEditor->EndColliderEdit();
	m_pWalkableEditor->EndWalkableEdit();
	m_isPlayerSpawnMode = false;
	m_isDraggingCamera = false;
	m_pPalette->ResetSelection();
	ReleaseCapture();
	float centerX = (MAP_WIDTH / 2.0f) * TILE_SIZE;
	float centerY = (MAP_HEIGHT / 2.0f) * TILE_SIZE;
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_walkableAreaMap[y][x] = true;
		}
	}
	m_pView->SetZoomFactor(1.0f);
	m_pView->SetMapOffset(0, 0);
	m_pLayerComposer->SetTileLayerDirty(true);
	m_pLayerComposer->SetObjectLayerDirty(true);
	m_objectsDirty = true;
	m_paletteLayerDirty = true;
	m_pLayerComposer->SetGridLayerDirty(true);
}

// Map save/load functions (delegate to EditorMapFileIO)
bool DontStarve_EditorMain::SaveMap(const WCHAR* filename) {
	return EditorMapFileIO::SaveMap(this, filename);
}

bool DontStarve_EditorMain::LoadMap(const WCHAR* filename) {
	return EditorMapFileIO::LoadMap(this, filename);
}

bool DontStarve_EditorMain::ShowSaveFileDialog(WCHAR* fileName, DWORD fileNameSize) {
	return EditorMapFileIO::ShowSaveFileDialog(this, fileName, fileNameSize);
}

bool DontStarve_EditorMain::ShowOpenFileDialog(WCHAR* fileName, DWORD fileNameSize) {
	return EditorMapFileIO::ShowOpenFileDialog(this, fileName, fileNameSize);
}

// DrawGrid, DrawTileMap, DrawObjects, ComposeTileLayer, ComposeObjectLayer moved to EditorLayerComposer

// DrawPreview (배치 프리뷰 그리기 - 투명하고 그리드 크기에 맞춤)
void DontStarve_EditorMain::DrawPreview(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || m_pPalette->GetSelectedPaletteIndex() == -1 || !m_isPlacingMode) return;

	int selIdx = m_pPalette->GetSelectedPaletteIndex();
	const PaletteItem* pSelectedItem = m_pPalette->GetPaletteItem((size_t)selIdx);
	if (!pSelectedItem) return;
	const PaletteItem& selectedItem = *pSelectedItem;
	Gdiplus::Bitmap* previewBitmap = nullptr;
	Gdiplus::RectF previewSourceRect;

	if (selectedItem.category == CATEGORY_TILE) {
		const ResourcePathUtils::TileResourceDef* tv = m_pPalette->GetSelectedTileVariant();
		if (tv && !tv->imageName.empty()) {
			std::wstring fullPath = ResourcePathUtils::BuildResourcePath(tv->baseDir, tv->imageName);
			previewBitmap = BitmapUtils::LoadBitmapFromFile(fullPath.c_str());
			if (previewBitmap && previewBitmap->GetLastStatus() == Gdiplus::Ok) {
				previewSourceRect = Gdiplus::RectF(0, 0, (float)previewBitmap->GetWidth(), (float)previewBitmap->GetHeight());
			}
		}
	}
	else if (selectedItem.category == CATEGORY_OBJECT) {
		const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
		if (ov && !ov->imageName.empty()) {
			std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
			previewBitmap = BitmapUtils::LoadBitmapFromFile(fullPath.c_str());
			if (previewBitmap && previewBitmap->GetLastStatus() == Gdiplus::Ok) {
				previewSourceRect = Gdiplus::RectF(0, 0, (float)previewBitmap->GetWidth(), (float)previewBitmap->GetHeight());
			}
		}
	}

	if (previewBitmap && previewBitmap->GetLastStatus() == Gdiplus::Ok) {
		// 투명도 설정을 위한 ColorMatrix
		Gdiplus::ColorMatrix colorMatrix = {
			1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Red
			0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // Green  
			0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // Blue
			0.0f, 0.0f, 0.0f, 0.6f, 0.0f,  // Alpha (60% 투명도)
			0.0f, 0.0f, 0.0f, 0.0f, 1.0f   // Scale
		};

		Gdiplus::ImageAttributes imageAttr;
		imageAttr.SetColorMatrix(&colorMatrix);

		Gdiplus::PointF screenPreviewPos = WorldToScreen(m_snappedPreviewPos);
		float finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight;

		if (selectedItem.category == CATEGORY_TILE) {
			// 타일: 월드 좌표계에서 TILE_SIZE 크기, 화면 변환 적용
			finalRenderWidth = (float)TILE_SIZE * m_pView->GetZoomFactor();
			finalRenderHeight = (float)TILE_SIZE * m_pView->GetZoomFactor();
			finalRenderX = screenPreviewPos.X;
			finalRenderY = screenPreviewPos.Y;

			// 타일 프리뷰 배경 (그리드 영역 표시)
			Gdiplus::Pen previewGridPen(Gdiplus::Color(150, 255, 255, 0), 2.0f);
			Gdiplus::RectF previewGridRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
			pGraphics->DrawRectangle(&previewGridPen, previewGridRect);

			// 3x3 모드일 때 추가 그리드 표시
			if (m_is3x3Mode) {
				Gdiplus::Pen gridPen3x3(Gdiplus::Color(100, 255, 255, 0), 1.5f);
				float tileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();

				for (int dy = -1; dy <= 1; ++dy) {
					for (int dx = -1; dx <= 1; ++dx) {
						// 3x3 그리드의 월드 좌표 계산
						float worldGridX = m_snappedPreviewPos.X + (dx * TILE_SIZE);
						float worldGridY = m_snappedPreviewPos.Y + (dy * TILE_SIZE);
						Gdiplus::PointF screenGridPos = WorldToScreen(Gdiplus::PointF(worldGridX, worldGridY));

						Gdiplus::RectF gridRect(screenGridPos.X, screenGridPos.Y, tileSize, tileSize);
						pGraphics->DrawRectangle(&gridPen3x3, gridRect);
					}
				}
			}
		}
		else if (selectedItem.category == CATEGORY_OBJECT) {
			// 오브젝트: 원본 크기 유지
			const ResourcePathUtils::ObjectResourceDef* ov_preview = m_pPalette->GetSelectedObjectVariant();
			if (ov_preview && previewBitmap) {
				finalRenderWidth = previewSourceRect.Width * m_pView->GetZoomFactor();
				finalRenderHeight = previewSourceRect.Height * m_pView->GetZoomFactor();
				finalRenderX = screenPreviewPos.X - (ov_preview->pivotX * finalRenderWidth);
				finalRenderY = screenPreviewPos.Y - (ov_preview->pivotY * finalRenderHeight);

				// 오브젝트 프리뷰 배경 (바운딩 박스 표시)
				Gdiplus::Pen previewBBoxPen(Gdiplus::Color(150, 0, 255, 255), 1.5f);
				Gdiplus::RectF previewBBoxRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
				pGraphics->DrawRectangle(&previewBBoxPen, previewBBoxRect);

				// 피벗 포인트 표시
				float pivotScreenX = screenPreviewPos.X;
				float pivotScreenY = screenPreviewPos.Y;
				Gdiplus::SolidBrush pivotBrush(Gdiplus::Color(200, 255, 0, 0));
				Gdiplus::RectF pivotRect(pivotScreenX - 3.0f, pivotScreenY - 3.0f, 6.0f, 6.0f);
				pGraphics->FillEllipse(&pivotBrush, pivotRect);
			}
			else {
				return; // ObjectVariant가 없으면 그리지 않음
			}
		}

		// 투명 프리뷰 이미지 그리기
		if (previewBitmap && previewBitmap->GetLastStatus() == Gdiplus::Ok) {
			Gdiplus::RectF destRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
			pGraphics->DrawImage(previewBitmap, destRect,
				0, 0, previewSourceRect.Width, previewSourceRect.Height,
				Gdiplus::UnitPixel, &imageAttr);
		}

		// 프리뷰 정보 텍스트 (마우스 근처에 표시)
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

		// 화면 경계 체크 (텍스트가 화면 밖으로 나가지 않도록)
		RECT clientRect;
		GetClientRect(g_hWnd, &clientRect);
		if (infoRect.Y < 0) {
			infoRect.Y = finalRenderY + finalRenderHeight + 5; // 아래로 이동
		}
		if (infoRect.X + infoRect.Width > clientRect.right) {
			infoRect.X = clientRect.right - infoRect.Width - 10; // 왼쪽으로 이동
		}

		pGraphics->FillRectangle(&infoBackBrush, infoRect);
		pGraphics->DrawString(infoText.c_str(), -1, &infoFont, infoRect, nullptr, &infoBrush);
	}
}

// 디버그 정보 그리기
// DrawDebugInfo moved to EditorDebugPanel

void DontStarve_EditorMain::SetDebugInfoVisible(bool visible) {
	if (m_pDebugPanel && visible != m_pDebugPanel->IsVisible()) {
		m_pDebugPanel->ToggleVisibility();
	}
}

bool DontStarve_EditorMain::IsDebugInfoVisible() const {
	return m_pDebugPanel ? m_pDebugPanel->IsVisible() : false;
}

// 오브젝트 관리 함수들
void DontStarve_EditorMain::AddObject(const ResourcePathUtils::ObjectResourceDef& obj) {
	m_gameObjects.push_back(obj);
	m_objectsDirty = true;
	m_pLayerComposer->SetObjectLayerDirty(true);
}

void DontStarve_EditorMain::RemoveObject(size_t idx) {
	if (idx < m_gameObjects.size()) {
		m_gameObjects.erase(m_gameObjects.begin() + idx);
		m_objectsDirty = true;
		m_pLayerComposer->SetObjectLayerDirty(true);
	}
}

void DontStarve_EditorMain::RemoveObject(ResourcePathUtils::ObjectResourceDef* objToRemove) {
	auto it = std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
		[objToRemove](const ResourcePathUtils::ObjectResourceDef& obj) { return &obj == objToRemove; });
	if (it != m_gameObjects.end()) {
		m_gameObjects.erase(it, m_gameObjects.end());
		m_objectsDirty = true;
		m_pLayerComposer->SetObjectLayerDirty(true);
	}
}

void DontStarve_EditorMain::UpdateObjectPosition(ResourcePathUtils::ObjectResourceDef* obj, int newX, int newY) {
	if (obj) {
		obj->x = newX;
		obj->y = newY;
		m_objectsDirty = true;
		m_pLayerComposer->SetObjectLayerDirty(true);
	}
}

// 성능 모니터링 함수
float DontStarve_EditorMain::GetLayerMemoryUsageMB() const {
	Gdiplus::Bitmap* tileLayerBitmap = m_pLayerComposer->GetTileLayerBitmap();
	if (!tileLayerBitmap) return 0.0f;

	UINT totalPixels = tileLayerBitmap->GetWidth() * tileLayerBitmap->GetHeight() * 3; // 3개 레이어
	return (totalPixels * 4) / (1024.0f * 1024.0f); // 4바이트/픽셀 (ARGB)
}

// 플레이어 스폰 포인트 그리기
void DontStarve_EditorMain::DrawPlayerSpawn(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;

	// 플레이어 스폰 모드일 때 안내 텍스트 표시
	if (m_isPlayerSpawnMode) {
		Gdiplus::Font font(L"Arial", 14, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 0));
		Gdiplus::SolidBrush backgroundBrush(Gdiplus::Color(150, 0, 0, 0));

		std::wstring modeText = L"[PLAYER SPAWN MODE] Click to set spawn point (P to exit)";
		Gdiplus::RectF textRect(10, 40, 600, 30);

		pGraphics->FillRectangle(&backgroundBrush, textRect);
		pGraphics->DrawString(modeText.c_str(), -1, &font, textRect, nullptr, &textBrush);
	}

	// 플레이어 스폰 포인트가 설정되어 있으면 그리기
	if (m_hasPlayerSpawn) {
		Gdiplus::PointF screenPos = WorldToScreen(m_playerSpawnPoint);
		float iconRadius = 16.0f;

		Gdiplus::SolidBrush spawnBrush(Gdiplus::Color(200, 0, 255, 0));
		Gdiplus::Pen spawnPen(Gdiplus::Color(255, 255, 255, 255), 3.0f);

		pGraphics->FillEllipse(&spawnBrush, screenPos.X - iconRadius, screenPos.Y - iconRadius, iconRadius * 2.0f, iconRadius * 2.0f);
		pGraphics->DrawEllipse(&spawnPen, screenPos.X - iconRadius, screenPos.Y - iconRadius, iconRadius * 2.0f, iconRadius * 2.0f);

		// 플레이어 심볼 (P 문자)
		if (m_isPlayerSpawnMode) {
			Gdiplus::Font playerFont(L"Arial", 12, Gdiplus::FontStyleBold);
			Gdiplus::SolidBrush playerTextBrush(Gdiplus::Color(255, 255, 255, 255));
			Gdiplus::RectF playerTextRect(screenPos.X - 8, screenPos.Y - 8, 16, 16);
			Gdiplus::StringFormat centerFormat;
			centerFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
			centerFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
			pGraphics->DrawString(L"P", 1, &playerFont, playerTextRect, &centerFormat, &playerTextBrush);

			// 좌표 정보 표시
			Gdiplus::Font coordFont(L"Arial", 10);
			Gdiplus::SolidBrush coordBrush(Gdiplus::Color(255, 255, 255, 255));
			std::wstringstream coordSS;
			coordSS << L"(" << (int)m_playerSpawnPoint.X << L", " << (int)m_playerSpawnPoint.Y << L")";
			Gdiplus::RectF coordRect(screenPos.X - 40, screenPos.Y + iconRadius + 5, 80, 15);
			pGraphics->DrawString(coordSS.str().c_str(), -1, &coordFont, coordRect, &centerFormat, &coordBrush);
		}
	}
}

// 좌표 변환 함수들 (EditorView로 위임, g_hWnd 사용)
Gdiplus::RectF DontStarve_EditorMain::GetViewWorldRect(float cullingMargin) const {
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	return m_pView->GetViewWorldRect(clientRect.right, clientRect.bottom, cullingMargin);
}

Gdiplus::PointF DontStarve_EditorMain::WorldToScreen(Gdiplus::PointF worldPos) const {
	return m_pView->WorldToScreen(worldPos);
}

Gdiplus::RectF DontStarve_EditorMain::WorldToScreen(Gdiplus::RectF worldRect) const {
	return m_pView->WorldToScreen(worldRect);
}

Gdiplus::PointF DontStarve_EditorMain::ScreenToWorld(Gdiplus::PointF screenPos) const {
	return m_pView->ScreenToWorld(screenPos);
}

// HandleMessage 헬퍼 함수들
void DontStarve_EditorMain::HandlePlacingModeClick(POINT clickPoint, HWND hWnd) {
	int selIdx = m_pPalette->GetSelectedPaletteIndex();
	const PaletteItem* pSelectedItem = (selIdx >= 0) ? m_pPalette->GetPaletteItem((size_t)selIdx) : nullptr;
	if (!pSelectedItem) return;

	const PaletteItem& selectedItem = *pSelectedItem;

		if (selectedItem.category == CATEGORY_TILE) {
		const ResourcePathUtils::TileResourceDef* tv = m_pPalette->GetSelectedTileVariant();
		if (!tv) {
			OutputDebugStringW(L"Error: TileResourceDef is NULL for selected palette tile.\n");
			return;
		}

		int mapX = (int)floor(m_snappedPreviewPos.X / TILE_SIZE);
		int mapY = (int)floor(m_snappedPreviewPos.Y / TILE_SIZE);

		if (m_is3x3Mode) {
			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					int targetX = mapX + dx;
					int targetY = mapY + dy;
					if (targetX >= 0 && targetX < MAP_WIDTH && targetY >= 0 && targetY < MAP_HEIGHT) {
						m_tileMap[targetY][targetX] = ResourcePathUtils::TileResourceDef(tv->type, tv->id, tv->baseDir, tv->imageName);
					}
				}
			}
		}
		else {
			if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
				m_tileMap[mapY][mapX] = ResourcePathUtils::TileResourceDef(tv->type, tv->id, tv->baseDir, tv->imageName);
			}
		}
		m_pLayerComposer->SetTileLayerDirty(true);
		InvalidateRect(hWnd, NULL, FALSE);
	}
	else if (selectedItem.category == CATEGORY_OBJECT) {
		const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
		if (!ov) {
			OutputDebugStringW(L"Error: ObjectResourceDef is NULL for selected palette object.\n");
			return;
		}

		GameObjectID selectedObjectID = m_pPalette->GetSelectedGameObjectID();

		// 콜라이더 템플릿 찾기
		bool hasCollider = true;
		ColliderType colliderType = COLLIDER_BOX;
		int colliderOffsetX, colliderOffsetY, colliderWidth, colliderHeight;
		float colliderCenterX, colliderCenterY, colliderRadius;

		auto templateIt = m_colliderTemplates.find(std::make_pair((int)selectedItem.typeId, (int)selectedObjectID));
		if (templateIt != m_colliderTemplates.end()) {
			const ColliderTemplate& t = templateIt->second;
			hasCollider = t.hasCollider;
			colliderType = t.colliderType;
			colliderOffsetX = t.colliderOffsetX;
			colliderOffsetY = t.colliderOffsetY;
			colliderWidth = t.colliderWidth;
			colliderHeight = t.colliderHeight;
			colliderCenterX = t.colliderCenterX;
			colliderCenterY = t.colliderCenterY;
			colliderRadius = t.colliderRadius;
		}
		else {
			// 맵 내 같은 타입 오브젝트 찾기
			const ResourcePathUtils::ObjectResourceDef* sameTypeTemplate = nullptr;
			for (const ResourcePathUtils::ObjectResourceDef& obj : m_gameObjects) {
				if (obj.type == (GameObjectType)selectedItem.typeId && obj.id == selectedObjectID) {
					sameTypeTemplate = &obj;
					break;
				}
			}
			if (sameTypeTemplate) {
				hasCollider = sameTypeTemplate->hasCollider;
				colliderType = sameTypeTemplate->colliderType;
				colliderOffsetX = sameTypeTemplate->colliderOffsetX;
				colliderOffsetY = sameTypeTemplate->colliderOffsetY;
				colliderWidth = sameTypeTemplate->colliderWidth;
				colliderHeight = sameTypeTemplate->colliderHeight;
				colliderCenterX = sameTypeTemplate->colliderCenterX;
				colliderCenterY = sameTypeTemplate->colliderCenterY;
				colliderRadius = sameTypeTemplate->colliderRadius;
			}
			else {
				// 이미지 크기 기반 기본값 (절대경로에서 로드)
				int imageWidth = 32, imageHeight = 32; // 기본값
				if (!ov->imageName.empty()) {
					std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
					std::unique_ptr<Gdiplus::Bitmap> pBitmap(BitmapUtils::LoadBitmapFromFile(fullPath.c_str()));
					if (pBitmap && pBitmap->GetLastStatus() == Gdiplus::Ok) {
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

		ResourcePathUtils::ObjectResourceDef newObject((GameObjectType)selectedItem.typeId, selectedObjectID,
			m_snappedPreviewPos.X, m_snappedPreviewPos.Y,
			ov->baseDir, ov->imageName, ov->pivotX, ov->pivotY,
			hasCollider, colliderType,
			colliderOffsetX, colliderOffsetY, colliderWidth, colliderHeight,
			colliderCenterX, colliderCenterY, colliderRadius);
		AddObject(newObject);
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void DontStarve_EditorMain::HandleObjectSelectionClick(POINT clickPoint, HWND hWnd) {
	Gdiplus::PointF mouseWorldClickPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

	for (int i = (int)m_gameObjects.size() - 1; i >= 0; --i) {
		ResourcePathUtils::ObjectResourceDef& obj = m_gameObjects[i];
		const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(obj.type, obj.id);
		if (!ov || ov->imageName.empty()) continue;

		// 이미지 크기 로드
		std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
		std::unique_ptr<Gdiplus::Bitmap> pBitmap(BitmapUtils::LoadBitmapFromFile(fullPath.c_str()));
		if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) continue;

		float objWidthWorld = (float)pBitmap->GetWidth();
		float objHeightWorld = (float)pBitmap->GetHeight();
		float objRenderLeftWorld = obj.x - (ov->pivotX * objWidthWorld);
		float objRenderTopWorld = obj.y - (ov->pivotY * objHeightWorld);

		Gdiplus::RectF objWorldRect(objRenderLeftWorld, objRenderTopWorld, objWidthWorld, objHeightWorld);

		if (objWorldRect.Contains(mouseWorldClickPos.X, mouseWorldClickPos.Y)) {
			// Pixel perfect click check
			int pixelX = (int)(mouseWorldClickPos.X - objRenderLeftWorld);
			int pixelY = (int)(mouseWorldClickPos.Y - objRenderTopWorld);

			Gdiplus::Color color;
			if (ov->pAtlasBitmap && pixelX >= 0 && pixelY >= 0 &&
				pixelX < (int)ov->pAtlasBitmap->GetWidth() && pixelY < (int)ov->pAtlasBitmap->GetHeight()) {
				ov->pAtlasBitmap->GetPixel(pixelX, pixelY, &color);
				if (color.GetAlpha() > 0) {
					m_selectedObjectPtr = &obj;
					m_pLayerComposer->SetObjectLayerDirty(true);
					InvalidateRect(hWnd, NULL, FALSE);
					return;
				}
			}
		}
	}

	// 오브젝트를 클릭하지 않았으면 선택 해제
	DeselectObject(hWnd);
}

void DontStarve_EditorMain::DeselectObject(HWND hWnd) {
	if (m_selectedObjectPtr != nullptr) {
		m_pColliderEditor->EndColliderEdit();
		m_selectedObjectPtr = nullptr;
		m_pLayerComposer->SetObjectLayerDirty(true);
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void DontStarve_EditorMain::ExitAllEditModes() {
	m_pPivotEditor->EndPivotEdit();
	m_pColliderEditor->EndColliderEdit();
	m_pWalkableEditor->EndWalkableEdit();
	m_isPlayerSpawnMode = false;
	m_isPlacingMode = false;
	if (m_pPalette->IsSubPaletteOpen()) {
		m_pPalette->CloseSubPalette();
		m_paletteLayerDirty = true;
	}
}
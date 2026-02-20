#include "../pch.h"
#include "DontStarve_EditorMain.h"
#include "../Resource.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../03_EditorMapFileIO/EditorMap.h"
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
	m_hasPlayerSpawn(false), m_playerSpawnPoint(0.0f, 0.0f), m_isPlayerSpawnMode(false),
	m_mapWidth(MAP_WIDTH), m_mapHeight(MAP_HEIGHT)
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

	// 팔레트 레이어 비트맵 생성 (InitPalette 이후에 Rect 확정됨, 크기가 유효할 때만)
	const RECT& pr = m_pPalette->GetPaletteRect();
	int paletteW = pr.right - pr.left;
	int paletteH = pr.bottom - pr.top;
	if (paletteW > 0 && paletteH > 0) {
		m_paletteLayerBitmap = new Gdiplus::Bitmap(paletteW, paletteH, PixelFormat32bppARGB);
	}

	// 디버그 패널 의존성 설정 (DrawDebugInfo에서 사용)
	m_pDebugPanel->SetDependencies(m_pView.get(), m_pPalette.get(), m_pPivotEditor.get(), m_pColliderEditor.get(), m_pWalkableEditor.get(), this);

	// 플레이어 스폰 포인트를 맵 중앙으로 초기화
	float centerX = (m_mapWidth / 2.0f) * TILE_SIZE;
	float centerY = (m_mapHeight / 2.0f) * TILE_SIZE;
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;

	std::wstringstream debugSS;
	debugSS << L"Initial Player Spawn set to map center: (" << (int)centerX << L", " << (int)centerY
		<< L")px = Tile(" << (m_mapWidth / 2) << L", " << (m_mapHeight / 2) << L")\n";
	OutputDebugStringW(debugSS.str().c_str());

	m_pLayerComposer->ComposeGridLayer();
	m_pLayerComposer->ComposeTileLayer();
	m_pLayerComposer->ComposeObjectLayer();
	m_pPalette->ComposePaletteLayer(m_paletteLayerBitmap, &m_paletteLayerDirty);
}

void DontStarve_EditorMain::InitPalette() {
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	m_pPalette->InitPalette(clientRect.right, clientRect.bottom, m_pResources.get());
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
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			int deltaX = m_rawMousePos.x - m_cameraDragStart.x;
			int deltaY = m_rawMousePos.y - m_cameraDragStart.y;

			// 맵 오프셋 업데이트 (드래그 방향과 반대로 이동, 맵 범위 내로 클램프)
			POINT oldOffset = m_pView->GetMapOffset();
			m_pView->SetMapOffsetClamped(
				m_initialMapOffset.x + deltaX, m_initialMapOffset.y + deltaY,
				clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
			POINT newOffset = m_pView->GetMapOffset();
			
			// 디버그: 오프셋 변화 출력
			std::wstringstream debugSS;
			debugSS << L"[DRAG MOVE] Delta(" << deltaX << L"," << deltaY 
					<< L") Old(" << oldOffset.x << L"," << oldOffset.y 
					<< L") New(" << newOffset.x << L"," << newOffset.y 
					<< L") MapSize(" << m_mapWidth << L"x" << m_mapHeight << L")\n";
			OutputDebugStringW(debugSS.str().c_str());
		
			// 뷰포트가 변경되었으므로 레이어 다시 그리기 필요
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
			if (pItem) {
				Gdiplus::PointF oldPreviewPos = m_snappedPreviewPos;
				Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)m_rawMousePos.x, (float)m_rawMousePos.y));

				if (pItem->category == CATEGORY_TILE) {
			// 타일: 그리드에 스냅된 월드 좌표 (좌상단)
			int snappedMapX = (int)floor(mouseWorldPos.X / TILE_SIZE);
			int snappedMapY = (int)floor(mouseWorldPos.Y / TILE_SIZE);
			snappedMapX = max(0, min(m_mapWidth - 1, snappedMapX));
			snappedMapY = max(0, min(m_mapHeight - 1, snappedMapY));
			m_snappedPreviewPos = Gdiplus::PointF((float)(snappedMapX * TILE_SIZE), (float)(snappedMapY * TILE_SIZE));
				}
				else if (pItem->category == CATEGORY_OBJECT) {
					// 오브젝트: 마우스 위치 그대로 (발 밑 중심)
					m_snappedPreviewPos = mouseWorldPos;
				}

				// 프리뷰 위치가 변경되었을 때만 화면 갱신
				if (oldPreviewPos.X != m_snappedPreviewPos.X || oldPreviewPos.Y != m_snappedPreviewPos.Y) {
					InvalidateRect(hWnd, NULL, FALSE);
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

		// 디버그 패널 위 클릭 시 맵/팔레트 처리하지 않음 (드래그·배치·선택 무시)
		if (IsPointInDebugPanel(clickPoint)) {
			return 0;
		}

		// 배치 모드일 때 맵 영역 클릭은 최우선 처리 (다른 편집 모드가 클릭을 가로채지 않도록)
		const RECT& paletteRectFirst = m_pPalette->GetPaletteRect();
		bool clickOnPaletteFirst = PtInRect(&paletteRectFirst, clickPoint) != FALSE;
		
		if (m_isPlacingMode && !clickOnPaletteFirst) {
			HandlePlacingModeClick(clickPoint, hWnd);
			return 0;
		}

		if (m_pWalkableEditor->IsWalkableEditMode()) {
			m_pWalkableEditor->OnLeftButtonDown(clickPoint, hWnd);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 2. Player Spawn Mode (우클릭이 눌려 있으면 스폰 설정하지 않음 - 우클릭 드래그와 겹침 방지)
		if (m_isPlayerSpawnMode) {
			if (GetKeyState(VK_RBUTTON) & 0x8000) {
				return 0;  // 우클릭 드래그 중에는 좌클릭으로 스폰 갱신하지 않음
			}
			Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

			// 현재 맵 크기 기준 경계 체크
			const float maxX = (float)(m_mapWidth * TILE_SIZE);
			const float maxY = (float)(m_mapHeight * TILE_SIZE);
			const bool isInBounds = (mouseWorldPos.X >= 0 && mouseWorldPos.X <= maxX &&
									 mouseWorldPos.Y >= 0 && mouseWorldPos.Y <= maxY);

			if (isInBounds) {
				m_playerSpawnPoint = mouseWorldPos;
				m_hasPlayerSpawn = true;

				// 디버그 로그
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
					<< L"Valid range: (0,0) to (" << (int)maxX << L"," << (int)maxY << L")\n";
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
				m_paletteLayerDirty = true;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			else {  // ClosedOutside
				// 팔레트 밖(맵) 클릭 → 선택 유지된 채 서브팔레트만 닫힘
				bool hasSelection = (m_pPalette->GetSelectedPaletteIndex() >= 0 &&
					(m_pPalette->GetSelectedTileVariant() != nullptr || m_pPalette->GetSelectedObjectVariant() != nullptr));
				
				m_isPlacingMode = hasSelection;
				m_paletteLayerDirty = true;
				InvalidateRect(hWnd, NULL, FALSE);
				
				if (!hasSelection) {
					m_pColliderEditor->EndColliderEdit();
					m_selectedObjectPtr = nullptr;
					return 0;
				}
				// hasSelection이면 return하지 않고 아래로 진행하여 이번 클릭으로 배치 수행
			}
		}

		// 4. Main Palette Click Handling (클릭이 팔레트 영역일 때만 처리)
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

		// 5. Map Click Handling (Placing Mode or Object Selection)
		// 배치 모드이고 맵 영역(팔레트 밖) 클릭일 때만 배치 수행 → 팔레트 영역 판정과 무관하게 배치가 확실히 동작
		if (m_isPlacingMode && !clickOnPaletteArea) {
			HandlePlacingModeClick(clickPoint, hWnd);
		}
		else if (!m_isPlacingMode) {
			HandleObjectSelectionClick(clickPoint, hWnd);
		}
	}
	break;

	case WM_RBUTTONDOWN:
	{
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		POINT clickPoint = { mouseX, mouseY };

		// 디버그 패널 위 클릭 시 카메라 드래그/오브젝트 선택 시작하지 않음
		if (IsPointInDebugPanel(clickPoint)) {
			OutputDebugStringW(L"[DRAG BLOCKED] Debug panel area\n");
			return 0;
		}

		// 1. 하위 팔레트가 열려 있으면 닫기
		if (m_pPalette->IsSubPaletteOpen()) {
			OutputDebugStringW(L"[DRAG BLOCKED] Closing sub-palette\n");
			m_pPalette->CloseSubPalette();
			m_isPlacingMode = false;
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 2. 배치 모드 중이면 배치 모드 해제
		if (m_isPlacingMode) {
			OutputDebugStringW(L"[DRAG BLOCKED] Exiting placing mode\n");
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
			debugSS << L"[DRAG START] Pos(" << clickPoint.x << L"," << clickPoint.y 
					<< L") InitOffset(" << m_initialMapOffset.x << L"," << m_initialMapOffset.y 
					<< L") Zoom(" << (int)(m_pView->GetZoomFactor() * 100) << L"%)\n";
			OutputDebugStringW(debugSS.str().c_str());
		}
		else {
			OutputDebugStringW(L"[DRAG BLOCKED] Palette area\n");
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

		// 디버그 패널 뷰포트 위에 마우스가 있을 때만 패널 스크롤 (그 외에는 줌)
		if (m_pDebugPanel->IsVisible()) {
			int mx = mouseClientPos.x;
			int my = mouseClientPos.y;
			Gdiplus::RectF vr = m_pDebugPanel->GetViewportRect();
			if (vr.Width > 0 && vr.Height > 0 &&
				(float)mx >= vr.X && (float)mx < vr.X + vr.Width &&
				(float)my >= vr.Y && (float)my < vr.Y + vr.Height) {
				OutputDebugStringW(L"[ZOOM BLOCKED] Debug panel scroll\n");
				m_pDebugPanel->HandleMouseWheel(zDelta, mx, my);
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

		// 디버그: 줌 변화 출력
		std::wstringstream zoomDebugSS;
		zoomDebugSS << L"[ZOOM] " << (zDelta > 0 ? L"IN" : L"OUT") 
					<< L" Old:" << (int)(oldZoomFactor * 100) << L"% -> New:" << (int)(newZoomFactor * 100) << L"%\n";
		OutputDebugStringW(zoomDebugSS.str().c_str());

		// 줌 팩터가 실제로 변경되었을 때만 처리
		if (newZoomFactor != oldZoomFactor) {
			// 변경된 줌 팩터로 마우스 위치에 해당하는 새로운 화면 좌표를 계산
			Gdiplus::PointF mouseScreenPos_after_zoom = WorldToScreen(mouseWorldPos_before_zoom);

			// 새로운 맵 오프셋 계산 (마우스가 클릭한 월드 지점이 화면상에서 같은 위치에 유지되도록, 맵 범위 내로 클램프)
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			POINT mo = m_pView->GetMapOffset();
			m_pView->SetMapOffsetClamped(
				mo.x + (LONG)(mouseScreenPos.x - mouseScreenPos_after_zoom.X),
				mo.y + (LONG)(mouseScreenPos.y - mouseScreenPos_after_zoom.Y),
				clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);

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
			bool needsRedraw = false;

			if (m_pPalette->IsSubPaletteOpen()) {
				m_pPalette->CloseSubPalette();
				m_isPlacingMode = false;
				m_paletteLayerDirty = true;
				needsRedraw = true;
			}
			else if (m_pColliderEditor->IsColliderEditMode()) {
				m_pColliderEditor->EndColliderEdit();
				m_pLayerComposer->SetObjectLayerDirty(true);
				needsRedraw = true;
			}
			else if (m_pPivotEditor->IsPivotEditMode()) {
				m_pPivotEditor->EndPivotEdit();
				m_pLayerComposer->SetObjectLayerDirty(true);
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
			else {
				DeselectObject(hWnd);
			}

			if (needsRedraw) {
				InvalidateRect(hWnd, NULL, FALSE);
			}
			return 0;
		}

		// Shift key for 3x3 mode toggle (only for tiles in placing mode)
		if (wParam == VK_SHIFT && m_isPlacingMode) {
			int sp = m_pPalette->GetSelectedPaletteIndex();
			const PaletteItem* pi = (sp >= 0) ? m_pPalette->GetPaletteItem((size_t)sp) : nullptr;
			if (pi && pi->category == CATEGORY_TILE) {
				m_is3x3Mode = !m_is3x3Mode;
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		// P key for Player Spawn mode toggle (피벗 편집 중일 때는 아래에서 P = 피벗 다이얼로그로 처리)
		if (wParam == 'P' && !m_pPivotEditor->IsPivotEditMode()) {
			if (m_isPlayerSpawnMode) {
				m_isPlayerSpawnMode = false;
			} else {
				ExitAllEditModes();  // 다른 편집 모드 해제 (이 안에서 m_isPlayerSpawnMode=false 됨)
				m_isPlayerSpawnMode = true;
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
				MessageBoxW(hWnd, L"오브젝트를 먼저 선택해주세요.", L"피벗 편집", MB_OK | MB_ICONINFORMATION);
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

		// P key: 피벗 값 입력 다이얼로그 (피벗 편집 모드일 때만)
		if (wParam == 'P' && m_pPivotEditor->IsPivotEditMode()) {
			m_pPivotEditor->ShowPivotDialog(hWnd);
			m_pLayerComposer->SetObjectLayerDirty(true);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// C key for Collider Edit mode toggle
		if (wParam == 'C') {
			if (!m_selectedObjectPtr) {
				MessageBoxW(hWnd, L"오브젝트를 먼저 선택해주세요.", L"콜라이더 편집", MB_OK | MB_ICONINFORMATION);
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

		// I key: 콜라이더 값 입력 다이얼로그 (콜라이더 편집 모드일 때만)
		if ((wParam == 'I' || wParam == 'i') && m_pColliderEditor->IsColliderEditMode()) {
			m_pColliderEditor->ShowColliderDialog(hWnd);
			m_pLayerComposer->SetObjectLayerDirty(true);
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
				MessageBoxW(hWnd, L"삭제할 오브젝트를 먼저 선택해주세요.", L"오브젝트 삭제", MB_OK | MB_ICONINFORMATION);
				return 0;
			}
			if (MessageBoxW(hWnd, L"선택된 오브젝트를 삭제하시겠습니까?", L"오브젝트 삭제", MB_YESNO | MB_ICONQUESTION) == IDYES) {
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
	float centerX = (m_mapWidth / 2.0f) * TILE_SIZE;
	float centerY = (m_mapHeight / 2.0f) * TILE_SIZE;
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
	m_pView->SetMapOffsetClamped(0, 0, clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
	m_pLayerComposer->SetTileLayerDirty(true);
	m_pLayerComposer->SetObjectLayerDirty(true);
	m_objectsDirty = true;
	m_paletteLayerDirty = true;
	m_pLayerComposer->SetGridLayerDirty(true);
}

void DontStarve_EditorMain::SetMapSize(int width, int height) {
	int w = max(1, min(MAP_WIDTH, width));
	int h = max(1, min(MAP_HEIGHT, height));
	if (w == m_mapWidth && h == m_mapHeight) return;
	m_mapWidth = w;
	m_mapHeight = h;
	// 맵 크기 변경 시 NewMap과 동일하게 초기화
	m_gameObjects.clear();
	m_selectedObjectPtr = nullptr;
	m_isPlacingMode = false;
	m_pPivotEditor->EndPivotEdit();
	m_pColliderEditor->EndColliderEdit();
	m_pWalkableEditor->EndWalkableEdit();
	m_isPlayerSpawnMode = false;
	m_pPalette->ResetSelection();
	float centerX = (m_mapWidth / 2.0f) * TILE_SIZE;
	float centerY = (m_mapHeight / 2.0f) * TILE_SIZE;
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
	m_pView->SetMapOffsetClamped(0, 0, clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
	m_pLayerComposer->SetTileLayerDirty(true);
	m_pLayerComposer->SetObjectLayerDirty(true);
	m_objectsDirty = true;
	m_paletteLayerDirty = true;
	m_pLayerComposer->SetGridLayerDirty(true);
}

// Map save/load functions (delegate to EditorMap)
bool DontStarve_EditorMain::SaveMap(const WCHAR* filename) {
	return EditorMap::SaveMap(this, filename);
}

bool DontStarve_EditorMain::LoadMap(const WCHAR* filename) {
	return EditorMap::LoadMap(this, filename);
}

bool DontStarve_EditorMain::ShowSaveFileDialog(WCHAR* fileName, DWORD fileNameSize) {
	return EditorMap::ShowSaveFileDialog(this, fileName, fileNameSize);
}

bool DontStarve_EditorMain::ShowOpenFileDialog(WCHAR* fileName, DWORD fileNameSize) {
	return EditorMap::ShowOpenFileDialog(this, fileName, fileNameSize);
}

// DrawGrid, DrawTileMap, DrawObjects, ComposeTileLayer, ComposeObjectLayer moved to EditorLayerComposer

// DrawPreview (배치 프리뷰 그리기 - 투명하고 그리드 크기에 맞춤)
void DontStarve_EditorMain::DrawPreview(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || !m_isPlacingMode) return;

	int selIdx = m_pPalette->GetSelectedPaletteIndex();
	const PaletteItem* pSelectedItem = (selIdx >= 0) ? m_pPalette->GetPaletteItem((size_t)selIdx) : nullptr;
	if (!pSelectedItem) return;
	const PaletteItem& selectedItem = *pSelectedItem;
	Gdiplus::Bitmap* previewBitmap = nullptr;
	Gdiplus::RectF previewSourceRect;

	if (selectedItem.category == CATEGORY_TILE) {
		const ResourcePathUtils::TileResourceDef* tv = m_pPalette->GetSelectedTileVariant();
		if (tv && !tv->imageName.empty()) {
			std::wstring fullPath = tv->baseDir;
			if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
				fullPath += L"\\";
			}
			fullPath += tv->imageName;
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
			std::wstring fullPath = ov->baseDir;
			if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
				fullPath += L"\\";
			}
			fullPath += ov->imageName;
			std::shared_ptr<Gdiplus::Bitmap> sharedBitmap = m_pResources->GetCachedBitmap(fullPath);
			if (sharedBitmap) {
				previewBitmap = sharedBitmap.get();
				previewSourceRect = Gdiplus::RectF(0, 0, (float)previewBitmap->GetWidth(), (float)previewBitmap->GetHeight());
			}
		}
	}

	if (!previewBitmap) return;

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
		if (!ov_preview) return;

		finalRenderWidth = previewSourceRect.Width * m_pView->GetZoomFactor();
		finalRenderHeight = previewSourceRect.Height * m_pView->GetZoomFactor();
		finalRenderX = screenPreviewPos.X - (ov_preview->pivotX * finalRenderWidth);
		finalRenderY = screenPreviewPos.Y - (ov_preview->pivotY * finalRenderHeight);

		// 오브젝트 프리뷰 배경 (바운딩 박스 표시)
		Gdiplus::Pen previewBBoxPen(Gdiplus::Color(150, 0, 255, 255), 1.5f);
		Gdiplus::RectF previewBBoxRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
		pGraphics->DrawRectangle(&previewBBoxPen, previewBBoxRect);

		// 피벗 포인트 표시
		Gdiplus::SolidBrush pivotBrush(Gdiplus::Color(200, 255, 0, 0));
		Gdiplus::RectF pivotRect(screenPreviewPos.X - 3.0f, screenPreviewPos.Y - 3.0f, 6.0f, 6.0f);
		pGraphics->FillEllipse(&pivotBrush, pivotRect);
	}

	// 투명 프리뷰 이미지 그리기
	Gdiplus::RectF destRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
	pGraphics->DrawImage(previewBitmap, destRect,
		0, 0, previewSourceRect.Width, previewSourceRect.Height,
		Gdiplus::UnitPixel, &imageAttr);

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
		infoRect.Y = finalRenderY + finalRenderHeight + 5;
	}
	if (infoRect.X + infoRect.Width > clientRect.right) {
		infoRect.X = clientRect.right - infoRect.Width - 10;
	}

	pGraphics->FillRectangle(&infoBackBrush, infoRect);
	pGraphics->DrawString(infoText.c_str(), -1, &infoFont, infoRect, nullptr, &infoBrush);
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

	// 플레이어 스폰 포인트 그리기 (항상 표시)
	if (m_hasPlayerSpawn) {
		Gdiplus::PointF screenPos = WorldToScreen(m_playerSpawnPoint);
		float iconRadius = 16.0f;

		// P 모드가 아닐 때는 반투명하게 표시하여 편집 불가능함을 시각적으로 표현
		int fillAlpha = m_isPlayerSpawnMode ? 200 : 100;
		int textAlpha = m_isPlayerSpawnMode ? 255 : 150;
		float penWidth = m_isPlayerSpawnMode ? 3.0f : 1.5f;

		Gdiplus::SolidBrush spawnBrush(Gdiplus::Color(fillAlpha, 0, 255, 0));
		Gdiplus::Pen spawnPen(Gdiplus::Color(textAlpha, 255, 255, 255), penWidth);

		pGraphics->FillEllipse(&spawnBrush, screenPos.X - iconRadius, screenPos.Y - iconRadius, iconRadius * 2.0f, iconRadius * 2.0f);
		pGraphics->DrawEllipse(&spawnPen, screenPos.X - iconRadius, screenPos.Y - iconRadius, iconRadius * 2.0f, iconRadius * 2.0f);

		// 플레이어 심볼 (P 문자)
		Gdiplus::Font playerFont(L"Arial", 12, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush playerTextBrush(Gdiplus::Color(textAlpha, 255, 255, 255));
		Gdiplus::RectF playerTextRect(screenPos.X - 8, screenPos.Y - 8, 16, 16);
		Gdiplus::StringFormat centerFormat;
		centerFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
		centerFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		pGraphics->DrawString(L"P", 1, &playerFont, playerTextRect, &centerFormat, &playerTextBrush);

		// 좌표 정보 표시 (P 모드일 때만)
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

bool DontStarve_EditorMain::IsPointInDebugPanel(POINT clickPoint) const {
	if (!m_pDebugPanel->IsVisible()) return false;
	
	Gdiplus::RectF r = m_pDebugPanel->GetViewportRect();
	return (r.Width > 0 && r.Height > 0 &&
		(float)clickPoint.x >= r.X && (float)clickPoint.x < r.X + r.Width &&
		(float)clickPoint.y >= r.Y && (float)clickPoint.y < r.Y + r.Height);
}

void DontStarve_EditorMain::HandlePlacingModeClick(POINT clickPoint, HWND hWnd) {
	int selIdx = m_pPalette->GetSelectedPaletteIndex();
	const PaletteItem* pSelectedItem = (selIdx >= 0) ? m_pPalette->GetPaletteItem((size_t)selIdx) : nullptr;
	if (!pSelectedItem) return;

	const PaletteItem& selectedItem = *pSelectedItem;
	// 클릭 위치를 월드 좌표로 사용(팔레트 선택 직후 맵 클릭 시 m_snappedPreviewPos가 갱신되지 않으므로)
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
		UpdateWindow(hWnd); // 즉시 갱신
	} else if (selectedItem.category == CATEGORY_OBJECT) {
		const ResourcePathUtils::ObjectResourceDef* ov = m_pPalette->GetSelectedObjectVariant();
		if (!ov) return;

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
				// 이미지 크기 기반 기본값 (캐시된 비트맵에서 로드)
				int imageWidth = 32, imageHeight = 32; // 기본값
				if (!ov->imageName.empty()) {
					std::wstring fullPath = ov->baseDir;
					if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
						fullPath += L"\\";
					}
					fullPath += ov->imageName;
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

		ResourcePathUtils::ObjectResourceDef newObject((GameObjectType)selectedItem.typeId, selectedObjectID,
			mouseWorldPos.X, mouseWorldPos.Y,
			ov->baseDir, ov->imageName, ov->pivotX, ov->pivotY,
			hasCollider, colliderType,
			colliderOffsetX, colliderOffsetY, colliderWidth, colliderHeight,
			colliderCenterX, colliderCenterY, colliderRadius);
		AddObject(newObject);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd); // 즉시 갱신
	}
}

void DontStarve_EditorMain::HandleObjectSelectionClick(POINT clickPoint, HWND hWnd) {
	Gdiplus::PointF mouseWorldClickPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

	for (int i = (int)m_gameObjects.size() - 1; i >= 0; --i) {
		ResourcePathUtils::ObjectResourceDef& obj = m_gameObjects[i];
		const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(obj.type, obj.id);
		if (!ov || ov->imageName.empty()) continue;

		// 이미지 파일에서 로드 (아틀라스 미사용)
		std::wstring fullPath = ov->baseDir;
		if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
			fullPath += L"\\";
		}
		fullPath += ov->imageName;
		std::unique_ptr<Gdiplus::Bitmap> pBitmap(Gdiplus::Bitmap::FromFile(fullPath.c_str()));
		if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) {
			if (pBitmap) pBitmap.reset();
		}
		if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) continue;

		float objWidthWorld = (float)pBitmap->GetWidth();
		float objHeightWorld = (float)pBitmap->GetHeight();
		float objRenderLeftWorld = obj.x - (ov->pivotX * objWidthWorld);
		float objRenderTopWorld = obj.y - (ov->pivotY * objHeightWorld);

		Gdiplus::RectF objWorldRect(objRenderLeftWorld, objRenderTopWorld, objWidthWorld, objHeightWorld);

		if (objWorldRect.Contains(mouseWorldClickPos.X, mouseWorldClickPos.Y)) {
			// Pixel perfect click check (이미지 파일에서 픽셀 로드)
			int pixelX = (int)(mouseWorldClickPos.X - objRenderLeftWorld);
			int pixelY = (int)(mouseWorldClickPos.Y - objRenderTopWorld);

			Gdiplus::Color color;
			if (pBitmap && pixelX >= 0 && pixelY >= 0 &&
				pixelX < (int)pBitmap->GetWidth() && pixelY < (int)pBitmap->GetHeight()) {
				pBitmap->GetPixel(pixelX, pixelY, &color);
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

void DontStarve_EditorMain::ShowMapSizeDialog(HWND parent) {
	EditorMap::ShowMapSizeDialog(this, parent);
}
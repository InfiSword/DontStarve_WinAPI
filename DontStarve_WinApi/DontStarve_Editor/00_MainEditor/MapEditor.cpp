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
	// m_tileMap 초기화 (ResourcePathUtils::TileResourceDef 기본 생성자로 초기화)
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = ResourcePathUtils::TileResourceDef();
		}
	}

	// walkable area map 초기화 (기본값으로 모든 영역을 walkable)
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

	// Double Buffer Bitmap 및 Graphics 객체 설정
	m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
	m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);

	// GDI+ 보간 모드 설정 (Client와 동일: 픽셀 격자 - NearestNeighbor, HighSpeed)
	m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
	m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);

	// 초기 레이어 텍스처 생성 (디버그 타일 최종)
	const UINT INIT_LAYER_SIZE = 1024;  // 초기 1024x1024 타일

	m_pResources->LoadResources();
	m_pLayerComposer->SetDependencies(m_pView.get(), m_pResources.get(), this);
	m_pLayerComposer->ResizeLayerBitmaps(INIT_LAYER_SIZE, INIT_LAYER_SIZE);
	InitPalette();

	// 팔레트 레이어 비트맵 생성 (InitPalette 이후에 Rect 확정됨)
	const RECT& pr = m_pPalette->GetPaletteRect();
	int paletteW = pr.right - pr.left;
	int paletteH = pr.bottom - pr.top;
	if (paletteW > 0 && paletteH > 0) {
		m_paletteLayerBitmap = new Gdiplus::Bitmap(paletteW, paletteH, PixelFormat32bppARGB);
	}

	// 디버그 패널 의존성 설정 (MapEditor 모드에서는 Pivot/Collider는 nullptr)
	m_pDebugPanel->SetDependencies(m_pView.get(), m_pPalette.get(), nullptr, nullptr, m_pWalkableEditor.get(), this);
	// Walkable 에디터에 View·MapEditor 전달 (G키 Walkable 영역 편집에 필요)
	m_pWalkableEditor->SetDependencies(m_pView.get(), this);

	// 플레이어 스폰 초기 위치를 맵 중앙 타일의 중심으로 설정
	float centerX = ((float)((m_mapWidth - 1) / 2) + 0.5f) * TILE_SIZE;
	float centerY = ((float)((m_mapHeight - 1) / 2) + 0.5f) * TILE_SIZE;
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;

	std::wstringstream debugSS;
	int centerTileX = (m_mapWidth - 1) / 2;
	int centerTileY = (m_mapHeight - 1) / 2;
	debugSS << L"Initial Player Spawn set to map center: (" << (int)centerX << L", " << (int)centerY
		<< L")px = Tile(" << centerTileX << L", " << centerTileY << L") center\n";
	OutputDebugStringW(debugSS.str().c_str());

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

	// 레이어 합성용 타일 크기 (픽셀 vs 논리 좌표 고려)

	UINT targetWidth = max(512U, min((UINT)clientRect.right + 256, 1536U));   // 최대 1536px
	UINT targetHeight = max(512U, min((UINT)clientRect.bottom + 256, 1536U)); // 최대 1536px

	// 레이어 텍스처 크기 검사 (타일과 창 크기에 따라 리사이즈)
	Gdiplus::Bitmap* tileLayerBitmap = m_pLayerComposer->GetTileLayerBitmap();
	if (!tileLayerBitmap ||
		abs((int)tileLayerBitmap->GetWidth() - (int)targetWidth) > 128 ||
		abs((int)tileLayerBitmap->GetHeight() - (int)targetHeight) > 128) {

		m_pLayerComposer->ResizeLayerBitmaps(targetWidth, targetHeight);
	}

	// 매 프레임 레이어 보간 모드 (필요시 리사이즈)
	m_pLayerComposer->ComposeGridLayer();
	m_pLayerComposer->ComposeTileLayer();
	m_pLayerComposer->ComposeObjectLayer();

	m_pPalette->ComposePaletteLayer(m_paletteLayerBitmap, &m_paletteLayerDirty); // 팔레트만 위에 동일하게

	// 합성된 최종 레이어 보간 모드 (픽셀 적절히)
	m_pLayerComposer->DrawLayers(m_pGraphics);

	// 팔레트만 위에 동일하게 보간 모드
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

	// 디버그 패널 (F1 키로 토글 가능)
	if (m_pDebugPanel->IsVisible()) {
		m_pDebugPanel->DrawDebugInfo(m_pGraphics);
	}

	// 좌측 하단 Launcher 버튼
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

	// Client와 동일: GDI+ DrawImage 또는 BitBlt로 최종 출력 (픽셀 적절히)
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
	m_pResources->ReleaseResources(); // 리소스(이미지 파일 텍스처들) 해제

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
		// 초기 카메라 생성은 나중에 수행 (실제로는 Initialize에서 순차 수행)
		return 0;
	}

	case WM_PAINT: {
		// Render() 함수가 실제 그리기 작업을 수행한 뒤 화면에 BitBlt하는 방식
		// WM_PAINT에서 InvalidateRect 호출로 발생하며, 타일 같은 장치 사용하지 않습니다.
		// 리사이즈, 기타 등 경우를 처리하기 위해 BeginPaint/EndPaint만 호출합니다.
		// 실제 그리기 보간 모드는 Render() 함수에서 담당합니다.
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

		// Double buffer 크기 재생성
		Utils::SafeDelete(m_pDoubleBufferBitmap);
		Utils::SafeDelete(m_pGraphics);
		m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
		m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
		m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);

		// 팔레트 크기 갱신
		m_pPalette->InitPalette(clientRect.right, clientRect.bottom, m_pResources.get());
		Utils::SafeDelete(m_paletteLayerBitmap);
		const RECT& pr = m_pPalette->GetPaletteRect();
		m_paletteLayerBitmap = new Gdiplus::Bitmap(pr.right - pr.left, pr.bottom - pr.top, PixelFormat32bppARGB);
		m_paletteLayerDirty = true;

		UpdateLauncherButtonRect(clientRect.right, clientRect.bottom);

		// 레이어 캐시 무효화
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

		// 카메라 드래그 모드 (오른쪽 드래그 - 휠닫힌 방식)
		if (m_isDraggingCamera) {
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			int deltaX = m_rawMousePos.x - m_cameraDragStart.x;
			int deltaY = m_rawMousePos.y - m_cameraDragStart.y;

			// 맵 오프셋 업데이트 (드래그 방향과 반대로 이동, 맵 좌표로 고정)
			POINT oldOffset = m_pView->GetMapOffset();
			m_pView->SetMapOffsetClamped(
				m_initialMapOffset.x + deltaX, m_initialMapOffset.y + deltaY,
				clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
			POINT newOffset = m_pView->GetMapOffset();
			
			// 디버그 오프셋 변경 출력
			std::wstringstream debugSS;
			debugSS << L"[DRAG MOVE] Delta(" << deltaX << L"," << deltaY 
					<< L") Old(" << oldOffset.x << L"," << oldOffset.y 
					<< L") New(" << newOffset.x << L"," << newOffset.y 
					<< L") MapSize(" << m_mapWidth << L"x" << m_mapHeight << L")\n";
			OutputDebugStringW(debugSS.str().c_str());
		
			// 합성물이 변경되었으므로 레이어 다시 그리기 필요
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

		// 배치 모드일 때 프리뷰 위치 계산
		int selIdx = m_pPalette->GetSelectedPaletteIndex();
		if (selIdx != -1 && m_isPlacingMode) {
			const PaletteItem* pItem = m_pPalette->GetPaletteItem((size_t)selIdx);
			if (pItem) {
				Gdiplus::PointF oldPreviewPos = m_snappedPreviewPos;
				Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)m_rawMousePos.x, (float)m_rawMousePos.y));

				if (pItem->category == CATEGORY_TILE) {
			// 타일 그리드에 스냅된 코드 좌표 (위쪽)
			int snappedMapX = (int)floor(mouseWorldPos.X / TILE_SIZE);
			int snappedMapY = (int)floor(mouseWorldPos.Y / TILE_SIZE);
			snappedMapX = max(0, min(m_mapWidth - 1, snappedMapX));
			snappedMapY = max(0, min(m_mapHeight - 1, snappedMapY));
			m_snappedPreviewPos = Gdiplus::PointF((float)(snappedMapX * TILE_SIZE), (float)(snappedMapY * TILE_SIZE));
				}
				else if (pItem->category == CATEGORY_OBJECT) {
					// 오브젝트: 마우스 좌표 그대로 (픽셀 픽셀 단위)
					m_snappedPreviewPos = mouseWorldPos;
				}

				// 그리드 좌표가 변경될 수 있으므로 디버그 갱신
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

		// 좌측 하단 Launcher 버튼 클릭 시 런처로 복귀
		if (IsPointInLauncherButton(clickPoint)) {
			m_requestedSwitch = EditorScreenSwitch::BackToLauncher;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 디버그 패널 영역 클릭 시 팔레트 처리하지 않음 (드래그블록같블록 등 때문)
		if (IsPointInDebugPanel(clickPoint)) {
			return 0;
		}

		// 배치 모드 등 영역 클릭은 최우선 처리 (다른 편집 모드가 클릭을 가로채도 되도록)
		const RECT& paletteRectFirst = m_pPalette->GetPaletteRect();
		bool clickOnPaletteFirst = PtInRect(&paletteRectFirst, clickPoint) != FALSE;
		
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

		// 2. Player Spawn Mode (오른쪽이 눌려 있으면 스폰 설정 안 함 - 오른쪽 드래그와 구분 위해)
		if (m_isPlayerSpawnMode) {
			if (GetKeyState(VK_RBUTTON) & 0x8000) {
				return 0;  // 오른쪽 드래그 중이면 플레이어스폰으로 갱신하지 않음
			}
			Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

			// 현재 맵 타일 범위 내 검사
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
				// 팔레트 등 맵 클릭 취소 시 서브팔레트만 닫음
				bool hasSelection = (m_pPalette->GetSelectedPaletteIndex() >= 0 &&
					(m_pPalette->GetSelectedTileVariant() != nullptr || m_pPalette->GetSelectedObjectVariant() != nullptr));
				
				m_isPlacingMode = hasSelection;
				m_paletteLayerDirty = true;
				InvalidateRect(hWnd, NULL, FALSE);
				
	if (!hasSelection) {
					m_selectedObjectPtr = nullptr;
					return 0;
				}
				// hasSelection이면 return하지 않고 계속 진행하여 다음 클릭으로 배치 수행
			}
		}

		// 4. Main Palette Click Handling (클릭 시 팔레트 영역 처리)
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
		// 배치 모드 따라 맵 영역(팔레트 밖 클릭 처리) 배치 수행 또는 팔레트 영역 편집 등 따라 배치가 끝나거나 시작
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

		// 디버그 패널 영역 클릭 시 카메라 드래그 오브젝트 클릭 시작하지 않음
		if (IsPointInDebugPanel(clickPoint)) {
			OutputDebugStringW(L"[DRAG BLOCKED] Debug panel area\n");
			return 0;
		}

		// 1. 서브 팔레트 열려 있으면 먼저 닫기
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

		// 3. 카메라 드래그 시작 (오른쪽 - 팔레트 영역 밖 경우)
		const RECT& pr = m_pPalette->GetPaletteRect();
		if (!PtInRect(&pr, clickPoint)) {
			m_isDraggingCamera = true;
			m_cameraDragStart = clickPoint;
			m_initialMapOffset = m_pView->GetMapOffset();
			SetCapture(hWnd); // 마우스 캡처
			
			// 디버그 패널 출력
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
		// 마우스 캡처가 다른 곳으로 해제된 경우 드래그 상태 초기화
		if (m_isDraggingCamera) {
			m_isDraggingCamera = false;
			std::wstringstream debugSS;
			debugSS << L"Camera drag cancelled due to capture loss\n";
			OutputDebugStringW(debugSS.str().c_str());
		}
		if (m_pWalkableEditor->IsDraggingWalkable()) {
			m_pWalkableEditor->OnLeftButtonUp();
		}
	}
	break;

	case WM_RBUTTONUP:
	{
		if (m_isDraggingCamera) {
			// 드래그 거리 계산 (클릭 시점 드래그인지 여부)
			int deltaX = m_rawMousePos.x - m_cameraDragStart.x;
			int deltaY = m_rawMousePos.y - m_cameraDragStart.y;
			int dragDistanceSquared = deltaX * deltaX + deltaY * deltaY;

			m_isDraggingCamera = false;
			ReleaseCapture();

			// 디버그 패널 출력
			std::wstringstream debugSS;
			debugSS << L"Camera drag ended - distance: " << sqrt(dragDistanceSquared) << L" pixels\n";
			OutputDebugStringW(debugSS.str().c_str());

			// 드래그 거리가 5픽셀 미만이면 클릭으로 간주하여 오브젝트 클릭 처리 (5^2 = 25)
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

		// 디버그 패널 영역에 마우스가 있으면 해당 영역 체크로 (휠 입력에 따라)
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

		// 디버그 줌 변경 출력
		std::wstringstream zoomDebugSS;
		zoomDebugSS << L"[ZOOM] " << (zDelta > 0 ? L"IN" : L"OUT") 
					<< L" Old:" << (int)(oldZoomFactor * 100) << L"% -> New:" << (int)(newZoomFactor * 100) << L"%\n";
		OutputDebugStringW(zoomDebugSS.str().c_str());

		// 줌 팩터가 이전과 변경될 수 있으므로 처리
		if (newZoomFactor != oldZoomFactor) {
			// 변경된 줌 팩터로 마우스 좌표에 대응하는 오프셋 좌표를 계산
			Gdiplus::PointF mouseScreenPos_after_zoom = WorldToScreen(mouseWorldPos_before_zoom);

			// 오프셋 맵 오프셋 계산 (마우스 위치 코드 지점이 맵 영역과 같은 좌표로 보이도록 맵 좌표로 고정)
			RECT clientRect;
			GetClientRect(hWnd, &clientRect);
			POINT mo = m_pView->GetMapOffset();
			m_pView->SetMapOffsetClamped(
				mo.x + (LONG)(mouseScreenPos.x - mouseScreenPos_after_zoom.X),
				mo.y + (LONG)(mouseScreenPos.y - mouseScreenPos_after_zoom.Y),
				clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);

			// 줌 변경 시 모든 레이어 캐시 무효화 (디버그 이후 타일을 다시 그림)
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
			// 우선순위: 서브팔레트 > 파일메뉴 편집 > 보기 편집 > 플레이어 스폰 > 워커블 편집 > 오브젝트 클릭 해제
			bool needsRedraw = false;

			if (m_pPalette->IsSubPaletteOpen()) {
				m_pPalette->CloseSubPalette();
				m_isPlacingMode = false;
				m_paletteLayerDirty = true;
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

		// P key for Player Spawn mode toggle (보기 편집 중인 경우 누른 키로 P = 보기 스폰모드로 설정)
		if (wParam == 'P') {
			if (m_isPlayerSpawnMode) {
				m_isPlayerSpawnMode = false;
			} else {
				ExitAllEditModes();  // 다른 편집 모드 해제 (여기서는 m_isPlayerSpawnMode=false 됨)
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

		// R key for Delete selected object
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
			// Walkable 모드 진입 시: 먼저 다른 편집 모드만 해제한 뒤 토글 (순서 반대면 ExitAllEditModes()의 EndWalkableEdit()이 방금 켠 모드를 다시 끔)
			if (!wasActive)
				ExitAllEditModes();
			m_pWalkableEditor->ToggleWalkableEditMode();
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
	// 플레이어 스폰을 맵 중앙 타일의 중심으로 설정
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
	m_pView->SetMapOffsetClamped(0, 0, clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
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
	// 맵 크기 변경 시 NewMap과 동일하게 초기화
	m_gameObjects.clear();
	m_selectedObjectPtr = nullptr;
	m_isPlacingMode = false;
	m_pWalkableEditor->EndWalkableEdit();
	m_isPlayerSpawnMode = false;
	m_pPalette->ResetSelection();
	// 플레이어 스폰을 맵 중앙 타일의 중심으로 설정
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
	m_pView->SetMapOffsetClamped(0, 0, clientRect.right, clientRect.bottom, m_mapWidth, m_mapHeight);
	m_pLayerComposer->SetTileLayerDirty(true);
	m_pLayerComposer->SetObjectLayerDirty(true);
	m_objectsDirty = true;
	m_paletteLayerDirty = true;
	m_pLayerComposer->SetGridLayerDirty(true);
}

// Map save/load functions (delegate to EditorMapFileIO)
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

// DrawGrid, DrawTileMap, DrawObjects, ComposeTileLayer, ComposeObjectLayer moved to EditorLayerComposer

// DrawPreview (배치 그리드 그리기 - 반투명하여 그리드와 타일 구분)
void MapEditor::DrawPreview(Gdiplus::Graphics* pGraphics) {
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

	// 반투명 설정을 위한 ColorMatrix
	Gdiplus::ColorMatrix colorMatrix = {
		1.0f, 0.0f, 0.0f, 0.0f, 0.0f,  // Red
		0.0f, 1.0f, 0.0f, 0.0f, 0.0f,  // Green  
		0.0f, 0.0f, 1.0f, 0.0f, 0.0f,  // Blue
		0.0f, 0.0f, 0.0f, 0.6f, 0.0f,  // Alpha (60% 반투명)
		0.0f, 0.0f, 0.0f, 0.0f, 1.0f   // Scale
	};

	Gdiplus::ImageAttributes imageAttr;
	imageAttr.SetColorMatrix(&colorMatrix);

	Gdiplus::PointF screenPreviewPos = WorldToScreen(m_snappedPreviewPos);
	float finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight;

	if (selectedItem.category == CATEGORY_TILE) {
		// 타일 코드 좌표에 맞춰 TILE_SIZE 타일, 디버그 변환 적용
		finalRenderWidth = (float)TILE_SIZE * m_pView->GetZoomFactor();
		finalRenderHeight = (float)TILE_SIZE * m_pView->GetZoomFactor();
		finalRenderX = screenPreviewPos.X;
		finalRenderY = screenPreviewPos.Y;

		// 타일 그리드 테두리 (그리드 영역 표시)
		Gdiplus::Pen previewGridPen(Gdiplus::Color(150, 255, 255, 0), 2.0f);
		Gdiplus::RectF previewGridRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
		pGraphics->DrawRectangle(&previewGridPen, previewGridRect);

		// 3x3 모드일 때 추가 그리드 표시
		if (m_is3x3Mode) {
			Gdiplus::Pen gridPen3x3(Gdiplus::Color(100, 255, 255, 0), 1.5f);
			float tileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();

			for (int dy = -1; dy <= 1; ++dy) {
				for (int dx = -1; dx <= 1; ++dx) {
					// 3x3 그리드 각의 코드 좌표 계산
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
		// 오브젝트: 원본 타일 기준
		const ResourcePathUtils::ObjectResourceDef* ov_preview = m_pPalette->GetSelectedObjectVariant();
		if (!ov_preview) return;

		finalRenderWidth = previewSourceRect.Width * m_pView->GetZoomFactor();
		finalRenderHeight = previewSourceRect.Height * m_pView->GetZoomFactor();
		finalRenderX = screenPreviewPos.X - (ov_preview->pivotX * finalRenderWidth);
		finalRenderY = screenPreviewPos.Y - (ov_preview->pivotY * finalRenderHeight);

		// 오브젝트 그리드 테두리 (시안색 파란 표시)
		Gdiplus::Pen previewBBoxPen(Gdiplus::Color(150, 0, 255, 255), 1.5f);
		Gdiplus::RectF previewBBoxRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
		pGraphics->DrawRectangle(&previewBBoxPen, previewBBoxRect);

		// 피벗 기준 표시
		Gdiplus::SolidBrush pivotBrush(Gdiplus::Color(200, 255, 0, 0));
		Gdiplus::RectF pivotRect(screenPreviewPos.X - 3.0f, screenPreviewPos.Y - 3.0f, 6.0f, 6.0f);
		pGraphics->FillEllipse(&pivotBrush, pivotRect);
	}

	// 반투명 그리드 위에 그리기
	Gdiplus::RectF destRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
	pGraphics->DrawImage(previewBitmap, destRect,
		0, 0, previewSourceRect.Width, previewSourceRect.Height,
		Gdiplus::UnitPixel, &imageAttr);

	// 그리드 위에 라벨 (마우스 위치 등 표시)
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

	// 디버그 범위 검사 (라벨이 디버그와 겹치지 않도록)
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

// 디버그 패널 그리기 (DrawDebugInfo moved to EditorDebugPanel)

void MapEditor::SetDebugInfoVisible(bool visible) {
	if (m_pDebugPanel && visible != m_pDebugPanel->IsVisible()) {
		m_pDebugPanel->ToggleVisibility();
	}
}

bool MapEditor::IsDebugInfoVisible() const {
	return m_pDebugPanel ? m_pDebugPanel->IsVisible() : false;
}

// 오브젝트 추가 함수
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
		obj->x = newX;
		obj->y = newY;
		m_objectsDirty = true;
		m_pLayerComposer->SetObjectLayerDirty(true);
	}
}

// 픽셀 메모리 사용량 함수 (ARGB 4바이트/픽셀)
float MapEditor::GetLayerMemoryUsageMB() const {
	Gdiplus::Bitmap* tileLayerBitmap = m_pLayerComposer->GetTileLayerBitmap();
	if (!tileLayerBitmap) return 0.0f;

	UINT totalPixels = tileLayerBitmap->GetWidth() * tileLayerBitmap->GetHeight();
	return (totalPixels * 4) / (1024.0f * 1024.0f); // 4바이트/픽셀 (ARGB)
}

// 플레이어 스폰 위치 그리기
void MapEditor::DrawPlayerSpawn(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;

	// 플레이어 스폰 모드일 때 위에 라벨 표시
	if (m_isPlayerSpawnMode) {
		Gdiplus::Font font(L"Arial", 14, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 0));
		Gdiplus::SolidBrush backgroundBrush(Gdiplus::Color(150, 0, 0, 0));

		std::wstring modeText = L"[PLAYER SPAWN MODE] Click to set spawn point (P to exit)";
		Gdiplus::RectF textRect(10, 40, 600, 30);

		pGraphics->FillRectangle(&backgroundBrush, textRect);
		pGraphics->DrawString(modeText.c_str(), -1, &font, textRect, nullptr, &textBrush);
	}

	// 플레이어 스폰 위치 그리기 (위에 표시)
	if (m_hasPlayerSpawn) {
		Gdiplus::PointF screenPos = WorldToScreen(m_playerSpawnPoint);
		float iconRadius = 16.0f;

		// P 모드가 아닐 때도 반투명하게 표시하여 편집 가능 여부를 주변으로 구분
		int fillAlpha = m_isPlayerSpawnMode ? 200 : 100;
		int textAlpha = m_isPlayerSpawnMode ? 255 : 150;
		float penWidth = m_isPlayerSpawnMode ? 3.0f : 1.5f;

		Gdiplus::SolidBrush spawnBrush(Gdiplus::Color(fillAlpha, 0, 255, 0));
		Gdiplus::Pen spawnPen(Gdiplus::Color(textAlpha, 255, 255, 255), penWidth);

		pGraphics->FillEllipse(&spawnBrush, screenPos.X - iconRadius, screenPos.Y - iconRadius, iconRadius * 2.0f, iconRadius * 2.0f);
		pGraphics->DrawEllipse(&spawnPen, screenPos.X - iconRadius, screenPos.Y - iconRadius, iconRadius * 2.0f, iconRadius * 2.0f);

		// 플레이어 표시 (P 문자)
		Gdiplus::Font playerFont(L"Arial", 12, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush playerTextBrush(Gdiplus::Color(textAlpha, 255, 255, 255));
		Gdiplus::RectF playerTextRect(screenPos.X - 8, screenPos.Y - 8, 16, 16);
		Gdiplus::StringFormat centerFormat;
		centerFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
		centerFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);
		pGraphics->DrawString(L"P", 1, &playerFont, playerTextRect, &centerFormat, &playerTextBrush);

		// 좌표 위에 표시 (P 모드일 때만)
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

// 좌표 변환 함수들 (EditorView에 위임, g_hWnd 사용)
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

// HandleMessage 보조 함수
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
	// ?대┃ ?꾩튂瑜??붾뱶 醫뚰몴濡??ъ슜(?붾젅???좏깮 吏곹썑 留??대┃ ??m_snappedPreviewPos媛 媛깆떊?섏? ?딆쑝誘濡?
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

		float pivotX = ov->pivotX;
		float pivotY = ov->pivotY;
		bool hasCollider = ov->hasCollider;
		ColliderType colliderType = ov->colliderType;
		int colliderOffsetX = ov->colliderOffsetX, colliderOffsetY = ov->colliderOffsetY;
		int colliderWidth = ov->colliderWidth, colliderHeight = ov->colliderHeight;
		float colliderCenterX = ov->colliderCenterX, colliderCenterY = ov->colliderCenterY, colliderRadius = ov->colliderRadius;

		// 리소스에 이미지 정보가 없으면 맵과 동일한 오브젝트 참조, 없으면 기본 타일 최종 기본값
		bool needFallback = !hasCollider || (colliderType == COLLIDER_BOX && colliderWidth <= 0 && colliderHeight <= 0) || (colliderType == COLLIDER_CIRCLE && colliderRadius <= 0.0f);
		if (needFallback) {
			const ResourcePathUtils::ObjectResourceDef* sameTypeTemplate = nullptr;
			for (const ResourcePathUtils::ObjectResourceDef& obj : m_gameObjects) {
				if (obj.type == (GameObjectType)selectedItem.typeId && obj.id == selectedObjectID) {
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
					std::wstring fullPath = ov->baseDir;
					if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') fullPath += L"\\";
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
			ov->baseDir, ov->imageName, pivotX, pivotY,
			hasCollider, colliderType,
			colliderOffsetX, colliderOffsetY, colliderWidth, colliderHeight,
			colliderCenterX, colliderCenterY, colliderRadius);
		AddObject(newObject);
		InvalidateRect(hWnd, NULL, FALSE);
		UpdateWindow(hWnd); // 즉시 갱신
	}
}

void MapEditor::HandleObjectSelectionClick(POINT clickPoint, HWND hWnd) {
	Gdiplus::PointF mouseWorldClickPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

	for (int i = (int)m_gameObjects.size() - 1; i >= 0; --i) {
		ResourcePathUtils::ObjectResourceDef& obj = m_gameObjects[i];
		const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(obj.type, obj.id);
		if (!ov || ov->imageName.empty()) continue;

		std::wstring fullPath = ov->baseDir;
		if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
			fullPath += L"\\";
		}
		fullPath += ov->imageName;
		std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
		if (!pBitmap) continue;

		float objWidthWorld = (float)pBitmap->GetWidth();
		float objHeightWorld = (float)pBitmap->GetHeight();
		float objRenderLeftWorld = obj.x - (ov->pivotX * objWidthWorld);
		float objRenderTopWorld = obj.y - (ov->pivotY * objHeightWorld);

		Gdiplus::RectF objWorldRect(objRenderLeftWorld, objRenderTopWorld, objWidthWorld, objHeightWorld);

		// 이미지 크기 바운딩 박스로 클릭 시 선택 (클릭 시 빨간 테두리 = 이미지 테두리)
		if (objWorldRect.Contains(mouseWorldClickPos.X, mouseWorldClickPos.Y)) {
			m_selectedObjectPtr = &obj;
			m_pLayerComposer->SetObjectLayerDirty(true);
			InvalidateRect(hWnd, NULL, FALSE);
			return;
		}
	}

	// 오브젝트를 클릭하지 않았으면 선택 해제
	DeselectObject(hWnd);
}

void MapEditor::DeselectObject(HWND hWnd) {
	if (m_selectedObjectPtr != nullptr) {
		m_selectedObjectPtr = nullptr;
		m_pLayerComposer->SetObjectLayerDirty(true);
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void MapEditor::ExitAllEditModes() {
	m_pWalkableEditor->EndWalkableEdit();
	m_isPlayerSpawnMode = false;
	m_isPlacingMode = false;
	if (m_pPalette->IsSubPaletteOpen()) {
		m_pPalette->CloseSubPalette();
		m_paletteLayerDirty = true;
	}
}

void MapEditor::ShowMapSizeDialog(HWND parent) {
	EditorMapFileIO::ShowMapSizeDialog(this, parent);
}

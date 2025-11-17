#include "../pch.h" 
#include "DontStarve_EditorMain.h"
#include <commdlg.h>  // 파일 다이얼로그용

DontStarve_EditorMain::DontStarve_EditorMain()
	: m_pGraphics(nullptr), m_pDoubleBufferBitmap(nullptr),
	m_selectedPaletteIndex(-1), m_isPlacingMode(false), m_is3x3Mode(false),
	m_rawMousePos({ 0,0 }), m_snappedPreviewPos(0.0f, 0.0f),
	m_isPivotEditMode(false), m_pivotEditPos({ 0,0 }), m_currentPivotX(0.5f), m_currentPivotY(1.0f), m_editingObject(nullptr),
	m_isColliderEditMode(false), m_editingColliderObject(nullptr), m_isDraggingCollider(false), m_draggingHandle(-1),
	m_mapOffset({ 0,0 }),
	m_zoomFactor(1.0f), m_minZoom(0.25f), m_maxZoom(2.0f), m_zoomStep(0.25f),
	m_objectsDirty(true),
	m_selectedObjectPtr(nullptr),
	m_tileLayerBitmap(nullptr), m_tileLayerDirty(true),
	m_objectLayerBitmap(nullptr), m_objectLayerDirty(true),
	m_paletteLayerBitmap(nullptr), m_paletteLayerDirty(true),
	m_gridLayerBitmap(nullptr), m_gridLayerDirty(true),
	m_hasPlayerSpawn(false), m_playerSpawnPoint(0.0f, 0.0f), m_isPlayerSpawnMode(false)
{
	// m_tileMap 초기화 (TileData의 기본 생성자 호출)
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = TileData();
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

	// GDI+ 렌더링 품질 설정
	m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
	m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
	m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

	// 초기 레이어 비트맵 생성 (화면 크기 기반)
	const UINT INIT_LAYER_SIZE = 1024;  // 초기 1024x1024 크기

	m_tileLayerBitmap = new Gdiplus::Bitmap(INIT_LAYER_SIZE, INIT_LAYER_SIZE, PixelFormat32bppARGB);
	m_objectLayerBitmap = new Gdiplus::Bitmap(INIT_LAYER_SIZE, INIT_LAYER_SIZE, PixelFormat32bppARGB);
	m_gridLayerBitmap = new Gdiplus::Bitmap(INIT_LAYER_SIZE, INIT_LAYER_SIZE, PixelFormat32bppARGB);

	LoadResources(); // resources.txt 로드 (아틀라스 비트맵 및 variants 채움)
	InitPalette();   // 팔레트 초기화 (m_paletteRect 및 m_paletteLayerBitmap 생성 포함)

	// 플레이어 스폰 포인트를 맵 중앙으로 초기화
	float centerX = (MAP_WIDTH / 2.0f) * TILE_SIZE;  // 25 * 128 = 3200px
	float centerY = (MAP_HEIGHT / 2.0f) * TILE_SIZE; // 25 * 128 = 3200px
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;

	std::wstringstream debugSS;
	debugSS << L"Initial Player Spawn set to map center: (" << (int)centerX << L", " << (int)centerY
		<< L")px = Tile(" << (MAP_WIDTH / 2) << L", " << (MAP_HEIGHT / 2) << L")\n";
	OutputDebugStringW(debugSS.str().c_str());

	ComposeGridLayer();
	ComposeTileLayer();
	ComposeObjectLayer();
	ComposePaletteLayer();
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
	if (!m_tileLayerBitmap ||
		abs((int)m_tileLayerBitmap->GetWidth() - (int)targetWidth) > 128 ||
		abs((int)m_tileLayerBitmap->GetHeight() - (int)targetHeight) > 128) {

		SafeDelete(m_tileLayerBitmap);
		SafeDelete(m_objectLayerBitmap);
		SafeDelete(m_gridLayerBitmap);

		m_tileLayerBitmap = new Gdiplus::Bitmap(targetWidth, targetHeight, PixelFormat32bppARGB);
		m_objectLayerBitmap = new Gdiplus::Bitmap(targetWidth, targetHeight, PixelFormat32bppARGB);
		m_gridLayerBitmap = new Gdiplus::Bitmap(targetWidth, targetHeight, PixelFormat32bppARGB);

		m_tileLayerDirty = true;
		m_objectLayerDirty = true;
		m_gridLayerDirty = true;
	}

	// 스마트 레이어 렌더링 (필요할 때만)
	ComposeGridLayer();
	ComposeTileLayer();
	ComposeObjectLayer();

	ComposePaletteLayer(); // 팔레트는 항상 동일

	// 뷰포트 기반 레이어 렌더링 (성능 최적화)
	if (m_gridLayerBitmap) {
		// 레이어 비트맵을 1:1 스케일로 화면에 그리기 (스케일링 없음)
		m_pGraphics->DrawImage(m_gridLayerBitmap, 0, 0);
	}

	if (m_tileLayerBitmap) {
		m_pGraphics->DrawImage(m_tileLayerBitmap, 0, 0);
	}

	if (m_objectLayerBitmap) {
		m_pGraphics->DrawImage(m_objectLayerBitmap, 0, 0);
	}

	// 팔레트는 항상 동일하게 렌더링
	if (m_paletteLayerBitmap) {
		m_pGraphics->DrawImage(m_paletteLayerBitmap,
			(Gdiplus::REAL)m_paletteRect.left, (Gdiplus::REAL)m_paletteRect.top,
			(Gdiplus::REAL)(m_paletteRect.right - m_paletteRect.left),
			(Gdiplus::REAL)(m_paletteRect.bottom - m_paletteRect.top));
	}

	DrawPreview(m_pGraphics);
	DrawSubPalette(m_pGraphics);
	DrawPivotEditor(m_pGraphics);
	DrawColliders(m_pGraphics);
	DrawPlayerSpawn(m_pGraphics);
	DrawWalkableAreas(m_pGraphics);

	// 디버그 정보 (F1 키로 토글 가능)
	if (m_showDebugInfo) {
		DrawDebugInfo(m_pGraphics);
	}

	HDC hdc = GetDC(g_hWnd);
	Gdiplus::Graphics screenGraphics(hdc);
	screenGraphics.DrawImage(m_pDoubleBufferBitmap, 0, 0, clientRect.right, clientRect.bottom);
	ReleaseDC(g_hWnd, hdc);
}

void DontStarve_EditorMain::Release()
{
	ReleaseResources(); // 리소스 (아틀라스 비트맵들) 해제

	SafeDelete(m_pGraphics);
	SafeDelete(m_pDoubleBufferBitmap);
	SafeDelete(m_tileLayerBitmap);
	SafeDelete(m_objectLayerBitmap);
	SafeDelete(m_paletteLayerBitmap);
	SafeDelete(m_gridLayerBitmap);
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
		// 윈도우 크기가 변경될 때 처리 (모든 비트맵 다시 생성 및 팔레트 위치 재계산)
		// m_pDoubleBufferBitmap은 항상 윈도우 크기에 맞춰야 함.
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);

		if (m_pDoubleBufferBitmap) SafeDelete(m_pDoubleBufferBitmap);
		if (m_pGraphics) SafeDelete(m_pGraphics);

		m_pDoubleBufferBitmap = new Gdiplus::Bitmap(clientRect.right, clientRect.bottom, PixelFormat32bppARGB);
		m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
		m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeBilinear);
		m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeDefault);
		m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);

		// 팔레트 위치 재계산 (InitPalette는 m_paletteRect도 초기화하므로 다시 호출)
		InitPalette(); // m_paletteLayerBitmap도 여기서 재생성 및 m_paletteLayerDirty = true;

		// 맵 레이어들도 크기 변화에 따라 재그리기 필요
		// (맵 자체의 크기는 고정이지만, 렌더링 스케일 등이 변경되었을 때 전체를 다시 그릴 필요가 있을 수 있음)
		// 여기서는 dirty 플래그만 설정
		m_gridLayerDirty = true;
		m_tileLayerDirty = true;
		m_objectLayerDirty = true;

		InvalidateRect(hWnd, NULL, FALSE); // 화면 갱신 요청
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
			m_mapOffset.x = m_initialMapOffset.x + deltaX;
			m_mapOffset.y = m_initialMapOffset.y + deltaY;
		
			// 뷰포트 렌더링에서는 맵 오프셋 변경 시 전체 재그리기 필요
			m_gridLayerDirty = true;
			m_tileLayerDirty = true;
			m_objectLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// Walkable 영역 드래그 모드
		if (m_isDraggingWalkable && m_isWalkableEditMode) {
			m_walkableDragEnd = m_rawMousePos;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 콜라이더 드래그 모드
		if (m_isDraggingCollider && m_editingColliderObject) {
			// 마우스 이동량 (화면 좌표)
			int deltaX = m_rawMousePos.x - m_colliderEditStartMousePos.x;
			int deltaY = m_rawMousePos.y - m_colliderEditStartMousePos.y;

			// 맵 좌표계로 변환된 이동량 (줌 팩터 고려)
			// g_displayScaleFactor도 적용되어야 합니다.
			int unzoomedDeltaX = (int)(deltaX / (m_zoomFactor));
			int unzoomedDeltaY = (int)(deltaY / (m_zoomFactor));

			if (m_draggingHandle == 4) { // 중앙 이동 핸들 (콜라이더 전체 이동)
				m_editingColliderObject->colliderOffsetX = m_initialColliderRect.left + unzoomedDeltaX;
				m_editingColliderObject->colliderOffsetY = m_initialColliderRect.top + unzoomedDeltaY;
			}
			else { // 크기 조절 (모서리 핸들 드래그)
				int newLeft = m_initialColliderRect.left;
				int newTop = m_initialColliderRect.top;
				int newRight = m_initialColliderRect.right;
				int newBottom = m_initialColliderRect.bottom;

				// 드래그하는 핸들에 따라 새로운 경계 계산
				if (m_draggingHandle == 0 || m_draggingHandle == 2) { // 좌측 핸들 (좌상단, 좌하단)
					newLeft = m_initialColliderRect.left + unzoomedDeltaX;
				}
				if (m_draggingHandle == 0 || m_draggingHandle == 1) { // 상단 핸들 (좌상단, 우상단)
					newTop = m_initialColliderRect.top + unzoomedDeltaY;
				}
				if (m_draggingHandle == 1 || m_draggingHandle == 3) { // 우측 핸들 (우상단, 우하단)
					newRight = m_initialColliderRect.right + unzoomedDeltaX;
				}
				if (m_draggingHandle == 2 || m_draggingHandle == 3) { // 하단 핸들 (좌하단, 우하단)
					newBottom = m_initialColliderRect.bottom + unzoomedDeltaY;
				}

				// 너비/높이가 최소값 이하로 줄어들지 않도록 제한
				if (newRight - newLeft < MIN_COLLIDER_SIZE) {
					if (m_draggingHandle == 0 || m_draggingHandle == 2) newLeft = newRight - MIN_COLLIDER_SIZE; // 좌측 핸들이면 좌측 경계 조정
					else newRight = newLeft + MIN_COLLIDER_SIZE; // 우측 핸들이면 우측 경계 조정
				}
				if (newBottom - newTop < MIN_COLLIDER_SIZE) {
					if (m_draggingHandle == 0 || m_draggingHandle == 1) newTop = newBottom - MIN_COLLIDER_SIZE; // 상단 핸들이면 상단 경계 조정
					else newBottom = newTop + MIN_COLLIDER_SIZE; // 하단 핸들이면 하단 경계 조정
				}

				// 콜라이더 정보 업데이트
				m_editingColliderObject->colliderOffsetX = newLeft;
				m_editingColliderObject->colliderOffsetY = newTop;
				m_editingColliderObject->colliderWidth = newRight - newLeft;
				m_editingColliderObject->colliderHeight = newBottom - newTop;
			}
			m_objectLayerDirty = true; // 콜라이더 변경 -> 레이어 다시 그려야 함
			InvalidateRect(hWnd, NULL, FALSE);
			return 0; // 메시지 처리 완료
		}

		// Pivot 편집 모드 중 실시간 pivot 업데이트
		if (m_isPivotEditMode && (GetKeyState(VK_LBUTTON) & 0x8000)) {
			UpdatePivotEdit(m_rawMousePos);
			return 0;
		}

		// 배치 모드일 때 프리뷰 위치 계산 (월드 좌표)
		if (m_selectedPaletteIndex != -1 && m_isPlacingMode) {
			const PaletteItem& selectedItem = m_paletteItems[m_selectedPaletteIndex];

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
	break;

	case WM_LBUTTONDOWN: {
		RECT clientRect;
		GetClientRect(hWnd, &clientRect);
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		POINT clickPoint = { mouseX, mouseY };

		// 1. Walkable Area Edit Mode (Highest Priority)
		if (m_isWalkableEditMode) {
			m_isDraggingWalkable = true;
			m_walkableDragStart = clickPoint;
			m_walkableDragEnd = clickPoint;
			SetCapture(hWnd); // 마우스 캡처 시작
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

		// 2. Collider Edit Mode
		if (m_isColliderEditMode && m_editingColliderObject) {
			m_draggingHandle = GetColliderHandleAt(clickPoint);
			if (m_draggingHandle != -1) {
				m_isDraggingCollider = true;
				m_colliderEditStartMousePos = clickPoint;
				m_initialColliderRect = {
					m_editingColliderObject->colliderOffsetX,
					m_editingColliderObject->colliderOffsetY,
					m_editingColliderObject->colliderOffsetX + m_editingColliderObject->colliderWidth,
					m_editingColliderObject->colliderOffsetY + m_editingColliderObject->colliderHeight
				};
				SetCapture(hWnd); // 마우스 캡처 (드래그 중에도 메시지 받기 위함)
				m_objectLayerDirty = true; // 선택 변경
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		// 2. Pivot Edit Mode
		if (m_isPivotEditMode) {
			UpdatePivotEdit(clickPoint);
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 3. Sub-Palette Click Handling
		if (m_subPalette.isOpen) {
			if (PtInRect(&m_subPalette.rect, clickPoint)) {
				for (size_t i = 0; i < m_subPalette.itemRects.size(); ++i) {
					if (PtInRect(&m_subPalette.itemRects[i], clickPoint)) {
						if (m_subPalette.category == CATEGORY_TILE) {
							m_subPalette.selectedTileVariantIndex = (int)i; // Cast size_t to int
						}
						else if (m_subPalette.category == CATEGORY_OBJECT) {
							m_subPalette.selectedObjectVariantIndex = (int)i; // Cast size_t to int
						}
						m_isPlacingMode = true;
						m_subPalette.isOpen = false;
						m_paletteLayerDirty = true; // Sub-palette selection/state change
						InvalidateRect(hWnd, NULL, FALSE);
						return 0;
					}
				}
			}
			// Clicked outside sub-palette
			m_subPalette.isOpen = false;
			m_subPalette.selectedTileVariantIndex = -1;
			m_subPalette.selectedObjectVariantIndex = -1;
			m_isPlacingMode = false;
			m_selectedObjectPtr = nullptr;
			m_paletteLayerDirty = true;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 4. Main Palette Click Handling
		if (PtInRect(&m_paletteRect, clickPoint)) {
			for (size_t i = 0; i < m_paletteItems.size(); ++i) {
				if (PtInRect(&m_paletteItems[i].displayRect, clickPoint)) {
					m_selectedPaletteIndex = (int)i; // Cast size_t to int

					// 메인팔레트 클릭 시에는 배치 모드 시작하지 않고 서브팔레트만 열기
					m_isPlacingMode = false;

					// Populate Sub-Palette based on selected category
					if (m_paletteItems[i].category == CATEGORY_TILE) {
						m_subPalette.category = CATEGORY_TILE;
						m_subPalette.targetCategoryId = m_paletteItems[i].typeId;
						m_subPalette.currentTileVariantDefs.clear();
						auto type_map_it = m_tileVariants.find((TileType)m_paletteItems[i].typeId);
						if (type_map_it != m_tileVariants.end()) {
							for (auto const& pair : type_map_it->second) {
								m_subPalette.currentTileVariantDefs.push_back({ pair.first, &(pair.second) });
							}
						}
						if (!m_subPalette.currentTileVariantDefs.empty()) {
							m_subPalette.selectedTileVariantIndex = 0; // 첫 번째 아이템을 기본 선택
						}
						else {
							m_subPalette.selectedTileVariantIndex = -1;
						}
					}
					else if (m_paletteItems[i].category == CATEGORY_OBJECT) {
						m_subPalette.category = CATEGORY_OBJECT;
						m_subPalette.targetCategoryId = m_paletteItems[i].typeId;
						m_subPalette.currentObjectVariantDefs.clear();
						auto type_map_it = m_objectVariants.find((GameObjectType)m_paletteItems[i].typeId);
						if (type_map_it != m_objectVariants.end()) {
							for (auto const& pair : type_map_it->second) {
								m_subPalette.currentObjectVariantDefs.push_back({ pair.first, &(pair.second) });
							}
						}
						if (!m_subPalette.currentObjectVariantDefs.empty()) {
							m_subPalette.selectedObjectVariantIndex = 0; // 첫 번째 아이템을 기본 선택
						}
						else {
							m_subPalette.selectedObjectVariantIndex = -1;
						}
					}

					// Calculate Sub-Palette position and size
					int subPaletteWidth = 170;
					int subItemSize = 48;
					int subPadding = 5;
					int subItemsPerRow = subPaletteWidth / (subItemSize + subPadding);
					if (subItemsPerRow == 0) subItemsPerRow = 1;

					int subPaletteHeight = 0;
					size_t numItemsInSubPalette = 0;
					if (m_subPalette.category == CATEGORY_TILE) {
						numItemsInSubPalette = m_subPalette.currentTileVariantDefs.size();
					}
					else if (m_subPalette.category == CATEGORY_OBJECT) {
						numItemsInSubPalette = m_subPalette.currentObjectVariantDefs.size();
					}

					subPaletteHeight = (int)ceil((float)numItemsInSubPalette / subItemsPerRow) * (subItemSize + subPadding) + subPadding;
					if (subPaletteHeight > clientRect.bottom) subPaletteHeight = clientRect.bottom;

					m_subPalette.rect = { m_paletteRect.left - subPaletteWidth, 0, m_paletteRect.left, subPaletteHeight };
					m_subPalette.isOpen = true;
					m_subPalette.itemRects.clear();

					int currentSubX = m_subPalette.rect.left + subPadding;
					int currentSubY = m_subPalette.rect.top + subPadding;
					for (size_t j = 0; j < numItemsInSubPalette; ++j) {
						m_subPalette.itemRects.push_back({ currentSubX, currentSubY, currentSubX + subItemSize, currentSubY + subItemSize });
						currentSubX += subItemSize + subPadding;
						if ((j + 1) % subItemsPerRow == 0) {
							currentSubX = m_subPalette.rect.left + subPadding;
							currentSubY += subItemSize + subPadding;
						}
					}
					m_paletteLayerDirty = true; // 메인 팔레트 선택 변경 시 레이어 갱신
					InvalidateRect(hWnd, NULL, FALSE);
					return 0; // Handled main palette click
				}
			}
		}

		// 5. Map Click Handling (Placing Mode or Object Selection)
		if (m_isPlacingMode) {
			const PaletteItem& selectedItem = m_paletteItems[m_selectedPaletteIndex];

			Gdiplus::PointF mouseWorldClickPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

			if (selectedItem.category == CATEGORY_TILE) {
				const TileVariant* tv = m_subPalette.getSelectedTileVariant();

				if (tv) {
					// 프리뷰와 동일한 방식으로 타일 인덱스 계산
					int mapX = (int)floor(m_snappedPreviewPos.X / TILE_SIZE);
					int mapY = (int)floor(m_snappedPreviewPos.Y / TILE_SIZE);

					std::wstringstream debugSS;
					debugSS << L"Placing Tile: Type=" << (int)tv->type << L", ID=" << (int)tv->id
						<< L", MapPos=(" << mapX << L"," << mapY << L")"
						<< L", WorldPos=(" << (int)m_snappedPreviewPos.X << L"," << (int)m_snappedPreviewPos.Y << L")\n";
					OutputDebugStringW(debugSS.str().c_str());

					if (m_is3x3Mode) { // 3x3 배치 모드
						for (int dy = -1; dy <= 1; ++dy) {
							for (int dx = -1; dx <= 1; ++dx) {
								int targetX = mapX + dx;
								int targetY = mapY + dy;
								if (targetX >= 0 && targetX < MAP_WIDTH && targetY >= 0 && targetY < MAP_HEIGHT) {
									m_tileMap[targetY][targetX] = TileData(tv->type, tv->id, tv->pAtlasBitmap, tv->sourceRect);
								}
							}
						}
					}
					else
					{ // 단일 타일 배치
						if (mapX >= 0 && mapX < MAP_WIDTH && mapY >= 0 && mapY < MAP_HEIGHT) {
							m_tileMap[mapY][mapX] = TileData(tv->type, tv->id, tv->pAtlasBitmap, tv->sourceRect);
						}
					}
					m_tileLayerDirty = true; // 타일 변경 -> 레이어 다시 그려야 함
					InvalidateRect(hWnd, NULL, FALSE);
				}
				else {
					std::wstringstream debugSS;
					debugSS << L"Error: TileVariant is NULL. SubPalette - Category=" << (int)m_subPalette.category
						<< L", TileIdx=" << m_subPalette.selectedTileVariantIndex
						<< L", TileVariants.size=" << m_subPalette.currentTileVariantDefs.size() << L"\n";
					OutputDebugStringW(debugSS.str().c_str());
				}
			}
			else if (selectedItem.category == CATEGORY_OBJECT) {
				const ObjectVariant* ov = m_subPalette.getSelectedObjectVariant();
				if (ov) {
					// 서브팔레트에서 선택된 실제 ObjectID 사용
					GameObjectID selectedObjectID = m_subPalette.getSelectedGameObjectID();

					std::wstringstream debugSS;
					debugSS << L"Placing Object: Type=" << (int)selectedItem.typeId << L", ID=" << (int)selectedObjectID
						<< L", Pos=(" << m_snappedPreviewPos.X << L"," << m_snappedPreviewPos.Y << L")\n";
					OutputDebugStringW(debugSS.str().c_str());

					GameObjectData newObject((GameObjectType)selectedItem.typeId, selectedObjectID,
						m_snappedPreviewPos.X, m_snappedPreviewPos.Y,
						ov->objectAssetBaseDirectory, ov->pivotX, ov->pivotY);
					AddObject(newObject);
					InvalidateRect(hWnd, NULL, FALSE);
				}
				else {
					std::wstringstream debugSS;
					debugSS << L"Error: ObjectVariant is NULL. SubPalette - Category=" << (int)m_subPalette.category
						<< L", ObjIdx=" << m_subPalette.selectedObjectVariantIndex
						<< L", ObjVariants.size=" << m_subPalette.currentObjectVariantDefs.size() << L"\n";
					OutputDebugStringW(debugSS.str().c_str());
				}
			}
		}
		else { // Not in placing mode (object selection/deselection)
			// 배치 모드가 아닐 때 클릭한 경우 디버그 출력
			std::wstringstream debugSS;
			debugSS << L"Click ignored - Not in placing mode. PlacingMode=" << (m_isPlacingMode ? L"TRUE" : L"FALSE")
				<< L", SelectedPaletteIdx=" << m_selectedPaletteIndex << L"\n";
			OutputDebugStringW(debugSS.str().c_str());
			bool objectClicked = false;
			for (int i = (int)m_gameObjects.size() - 1; i >= 0; --i) {
				GameObjectData& obj = m_gameObjects[i];
				const ObjectVariant* ov = GetObjectVariant(obj.type, obj.id);

				if (!ov) continue;

				Gdiplus::PointF mouseWorldClickPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

				// Calculate object's world bounding box based on its x,y (bottom-center) and variant's size/pivot
				float objRenderLeftWorld = obj.x - (ov->pivotX * ov->sourceRect.Width);
				float objRenderTopWorld = obj.y - (ov->pivotY * ov->sourceRect.Height);
				float objWidthWorld = (Gdiplus::REAL)ov->sourceRect.Width;
				float objHeightWorld = (Gdiplus::REAL)ov->sourceRect.Height;

				Gdiplus::RectF objWorldRect(objRenderLeftWorld, objRenderTopWorld, objWidthWorld, objHeightWorld);

				if (objWorldRect.Contains(mouseWorldClickPos.X, mouseWorldClickPos.Y))
				{
					objectClicked = true;
					// Pixel perfect click check:
					int pixelX = (int)(ov->sourceRect.X + (mouseWorldClickPos.X - objRenderLeftWorld));
					int pixelY = (int)(ov->sourceRect.Y + (mouseWorldClickPos.Y - objRenderTopWorld));

					Gdiplus::Color color;
					if (ov->pAtlasBitmap && pixelX >= 0 && pixelY >= 0 &&
						pixelX < (int)ov->pAtlasBitmap->GetWidth() && pixelY < (int)ov->pAtlasBitmap->GetHeight())
					{
						ov->pAtlasBitmap->GetPixel(pixelX, pixelY, &color);
						if (color.GetAlpha() > 0) {
							// 오브젝트 선택 (삭제는 R키로만 가능)
							m_selectedObjectPtr = &obj;
							m_objectLayerDirty = true;
							InvalidateRect(hWnd, NULL, FALSE);
							break;
						}
					}
				}
			}
			if (!objectClicked) {
				if (m_selectedObjectPtr != nullptr) {
					m_selectedObjectPtr = nullptr;
					m_objectLayerDirty = true;
					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
		}
	}
					   break;

	case WM_RBUTTONDOWN:
	{
		int mouseX = LOWORD(lParam);
		int mouseY = HIWORD(lParam);
		POINT clickPoint = { mouseX, mouseY };

		// 1. 하위 팔레트가 열려 있으면 닫기
		if (m_subPalette.isOpen) {
			m_subPalette.isOpen = false;
			m_subPalette.selectedTileVariantIndex = -1;
			m_subPalette.selectedObjectVariantIndex = -1;
			m_isPlacingMode = false;
			m_paletteLayerDirty = true; // Sub-palette closed, needs redraw
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 2. 배치 모드 중이면 배치 모드 해제
		if (m_isPlacingMode) {
			m_isPlacingMode = false;
			m_selectedPaletteIndex = -1;
			m_paletteLayerDirty = true; // Palette selection changed, needs redraw
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// 3. 카메라 드래그 시작 (우클릭) - 팔레트 영역이 아닐 때만
		if (!PtInRect(&m_paletteRect, clickPoint)) {
			m_isDraggingCamera = true;
			m_cameraDragStart = clickPoint;
			m_initialMapOffset = m_mapOffset;
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
		if (m_isDraggingWalkable) {
			m_isDraggingWalkable = false;
		}
		if (m_isDraggingCollider) {
			m_isDraggingCollider = false;
			m_draggingHandle = -1;
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

				bool objectClicked = false;
				for (int i = (int)m_gameObjects.size() - 1; i >= 0; --i) {
					GameObjectData& obj = m_gameObjects[i];
					const ObjectVariant* ov = GetObjectVariant(obj.type, obj.id);

					if (!ov) continue;

					Gdiplus::PointF mouseWorldClickPos = ScreenToWorld(Gdiplus::PointF((float)clickPoint.x, (float)clickPoint.y));

					float objRenderLeftWorld = obj.x - (ov->pivotX * ov->sourceRect.Width);
					float objRenderTopWorld = obj.y - (ov->pivotY * ov->sourceRect.Height);
					float objWidthWorld = (Gdiplus::REAL)ov->sourceRect.Width;
					float objHeightWorld = (Gdiplus::REAL)ov->sourceRect.Height;

					Gdiplus::RectF objWorldRect(objRenderLeftWorld, objRenderTopWorld, objWidthWorld, objHeightWorld);

					if (objWorldRect.Contains(mouseWorldClickPos.X, mouseWorldClickPos.Y)) {
						int pixelX = (int)(ov->sourceRect.X + (mouseWorldClickPos.X - objRenderLeftWorld));
						int pixelY = (int)(ov->sourceRect.Y + (mouseWorldClickPos.Y - objRenderTopWorld));

						Gdiplus::Color color;
						if (ov->pAtlasBitmap && pixelX >= 0 && pixelY >= 0 &&
							pixelX < (int)ov->pAtlasBitmap->GetWidth() && pixelY < (int)ov->pAtlasBitmap->GetHeight()) {
							ov->pAtlasBitmap->GetPixel(pixelX, pixelY, &color);
							if (color.GetAlpha() > 0) { // If pixel is not fully transparent
								objectClicked = true;
								// 오브젝트 선택 (삭제는 R키로만 가능)
								m_selectedObjectPtr = &obj;
								m_objectLayerDirty = true; // Selection state changed, requires recompose
								InvalidateRect(hWnd, NULL, FALSE);
								break;
							}
						}
					}
				}

				if (!objectClicked) { // If no object was clicked
					if (m_selectedObjectPtr != nullptr) { // If an object was previously selected
						m_selectedObjectPtr = nullptr; // Deselect it
						m_objectLayerDirty = true; // Deselection requires recompose
						InvalidateRect(hWnd, NULL, FALSE);
					}
				}
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

		Gdiplus::PointF mouseWorldPos_before_zoom = ScreenToWorld(Gdiplus::PointF((float)mouseScreenPos.x, (float)mouseScreenPos.y));

		float oldZoomFactor = m_zoomFactor;

		if (zDelta > 0) { // Zoom In
			m_zoomFactor += m_zoomStep;
		}
		else { // Zoom Out
			m_zoomFactor -= m_zoomStep;
		}

		// 줌 팩터 범위 제한
		if (m_zoomFactor < m_minZoom) m_zoomFactor = m_minZoom;
		if (m_zoomFactor > m_maxZoom) m_zoomFactor = m_maxZoom;

		// 줌 팩터가 실제로 변경되었을 때만 처리
		if (m_zoomFactor != oldZoomFactor) {
			// 변경된 줌 팩터로 마우스 위치에 해당하는 새로운 화면 좌표를 계산
			Gdiplus::PointF mouseScreenPos_after_zoom = WorldToScreen(mouseWorldPos_before_zoom);

			// 새로운 맵 오프셋 계산 (마우스가 클릭한 월드 지점이 화면상에서 같은 위치에 유지되도록)
			m_mapOffset.x += (LONG)(mouseScreenPos.x - mouseScreenPos_after_zoom.X);
			m_mapOffset.y += (LONG)(mouseScreenPos.y - mouseScreenPos_after_zoom.Y);

			// 줌 변경 시 모든 레이어 재그리기 (화면에서 크기가 변하므로)
			m_gridLayerDirty = true;
			m_tileLayerDirty = true;
			m_objectLayerDirty = true;

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
			// Close sub-palette
			if (m_subPalette.isOpen) {
				m_subPalette.isOpen = false;
				m_subPalette.selectedTileVariantIndex = -1;
				m_subPalette.selectedObjectVariantIndex = -1;
				m_isPlacingMode = false;
				m_paletteLayerDirty = true; // State change
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			// Exit collider edit mode
			if (m_isColliderEditMode) {
				EndColliderEdit(); // This sets m_objectLayerDirty
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			// Exit pivot edit mode
			if (m_isPivotEditMode) {
				EndPivotEdit(); // This sets m_objectLayerDirty
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			// Exit player spawn mode
			if (m_isPlayerSpawnMode) {
				m_isPlayerSpawnMode = false;
				OutputDebugStringW(L"Player Spawn Mode: OFF (ESC pressed)\n");
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			// Exit walkable area edit mode
			if (m_isWalkableEditMode) {
				m_isWalkableEditMode = false;
				m_isDraggingWalkable = false;
				OutputDebugStringW(L"Walkable Area Edit Mode: OFF (ESC pressed)\n");
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
			// Deselect object if no modes are active
			if (m_selectedObjectPtr != nullptr) {
				m_selectedObjectPtr = nullptr;
				m_objectLayerDirty = true; // State change
				InvalidateRect(hWnd, NULL, FALSE);
				return 0;
			}
		}

		// Shift key for 3x3 mode toggle (only for tiles in placing mode)
		if (wParam == VK_SHIFT && m_selectedPaletteIndex != -1 && m_isPlacingMode &&
			m_paletteItems[m_selectedPaletteIndex].category == CATEGORY_TILE) {
			m_is3x3Mode = !m_is3x3Mode;
			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// P key for Player Spawn mode toggle
		if (wParam == 'P') {
			m_isPlayerSpawnMode = !m_isPlayerSpawnMode;

			// 플레이어 스폰 모드 활성화 시 다른 모드들 해제
			if (m_isPlayerSpawnMode) {
				m_isPlacingMode = false;
				m_isPivotEditMode = false;
				m_isColliderEditMode = false;
				if (m_subPalette.isOpen) {
					m_subPalette.isOpen = false;
					m_paletteLayerDirty = true;
				}
			}

			std::wstringstream debugSS;
			debugSS << L"Player Spawn Mode: " << (m_isPlayerSpawnMode ? L"ON" : L"OFF") << L"\n";
			OutputDebugStringW(debugSS.str().c_str());

			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// F1 key for Debug Info toggle
		if (wParam == VK_F1) {
			m_showDebugInfo = !m_showDebugInfo;

			std::wstringstream debugSS;
			debugSS << L"Debug Info: " << (m_showDebugInfo ? L"ON" : L"OFF") << L"\n";
			OutputDebugStringW(debugSS.str().c_str());

			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		// V key for Pivot Edit mode toggle
		if (wParam == 'V') {
			if (m_selectedObjectPtr) {
				if (!m_isPivotEditMode) {
					// 다른 편집 모드들 해제
					m_isColliderEditMode = false;
					m_isPlayerSpawnMode = false;
					m_isPlacingMode = false;
					if (m_subPalette.isOpen) {
						m_subPalette.isOpen = false;
						m_paletteLayerDirty = true;
					}

					StartPivotEdit(m_selectedObjectPtr);
					std::wstringstream debugSS;
					debugSS << L"Pivot Edit Mode: ON for selected object\n";
					OutputDebugStringW(debugSS.str().c_str());
				}
				else {
					EndPivotEdit();
					std::wstringstream debugSS;
					debugSS << L"Pivot Edit Mode: OFF\n";
					OutputDebugStringW(debugSS.str().c_str());
				}
				InvalidateRect(hWnd, NULL, FALSE);
			}
			else {
				MessageBox(hWnd, L"오브젝트를 먼저 선택해주세요.", L"피벗 편집", MB_OK | MB_ICONINFORMATION);
			}
			return 0;
		}

		// C key for Collider Edit mode toggle
		if (wParam == 'C') {
			if (m_selectedObjectPtr) {
				if (!m_isColliderEditMode) {
					// 다른 편집 모드들 해제
					m_isPivotEditMode = false;
					m_isPlayerSpawnMode = false;
					m_isPlacingMode = false;
					if (m_subPalette.isOpen) {
						m_subPalette.isOpen = false;
						m_paletteLayerDirty = true;
					}

					StartColliderEdit(m_selectedObjectPtr);
					std::wstringstream debugSS;
					debugSS << L"Collider Edit Mode: ON for selected object\n";
					OutputDebugStringW(debugSS.str().c_str());
				}
				else {
					EndColliderEdit();
					std::wstringstream debugSS;
					debugSS << L"Collider Edit Mode: OFF\n";
					OutputDebugStringW(debugSS.str().c_str());
				}
				InvalidateRect(hWnd, NULL, FALSE);
			}
			else {
				MessageBox(hWnd, L"오브젝트를 먼저 선택해주세요.", L"콜라이더 편집", MB_OK | MB_ICONINFORMATION);
			}
			return 0;
		}

		// R key for Delete selected object
		if (wParam == 'R') {
			if (m_selectedObjectPtr) {
				// 확인 대화상자 표시
				int result = MessageBox(hWnd, L"선택된 오브젝트를 삭제하시겠습니까?", L"오브젝트 삭제", MB_YESNO | MB_ICONQUESTION);

				if (result == IDYES) {
					// 편집 모드 해제 (삭제할 오브젝트가 편집 중이라면)
					if (m_editingObject == m_selectedObjectPtr) {
						EndPivotEdit();
					}
					if (m_editingColliderObject == m_selectedObjectPtr) {
						EndColliderEdit();
					}

					// 오브젝트 삭제
					RemoveObject(m_selectedObjectPtr);
					m_selectedObjectPtr = nullptr;

					std::wstringstream debugSS;
					debugSS << L"Object deleted successfully\n";
					OutputDebugStringW(debugSS.str().c_str());

					InvalidateRect(hWnd, NULL, FALSE);
				}
			}
			else {
				MessageBox(hWnd, L"삭제할 오브젝트를 먼저 선택해주세요.", L"오브젝트 삭제", MB_OK | MB_ICONINFORMATION);
			}
			return 0;
		}

		// G key for Walkable Area Edit mode toggle
		if (wParam == 'G') {
			m_isWalkableEditMode = !m_isWalkableEditMode;

			// Walkable 편집 모드 활성화 시 다른 모드들 해제
			if (m_isWalkableEditMode) {
				m_isPlacingMode = false;
				m_isPivotEditMode = false;
				m_isColliderEditMode = false;
				m_isPlayerSpawnMode = false;
				if (m_subPalette.isOpen) {
					m_subPalette.isOpen = false;
					m_paletteLayerDirty = true;
				}
				// 드래그 상태 초기화
				m_isDraggingWalkable = false;
			}

			std::wstringstream debugSS;
			debugSS << L"Walkable Area Edit Mode: " << (m_isWalkableEditMode ? L"ON" : L"OFF") << L"\n";
			OutputDebugStringW(debugSS.str().c_str());

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
		if (m_isDraggingWalkable && m_isWalkableEditMode) {
			// 드래그 영역의 타일들을 walkable로 설정
			Gdiplus::PointF startWorldPos = ScreenToWorld(Gdiplus::PointF((float)m_walkableDragStart.x, (float)m_walkableDragStart.y));
			Gdiplus::PointF endWorldPos = ScreenToWorld(Gdiplus::PointF((float)m_walkableDragEnd.x, (float)m_walkableDragEnd.y));

			// 타일 좌표로 변환
			int startTileX = max(0, min(MAP_WIDTH - 1, (int)floor(startWorldPos.X / TILE_SIZE)));
			int startTileY = max(0, min(MAP_HEIGHT - 1, (int)floor(startWorldPos.Y / TILE_SIZE)));
			int endTileX = max(0, min(MAP_WIDTH - 1, (int)floor(endWorldPos.X / TILE_SIZE)));
			int endTileY = max(0, min(MAP_HEIGHT - 1, (int)floor(endWorldPos.Y / TILE_SIZE)));

			// 시작과 끝 좌표 정리 (min, max)
			int minTileX = min(startTileX, endTileX);
			int maxTileX = max(startTileX, endTileX);
			int minTileY = min(startTileY, endTileY);
			int maxTileY = max(startTileY, endTileY);

			// 해당 영역의 타일들을 walkable로 설정 (토글 방식)
			bool newWalkableState = !m_walkableAreaMap[minTileY][minTileX]; // 첫 번째 타일의 반대 상태로 설정
			for (int y = minTileY; y <= maxTileY; ++y) {
				for (int x = minTileX; x <= maxTileX; ++x) {
					m_walkableAreaMap[y][x] = newWalkableState;
				}
			}

			// 드래그 상태 해제
			m_isDraggingWalkable = false;
			ReleaseCapture();

			std::wstringstream debugSS;
			debugSS << L"Walkable area set: (" << minTileX << L"," << minTileY << L") to ("
				<< maxTileX << L"," << maxTileY << L") = " << (newWalkableState ? L"WALKABLE" : L"BLOCKED") << L"\n";
			OutputDebugStringW(debugSS.str().c_str());

			InvalidateRect(hWnd, NULL, FALSE);
			return 0;
		}

		if (m_isDraggingCollider) {
			m_isDraggingCollider = false;
			m_draggingHandle = -1; // Reset drag handle
			ReleaseCapture();      // Release mouse capture
			m_objectLayerDirty = true; // Object's collider state might have changed
			InvalidateRect(hWnd, NULL, FALSE); // Redraw
			return 0; // Message handled
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


// 맵 저장 함수 구현
bool DontStarve_EditorMain::SaveMap(const WCHAR* filename) {
	std::wofstream outFile(filename);
	if (!outFile.is_open()) {
		OutputDebugStringW(L"맵 파일 열기 실패: ");
		OutputDebugStringW(filename);
		OutputDebugStringW(L"\n");
		return false;
	}

	// 맵 메타데이터 저장
	outFile << L"# MAP_METADATA\n";
	outFile << L"MAP_WIDTH=" << MAP_WIDTH << L"\n";
	outFile << L"MAP_HEIGHT=" << MAP_HEIGHT << L"\n";
	outFile << L"# Editor Info: FPS=" << (int)m_currentFPS
		<< L", Memory=" << (int)GetLayerMemoryUsageMB() << L"MB"
		<< L", Zoom=" << (int)(m_zoomFactor * 100) << L"%\n\n";

	// 플레이어 스폰 포인트 저장
	outFile << L"# PLAYER_SPAWN\n";
	if (m_hasPlayerSpawn) {
		outFile << L"PLAYER_SPAWN_X=" << m_playerSpawnPoint.X << L"\n";
		outFile << L"PLAYER_SPAWN_Y=" << m_playerSpawnPoint.Y << L"\n";
	}
	else {
		outFile << L"PLAYER_SPAWN_X=-1\n";
		outFile << L"PLAYER_SPAWN_Y=-1\n";
	}
	outFile << L"\n";

	// 타일 데이터 저장
	outFile << L"# TILES\n";
	for (int y = 0; y < MAP_HEIGHT; ++y) { // y를 먼저 순회 (MAP_HEIGHT)
		for (int x = 0; x < MAP_WIDTH; ++x) { // x를 나중에 순회 (MAP_WIDTH)
			TileData tile = m_tileMap[y][x];
			outFile << EnumUtils::GetEnumName(tile.type) << L","
				<< EnumUtils::GetEnumName(tile.id);
			if (x < MAP_WIDTH - 1) {
				outFile << L",";
			}
		}
		outFile << L"\n";
	}
	outFile << L"\n";

	// 오브젝트 데이터 저장
	outFile << L"# OBJECTS\n";
	for (const auto& obj : m_gameObjects) {
		outFile << EnumUtils::GetEnumName(obj.type) << L","
			<< EnumUtils::GetEnumName(obj.id) << L","
			<< obj.x << L"," << obj.y << L","
			<< obj.objectAssetBaseDirectory << L","
			<< obj.pivotX << L"," << obj.pivotY << L"\n";

		// 콜라이더 정보 저장
		outFile << obj.hasCollider << L","
			<< obj.colliderOffsetX << L","
			<< obj.colliderOffsetY << L","
			<< obj.colliderWidth << L","
			<< obj.colliderHeight << L"\n";
	}
	outFile << L"\n";

	// Walkable Area 데이터 저장
	outFile << L"# WALKABLE_AREAS\n";
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			outFile << (m_walkableAreaMap[y][x] ? L"1" : L"0");
			if (x < MAP_WIDTH - 1) {
				outFile << L",";
			}
		}
		outFile << L"\n";
	}

	outFile.close();

	// 저장된 walkable areas 정보 디버그 출력
	int walkableCount = 0;
	int blockedCount = 0;
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			if (m_walkableAreaMap[y][x]) walkableCount++;
			else blockedCount++;
		}
	}

	std::wstringstream debugSS;
	debugSS << L"Walkable Areas saved - Walkable: " << walkableCount
		<< L", Blocked: " << blockedCount << L" tiles\n";
	OutputDebugStringW(debugSS.str().c_str());

	OutputDebugStringW(L"맵 저장 완료: ");
	OutputDebugStringW(filename);
	OutputDebugStringW(L"\n");
	return true;
}

// 맵 불러오기 함수 구현
bool DontStarve_EditorMain::LoadMap(const WCHAR* filename) {
	std::wifstream inFile(filename);
	if (!inFile.is_open()) {
		OutputDebugStringW(L"맵 파일 열기 실패: ");
		OutputDebugStringW(filename);
		OutputDebugStringW(L"\n");
		return false;
	}

	// 기존 맵 데이터 초기화
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = TileData();
			m_walkableAreaMap[y][x] = true; // 기본값: walkable
		}
	}
	m_gameObjects.clear();
	m_hasPlayerSpawn = false;
	m_playerSpawnPoint = Gdiplus::PointF(0.0f, 0.0f);

	std::wstring line;
	bool inTilesSection = false;
	bool inObjectsSection = false;
	bool inPlayerSpawnSection = false;
	bool inWalkableAreasSection = false;
	int tileRowIndex = 0;
	int walkableRowIndex = 0;

	while (std::getline(inFile, line)) {
		// 빈 줄이나 주석 무시
		if (line.empty() || line[0] == L'#') {
			if (line == L"# PLAYER_SPAWN") {
				inPlayerSpawnSection = true;
				inTilesSection = false;
				inObjectsSection = false;
				inWalkableAreasSection = false;
			}
			else if (line == L"# TILES") {
				inTilesSection = true;
				inPlayerSpawnSection = false;
				inObjectsSection = false;
				inWalkableAreasSection = false;
				tileRowIndex = 0;
			}
			else if (line == L"# OBJECTS") {
				inObjectsSection = true;
				inTilesSection = false;
				inPlayerSpawnSection = false;
				inWalkableAreasSection = false;
			}
			else if (line == L"# WALKABLE_AREAS") {
				inWalkableAreasSection = true;
				inTilesSection = false;
				inPlayerSpawnSection = false;
				inObjectsSection = false;
				walkableRowIndex = 0;
			}
			continue;
		}

		if (inPlayerSpawnSection) {
			// 플레이어 스폰 포인트 파싱
			if (line.find(L"PLAYER_SPAWN_X=") == 0) {
				float spawnX = std::stof(line.substr(15));
				if (spawnX >= 0) {
					m_playerSpawnPoint.X = spawnX;
					m_hasPlayerSpawn = true;
				}
			}
			else if (line.find(L"PLAYER_SPAWN_Y=") == 0) {
				float spawnY = std::stof(line.substr(15));
				if (spawnY >= 0) {
					m_playerSpawnPoint.Y = spawnY;
				}
			}
		}
		else if (inTilesSection && tileRowIndex < MAP_HEIGHT) {
			// 타일 데이터 파싱 (SaveMap에서 타입,ID,타입,ID... 형식으로 저장됨)
			std::wstringstream ss(line);
			std::wstring token;
			std::vector<std::wstring> tokens;

			// 모든 토큰을 벡터에 저장
			while (std::getline(ss, token, L',')) {
				tokens.push_back(token);
			}

			// 2개씩 묶어서 처리 (타입, ID)
			for (int tileX = 0; tileX < MAP_WIDTH && (tileX * 2 + 1) < tokens.size(); ++tileX) {
				std::wstring tileTypeStr = tokens[tileX * 2];
				std::wstring tileIdStr = tokens[tileX * 2 + 1];

				TileType tileType = EnumUtils::GetEnumValue<TileType>(tileTypeStr.c_str(), TILE_NONE);
				TileID tileId = EnumUtils::GetEnumValue<TileID>(tileIdStr.c_str(), TILEID_NONE);

				if (tileType != TILE_NONE && tileId != TILEID_NONE) {
					const TileVariant* tv = GetTileVariant(tileType, tileId);
					if (tv) {
						m_tileMap[tileRowIndex][tileX] = TileData(tileType, tileId, tv->pAtlasBitmap, tv->sourceRect);
					}
				}
			}
			tileRowIndex++;
		}
		else if (inObjectsSection) {
			// 오브젝트 데이터 파싱 (2줄씩 처리)
			std::wstringstream objStream(line);
			std::wstring field1, field2, field3, field4, field5, field6, field7;

			std::getline(objStream, field1, L',');  // GameObjectType
			std::getline(objStream, field2, L',');  // GameObjectID
			std::getline(objStream, field3, L',');  // x
			std::getline(objStream, field4, L',');  // y
			std::getline(objStream, field5, L',');  // objectAssetBaseDirectory
			std::getline(objStream, field6, L',');  // pivotX
			std::getline(objStream, field7);        // pivotY

			if (!field1.empty() && !field2.empty()) {
				GameObjectType objType = EnumUtils::GetEnumValue<GameObjectType>(field1.c_str(), GOBJ_NONE);
				GameObjectID objId = EnumUtils::GetEnumValue<GameObjectID>(field2.c_str(), GOID_NONE);

				if (objType != GOBJ_NONE && objId != GOID_NONE) {
					float x = std::stof(field3);
					float y = std::stof(field4);
					float pivotX = std::stof(field6);
					float pivotY = std::stof(field7);

					GameObjectData newObj(objType, objId, x, y, field5, pivotX, pivotY);

					// 다음 줄에서 콜라이더 정보 읽기
					if (std::getline(inFile, line)) {
						std::wstringstream colliderStream(line);
						std::wstring hasColliderStr, offsetXStr, offsetYStr, widthStr, heightStr;

						std::getline(colliderStream, hasColliderStr, L',');
						std::getline(colliderStream, offsetXStr, L',');
						std::getline(colliderStream, offsetYStr, L',');
						std::getline(colliderStream, widthStr, L',');
						std::getline(colliderStream, heightStr);

						if (!hasColliderStr.empty()) {
							newObj.hasCollider = (hasColliderStr == L"1");
							if (newObj.hasCollider) {
								newObj.colliderOffsetX = std::stoi(offsetXStr);
								newObj.colliderOffsetY = std::stoi(offsetYStr);
								newObj.colliderWidth = std::stoi(widthStr);
								newObj.colliderHeight = std::stoi(heightStr);
							}
						}
					}

					m_gameObjects.push_back(newObj);
				}
			}
			else if (inWalkableAreasSection && walkableRowIndex < MAP_HEIGHT) {
				// Walkable Areas 데이터 파싱
				std::wstringstream ss(line);
				std::wstring token;
				std::vector<std::wstring> tokens;

				// 모든 토큰을 벡터에 저장
				while (std::getline(ss, token, L',')) {
					tokens.push_back(token);
				}

				// 각 타일의 walkable 상태 설정
				for (int x = 0; x < MAP_WIDTH && x < tokens.size(); ++x) {
					m_walkableAreaMap[walkableRowIndex][x] = (tokens[x] == L"1");
				}
				walkableRowIndex++;
			}
		}
	}

	inFile.close();

	// 로드 완료 후 플레이어 스폰 포인트 검증 (설정되지 않은 경우 중앙으로 설정)
	if (!m_hasPlayerSpawn) {
		float centerX = (MAP_WIDTH / 2.0f) * TILE_SIZE;
		float centerY = (MAP_HEIGHT / 2.0f) * TILE_SIZE;
		m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
		m_hasPlayerSpawn = true;

		std::wstringstream debugSS;
		debugSS << L"No spawn point in loaded map. Set to center: (" << (int)centerX << L", " << (int)centerY << L")\n";
		OutputDebugStringW(debugSS.str().c_str());
	}

	// 로드 완료 후 화면 갱신
	m_tileLayerDirty = true;
	m_objectLayerDirty = true;
	m_objectsDirty = true;

	// 로드된 walkable areas 정보 디버그 출력
	int walkableCount = 0;
	int blockedCount = 0;
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			if (m_walkableAreaMap[y][x]) walkableCount++;
			else blockedCount++;
		}
	}

	std::wstringstream debugSS;
	debugSS << L"Walkable Areas loaded - Walkable: " << walkableCount
		<< L", Blocked: " << blockedCount << L" tiles\n";
	OutputDebugStringW(debugSS.str().c_str());

	OutputDebugStringW(L"맵 불러오기 완료: ");
	OutputDebugStringW(filename);
	OutputDebugStringW(L"\n");
	return true;
}

// 새 맵 생성 (맵 초기화)
void DontStarve_EditorMain::NewMap() {
	// 기존 맵 데이터 초기화
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_tileMap[y][x] = TileData();  // 빈 타일로 초기화
		}
	}

	// 모든 오브젝트 제거
	m_gameObjects.clear();
	m_selectedObjectPtr = nullptr;

	// 편집 모드들 해제
	m_isPlacingMode = false;
	m_isPivotEditMode = false;
	m_isColliderEditMode = false;
	m_isPlayerSpawnMode = false;
	m_isWalkableEditMode = false;
	m_isDraggingWalkable = false;
	m_isDraggingCamera = false;
	m_selectedPaletteIndex = -1;
	m_editingObject = nullptr;
	m_editingColliderObject = nullptr;

	// 마우스 캡처 해제 (혹시나 남아있을 수 있는 캡처 상태 정리)
	ReleaseCapture();

	// 서브팔레트 닫기
	m_subPalette.isOpen = false;
	m_subPalette.selectedTileVariantIndex = -1;
	m_subPalette.selectedObjectVariantIndex = -1;

	// 플레이어 스폰 포인트를 맵 중앙으로 설정
	float centerX = (MAP_WIDTH / 2.0f) * TILE_SIZE;  // 25 * 128 = 3200px
	float centerY = (MAP_HEIGHT / 2.0f) * TILE_SIZE; // 25 * 128 = 3200px
	m_playerSpawnPoint = Gdiplus::PointF(centerX, centerY);
	m_hasPlayerSpawn = true;

	// walkable area map 초기화 (기본적으로 모든 영역이 walkable)
	for (int y = 0; y < MAP_HEIGHT; ++y) {
		for (int x = 0; x < MAP_WIDTH; ++x) {
			m_walkableAreaMap[y][x] = true;
		}
	}

	// 맵 뷰 리셋 (줌과 오프셋 초기화)
	m_zoomFactor = 1.0f;
	m_mapOffset = { 0, 0 };

	// 모든 레이어 다시 그리기
	m_tileLayerDirty = true;
	m_objectLayerDirty = true;
	m_objectsDirty = true;
	m_paletteLayerDirty = true;
	m_gridLayerDirty = true;

	std::wstringstream debugSS;
	debugSS << L"New Map Created - Player spawn at center: (" << (int)centerX << L", " << (int)centerY
		<< L")px = Tile(" << (MAP_WIDTH / 2) << L", " << (MAP_HEIGHT / 2) << L")\n";
	OutputDebugStringW(debugSS.str().c_str());
}

// 저장 파일 다이얼로그 표시
bool DontStarve_EditorMain::ShowSaveFileDialog(WCHAR* fileName, DWORD fileNameSize) {
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = fileNameSize;
	ofn.lpstrFilter = L"Map Files (*.dsm)\0*.dsm\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"맵 저장";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_OVERWRITEPROMPT;
	ofn.lpstrDefExt = L"dsm";

	return GetSaveFileName(&ofn) != 0;
}

// 열기 파일 다이얼로그 표시
bool DontStarve_EditorMain::ShowOpenFileDialog(WCHAR* fileName, DWORD fileNameSize) {
	OPENFILENAME ofn = {};
	ofn.lStructSize = sizeof(ofn);
	ofn.hwndOwner = g_hWnd;
	ofn.lpstrFile = fileName;
	ofn.nMaxFile = fileNameSize;
	ofn.lpstrFilter = L"Map Files (*.dsm)\0*.dsm\0All Files (*.*)\0*.*\0";
	ofn.nFilterIndex = 1;
	ofn.lpstrFileTitle = NULL;
	ofn.nMaxFileTitle = 0;
	ofn.lpstrInitialDir = NULL;
	ofn.lpstrTitle = L"맵 불러오기";
	ofn.Flags = OFN_PATHMUSTEXIST | OFN_FILEMUSTEXIST;

	return GetOpenFileName(&ofn) != 0;
}

// 리소스 로드 -> 아틀라스 생성 포함
void DontStarve_EditorMain::LoadResources()
{
	// 기존 리소스 클리어 (아틀라스 비트맵도 해제됨)
	ReleaseResources();

	m_tileVariants.clear();
	m_objectVariants.clear();
	m_tileAtlasBitmapOwner.reset();
	m_objectAtlasBitmapOwner.reset();

	struct TempImageInfo_Local {
		std::unique_ptr<Gdiplus::Bitmap> pRawBitmap;
		TileType tileType = TILE_NONE;       // Tile section
		TileID tileId = TILEID_NONE;         // Tile section
		GameObjectType objectType = GOBJ_NONE; // Object section
		GameObjectID gameObjectId = GOID_NONE; // Object section
		std::wstring baseDirectory;           // base directory part (e.g. "Resource/Tiles/Dirt")
		std::wstring imageFileName;           // file name part (e.g. "dirt_01.png")
		float pivotX = 0.5f;                  // Default pivot for objects
		float pivotY = 1.0f;                  // Default pivot for objects
	};

	std::vector<TempImageInfo_Local> tempTilesInfo;
	std::vector<TempImageInfo_Local> tempObjectsInfo;

	std::wifstream inFile(L"../Resource/resources.txt");
	if (!inFile.is_open()) {
		OutputDebugStringW(L"Error: Failed to open resources.txt. Make sure it's in the same directory as the executable.\n");
		return;
	}
	OutputDebugStringW(L"=== Loading resources.txt ===\n");

	std::wstring line;
	bool inTilesSection = false;
	bool inObjectsSection = false;

	// Parse resources.txt line by line
	while (std::getline(inFile, line)) {
		if (line.empty() || line[0] == L'#') { // Skip empty lines and comment lines (starting with #)
			if (line == L"# TILES") { inTilesSection = true; inObjectsSection = false; }
			else if (line == L"# OBJECTS") { inTilesSection = false; inObjectsSection = true; }
			continue; // Process next line
		}

		std::wstringstream ss(line);
		std::wstring field1, field2, field3, field4;

		if (inTilesSection) { // Processing #TILES section
			std::getline(ss, field1, L','); // Field 1: TileType (e.g., TILE_DIRT)
			std::getline(ss, field2, L','); // Field 2: TileID (e.g., TILEID_DIRT_00)
			std::getline(ss, field3, L','); // Field 3: baseDirectory (e.g., Resource/Tiles/Dirt)
			std::getline(ss, field4);       // Field 4: imageFileName (e.g., dirt_01.png)

			TileType type = EnumUtils::GetEnumValue<TileType>(field1.c_str(), TILE_NONE);
			TileID id = EnumUtils::GetEnumValue<TileID>(field2.c_str(), TILEID_NONE);
			std::wstring fullImagePath = L"../" + field3 + L"/" + field4; // Construct full image path
			Gdiplus::Bitmap* pBitmapRaw = BitmapUtils::LoadBitmapFromFile(fullImagePath.c_str()); // Load raw bitmap

			// Validate loaded bitmap and parsed enums
			if (type != TILE_NONE && id != TILEID_NONE && pBitmapRaw && pBitmapRaw->GetLastStatus() == Gdiplus::Ok) {
				TempImageInfo_Local info; info.pRawBitmap = std::unique_ptr<Gdiplus::Bitmap>(pBitmapRaw); // Transfer ownership to unique_ptr
				info.tileType = type;
				info.tileId = id;
				info.baseDirectory = field3;
				info.imageFileName = field4;
				tempTilesInfo.push_back(std::move(info)); // Add to temporary list for atlas creation
			}
			else { OutputDebugStringW((L"Error: Failed to load tile bitmap or parse enum for: " + fullImagePath + L"\n").c_str()); }
		}
		else if (inObjectsSection) { // Processing #OBJECTS section
			std::getline(ss, field1, L','); // Field 1: GameObjectType (e.g., GOBJ_ITEM)
			std::getline(ss, field2, L','); // Field 2: GameObjectID (e.g., GOID_ITEM_CUT_NORMAL_GRASS)
			std::getline(ss, field3, L','); // Field 3: objectAssetBaseDirectory (e.g., Resource/Objects/ingredient)
			std::getline(ss, field4);       // Field 4: editorDisplayFileName (e.g., cutgrass01-0.png)


			GameObjectType type = EnumUtils::GetEnumValue<GameObjectType>(field1.c_str(), GOBJ_NONE);
			GameObjectID id = EnumUtils::GetEnumValue<GameObjectID>(field2.c_str(), GOID_NONE);
			std::wstring editorDisplayImagePath = L"../" + field3 + L"/" + field4; // Construct full image path

			Gdiplus::Bitmap* pBitmapRaw = BitmapUtils::LoadBitmapFromFile(editorDisplayImagePath.c_str());

			if (type != GOBJ_NONE && id != GOID_NONE && pBitmapRaw && pBitmapRaw->GetLastStatus() == Gdiplus::Ok) {
				TempImageInfo_Local info; info.pRawBitmap = std::unique_ptr<Gdiplus::Bitmap>(pBitmapRaw);
				info.objectType = type;
				info.gameObjectId = id;
				info.baseDirectory = field3; // Maps to ObjectVariant's objectAssetBaseDirectory
				info.imageFileName = field4; // Maps to ObjectVariant's editorDisplayFileName
				// pivotX, pivotY default to 0.5, 1.0 from TempImageInfo_Local
				tempObjectsInfo.push_back(std::move(info)); // Add to temporary list
			}
			else { OutputDebugStringW((L"Error: Failed to load object bitmap or parse enum for: " + editorDisplayImagePath + L"\n").c_str()); }
		}
	}
	inFile.close();

	// ----------------------------------------------------------------------------------------------------
	// Create Atlases and populate m_tileVariants / m_objectVariants maps from temp info
	// ----------------------------------------------------------------------------------------------------

	// 2.1. Tile Atlas Generation
	UINT tileAtlasWidth = 0; UINT maxTileAtlasHeight = 0;
	for (const auto& info : tempTilesInfo) { // Determine total width and max height needed for the atlas
		if (info.pRawBitmap) {
			tileAtlasWidth += info.pRawBitmap->GetWidth();
			if (info.pRawBitmap->GetHeight() > maxTileAtlasHeight) maxTileAtlasHeight = info.pRawBitmap->GetHeight();
		}
	}
	if (tileAtlasWidth > 0 && maxTileAtlasHeight > 0) {
		m_tileAtlasBitmapOwner = std::make_unique<Gdiplus::Bitmap>(tileAtlasWidth, maxTileAtlasHeight, PixelFormat32bppARGB);
		Gdiplus::Graphics tileAtlasGraphics(m_tileAtlasBitmapOwner.get()); // Get raw pointer for GDI+ drawing
		tileAtlasGraphics.Clear(Gdiplus::Color(0, 0, 0, 0)); // Clear with transparent background

		float drawCursorX = 0.0f; // Current X position on the atlas to draw the next bitmap
		for (auto& info : tempTilesInfo) { // Draw individual bitmaps onto the atlas
			if (info.pRawBitmap) {
				Gdiplus::RectF calculatedSrcRect(drawCursorX, 0.0f, (float)info.pRawBitmap->GetWidth(), (float)info.pRawBitmap->GetHeight());
				tileAtlasGraphics.DrawImage(info.pRawBitmap.get(), calculatedSrcRect); // Draw raw bitmap onto the atlas

				// Populate m_tileVariants map (Indexed by TileType, then TileID)
				m_tileVariants[info.tileType][info.tileId] =
					TileVariant(m_tileAtlasBitmapOwner.get(), calculatedSrcRect, // Pass raw atlas bitmap pointer and sourceRect
						info.tileType, info.tileId, info.baseDirectory, info.imageFileName);
				drawCursorX += info.pRawBitmap->GetWidth(); // Advance cursor for the next image
			}
		}
	}

	// 아틀라스 비트맵
	UINT objectAtlasWidth = 0; UINT maxObjectAtlasHeight = 0;
	for (const auto& info : tempObjectsInfo) {
		if (info.pRawBitmap) {
			objectAtlasWidth += info.pRawBitmap->GetWidth();
			if (info.pRawBitmap->GetHeight() > maxObjectAtlasHeight) maxObjectAtlasHeight = info.pRawBitmap->GetHeight();
		}
	}
	if (objectAtlasWidth > 0 && maxObjectAtlasHeight > 0) {
		m_objectAtlasBitmapOwner = std::make_unique<Gdiplus::Bitmap>(objectAtlasWidth, maxObjectAtlasHeight, PixelFormat32bppARGB);
		Gdiplus::Graphics objectAtlasGraphics(m_objectAtlasBitmapOwner.get());
		objectAtlasGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

		float drawCursorX = 0.0f;
		for (auto& info : tempObjectsInfo) {
			if (info.pRawBitmap) {
				Gdiplus::RectF calculatedSrcRect(drawCursorX, 0.0f, (float)info.pRawBitmap->GetWidth(), (float)info.pRawBitmap->GetHeight());
				objectAtlasGraphics.DrawImage(info.pRawBitmap.get(), calculatedSrcRect);

				// m_objectVariants (Indexed -> GameObjectType, then GameObjectID)
				m_objectVariants[info.objectType][info.gameObjectId] =
					ObjectVariant(m_objectAtlasBitmapOwner.get(), calculatedSrcRect,
						info.objectType, info.gameObjectId,
						info.baseDirectory, info.imageFileName, // baseDirectory , editorDisplayFileName
						info.pivotX, info.pivotY);
				drawCursorX += info.pRawBitmap->GetWidth();
			}
		}
	}

	OutputDebugStringW((L"Final Tile Variants Map Size: " + std::to_wstring(m_tileVariants.size()) + L"\n").c_str());
	OutputDebugStringW((L"Final Object Variants Map Size: " + std::to_wstring(m_objectVariants.size()) + L"\n").c_str());

}

void DontStarve_EditorMain::ReleaseResources()
{
	m_tileVariants.clear();
	m_objectVariants.clear();
	m_tileAtlasBitmapOwner.reset();
	m_objectAtlasBitmapOwner.reset();
}

// 팔레트 초기화 (GameObjectType 및 ObjectVariant 반영)
void DontStarve_EditorMain::InitPalette()
{
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	int paletteWidth = 140;
	int paletteHeight = clientRect.bottom;

	m_paletteRect = {
		clientRect.right - paletteWidth,
		0,
		clientRect.right,
		paletteHeight
	};

	if (m_paletteLayerBitmap) {
		SafeDelete(m_paletteLayerBitmap);
	}
	m_paletteLayerBitmap = new Gdiplus::Bitmap(m_paletteRect.right - m_paletteRect.left, m_paletteRect.bottom - m_paletteRect.top, PixelFormat32bppARGB);
	if (!m_paletteLayerBitmap) { return; }

	m_paletteItems.clear();

	int itemSize = 64;
	int padding = 5;

	int currentY = m_paletteRect.top + padding;

	// 1. Add Tile Categories to Main Palette (indexed by TileType)
	for (int i = 0; i < TILE_COUNT; ++i) {
		TileType type = (TileType)i;
		if (type == TILE_NONE || type == TILE_COUNT) continue;

		auto type_map_it = m_tileVariants.find(type);
		if (type_map_it != m_tileVariants.end() && !type_map_it->second.empty()) {
			const TileVariant& sampleVariant = type_map_it->second.begin()->second;
			Gdiplus::Bitmap* iconAtlasBmp = sampleVariant.pAtlasBitmap;
			Gdiplus::RectF iconSrcRect = sampleVariant.sourceRect;

			RECT itemRect = { m_paletteRect.left + padding, currentY, m_paletteRect.left + padding + itemSize, currentY + itemSize };
			m_paletteItems.push_back({ CATEGORY_TILE, (int)type, (UINT)type, itemRect, iconAtlasBmp, iconSrcRect });
			currentY += itemSize + padding;
		}
	}

	// 2. Add Object Categories to Main Palette (indexed by GameObjectType)
	for (int i = 0; i < GOBJ_COUNT; ++i) {
		GameObjectType type = (GameObjectType)i;
		if (type == GOBJ_NONE || type == GOBJ_COUNT) continue;

		auto type_map_it = m_objectVariants.find(type);
		if (type_map_it != m_objectVariants.end() && !type_map_it->second.empty()) {
			const ObjectVariant& sampleVariant = type_map_it->second.begin()->second;
			Gdiplus::Bitmap* iconAtlasBmp = sampleVariant.pAtlasBitmap;
			Gdiplus::RectF iconSrcRect = sampleVariant.sourceRect;

			RECT itemRect = { m_paletteRect.left + padding, currentY, m_paletteRect.left + padding + itemSize, currentY + itemSize };
			m_paletteItems.push_back({ CATEGORY_OBJECT, (int)type, (UINT)type, itemRect, iconAtlasBmp, iconSrcRect });
			currentY += itemSize + padding;
		}
	}

	// Set initial selected item if any palette items were added
	if (!m_paletteItems.empty()) {
		m_selectedPaletteIndex = 0;
	}

	m_paletteLayerDirty = true; // 팔레트 아이템이 채워졌으므로 레이어 다시 그리기
}

// 뷰포트 기반 그리드 렌더링 (성능 최적화)
void DontStarve_EditorMain::DrawGrid(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;

	// 그리드 펜 설정
	Gdiplus::Pen gridPen(Gdiplus::Color(120, 150, 150, 150), 1.0f);        // 일반 그리드 선
	Gdiplus::Pen majorGridPen(Gdiplus::Color(180, 100, 100, 100), 1.5f);   // 10타일마다 굵은 선
	Gdiplus::Pen mapBoundaryPen(Gdiplus::Color(255, 255, 0, 0), 3.0f);     // 맵 경계선

	// 줌 팩터가 적용된 화면상 타일 크기
	float screenTileSize = (float)TILE_SIZE * m_zoomFactor;

	// 그리드 선이 너무 작을 때는 건너뛰기 (성능 최적화)
	if (screenTileSize < 6.0f) {
		return; // 매우 작은 그리드는 그리지 않음
	}

	// 레이어 비트맵 크기 (현재 뷰포트 크기)
	UINT layerWidth = m_gridLayerBitmap->GetWidth();
	UINT layerHeight = m_gridLayerBitmap->GetHeight();

	// 현재 뷰포트의 월드 좌표 계산
	Gdiplus::PointF viewTopLeft = ScreenToWorld(Gdiplus::PointF(0, 0));

	// 그리드 간격 최적화 (줌 레벨에 따라)
	int gridSpacing = TILE_SIZE;
	if (screenTileSize < 16.0f) gridSpacing *= 4;      // 매우 작을 때는 4칸마다
	else if (screenTileSize < 32.0f) gridSpacing *= 2;  // 작을 때는 2칸마다

	// 시작 그리드 인덱스 계산
	int startGridX = (int)floor(viewTopLeft.X / gridSpacing);
	int startGridY = (int)floor(viewTopLeft.Y / gridSpacing);

	// 화면에 그려질 그리드 개수 계산
	int maxGridLines = max(layerWidth / max(8, (int)screenTileSize), layerHeight / max(8, (int)screenTileSize));
	maxGridLines = min(maxGridLines, 100); // 최대 100개로 제한

	// 세로선 그리기
	for (int i = 0; i <= maxGridLines; ++i) {
		int gridX = startGridX + i;
		if (gridX < 0 || gridX > MAP_WIDTH) continue;

		float worldX = (float)(gridX * gridSpacing);
		float screenX = (worldX - viewTopLeft.X) * m_zoomFactor;

		if (screenX >= 0 && screenX <= layerWidth) {
			bool isMajor = (gridX % 10 == 0) && (gridX != 0) && (gridX != MAP_WIDTH);
			Gdiplus::Pen* currentPen = isMajor ? &majorGridPen : &gridPen;

			pGraphics->DrawLine(currentPen, screenX, 0.0f, screenX, (float)layerHeight);
		}
	}

	// 가로선 그리기
	for (int i = 0; i <= maxGridLines; ++i) {
		int gridY = startGridY + i;
		if (gridY < 0 || gridY > MAP_HEIGHT) continue;

		float worldY = (float)(gridY * gridSpacing);
		float screenY = (worldY - viewTopLeft.Y) * m_zoomFactor;

		if (screenY >= 0 && screenY <= layerHeight) {
			bool isMajor = (gridY % 10 == 0) && (gridY != 0) && (gridY != MAP_HEIGHT);
			Gdiplus::Pen* currentPen = isMajor ? &majorGridPen : &gridPen;

			pGraphics->DrawLine(currentPen, 0.0f, screenY, (float)layerWidth, screenY);
		}
	}

	// 맵 경계선 그리기 (뷰포트에 보이는 경우에만)
	float mapLeftScreen = (0 - viewTopLeft.X) * m_zoomFactor;
	float mapTopScreen = (0 - viewTopLeft.Y) * m_zoomFactor;
	float mapWidthScreen = (MAP_WIDTH * TILE_SIZE) * m_zoomFactor;
	float mapHeightScreen = (MAP_HEIGHT * TILE_SIZE) * m_zoomFactor;

	if (mapLeftScreen < layerWidth && mapTopScreen < layerHeight &&
		mapLeftScreen + mapWidthScreen > 0 && mapTopScreen + mapHeightScreen > 0) {

		Gdiplus::RectF mapRect(mapLeftScreen, mapTopScreen, mapWidthScreen, mapHeightScreen);
		pGraphics->DrawRectangle(&mapBoundaryPen, mapRect);
	}
}

void DontStarve_EditorMain::DrawTileMap(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;

	// 레이어 비트맵 크기
	UINT layerWidth = m_tileLayerBitmap->GetWidth();
	UINT layerHeight = m_tileLayerBitmap->GetHeight();

	// 현재 뷰포트의 월드 좌표 계산
	Gdiplus::PointF viewTopLeft = ScreenToWorld(Gdiplus::PointF(0, 0));
	Gdiplus::PointF viewBottomRight = ScreenToWorld(Gdiplus::PointF((float)layerWidth, (float)layerHeight));

	// 타일 인덱스 범위 계산 (뷰포트에 보이는 타일만)
	int startX = max(0, (int)floor(viewTopLeft.X / TILE_SIZE));
	int endX = min(MAP_WIDTH, (int)ceil(viewBottomRight.X / TILE_SIZE));
	int startY = max(0, (int)floor(viewTopLeft.Y / TILE_SIZE));
	int endY = min(MAP_HEIGHT, (int)ceil(viewBottomRight.Y / TILE_SIZE));

	// 화면상 타일 크기
	float screenTileSize = (float)TILE_SIZE * m_zoomFactor;

	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			TileData tile = m_tileMap[y][x];
			if (tile.type == TILE_NONE || !tile.pAtlasBitmap) continue;

			// 월드 좌표 계산
			float worldX = (float)x * TILE_SIZE;
			float worldY = (float)y * TILE_SIZE;

			// 뷰포트 기준 화면 좌표 계산
			float screenX = (worldX - viewTopLeft.X) * m_zoomFactor;
			float screenY = (worldY - viewTopLeft.Y) * m_zoomFactor;

			// 화면 밖 컬링
			if (screenX + screenTileSize < 0 || screenX > layerWidth ||
				screenY + screenTileSize < 0 || screenY > layerHeight) continue;

			Gdiplus::RectF destRect(screenX, screenY, screenTileSize, screenTileSize);

			pGraphics->DrawImage(tile.pAtlasBitmap, destRect,
				tile.sourceRect.X, tile.sourceRect.Y,
				tile.sourceRect.Width, tile.sourceRect.Height,
				Gdiplus::UnitPixel);
		}
	}
}

void DontStarve_EditorMain::DrawObjects(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;

	// Perform Y-sorting for rendering depth (only if data changed)
	if (m_objectsDirty) {
		m_sortedObjects.clear();
		for (const auto& obj : m_gameObjects) {
			m_sortedObjects.push_back(&obj);
		}
		std::sort(m_sortedObjects.begin(), m_sortedObjects.end(),
			[](const GameObjectData* a, const GameObjectData* b) {
				return a->y < b->y; // Y-sort based on bottom-center Y position
			});
		m_objectsDirty = false;
	}

	// 레이어 비트맵 크기
	UINT layerWidth = m_objectLayerBitmap->GetWidth();
	UINT layerHeight = m_objectLayerBitmap->GetHeight();

	// 현재 뷰포트의 월드 좌표 계산
	Gdiplus::PointF viewTopLeft = ScreenToWorld(Gdiplus::PointF(0, 0));
	Gdiplus::PointF viewBottomRight = ScreenToWorld(Gdiplus::PointF((float)layerWidth, (float)layerHeight));

	// 뷰포트 컬링을 위한 월드 영역 (여유 공간 포함)
	const float CULL_MARGIN = 100.0f;
	Gdiplus::RectF viewWorldRect(
		viewTopLeft.X - CULL_MARGIN, viewTopLeft.Y - CULL_MARGIN,
		(viewBottomRight.X - viewTopLeft.X) + 2 * CULL_MARGIN,
		(viewBottomRight.Y - viewTopLeft.Y) + 2 * CULL_MARGIN
	);

	// Draw objects in sorted order
	for (const GameObjectData* objData : m_sortedObjects) {
		// Retrieve ObjectVariant definition
		const ObjectVariant* ov = GetObjectVariant(objData->type, objData->id);
		if (!ov || !ov->pAtlasBitmap) continue;

		// Calculate object's world bounding box
		float objRenderLeftWorld = (float)objData->x - (ov->pivotX * ov->sourceRect.Width);
		float objRenderTopWorld = (float)objData->y - (ov->pivotY * ov->sourceRect.Height);
		float objRenderWidthWorld = (float)ov->sourceRect.Width;
		float objRenderHeightWorld = (float)ov->sourceRect.Height;

		Gdiplus::RectF objWorldRect(objRenderLeftWorld, objRenderTopWorld, objRenderWidthWorld, objRenderHeightWorld);

		// 뷰포트 컬링
		if (!objWorldRect.IntersectsWith(viewWorldRect)) {
			continue; // 화면 밖에 있으면 스킵
		}

		// 뷰포트 기준 화면 좌표 계산
		float screenX = (objRenderLeftWorld - viewTopLeft.X) * m_zoomFactor;
		float screenY = (objRenderTopWorld - viewTopLeft.Y) * m_zoomFactor;
		float screenWidth = objRenderWidthWorld * m_zoomFactor;
		float screenHeight = objRenderHeightWorld * m_zoomFactor;

		// 추가 화면 컬링
		if (screenX + screenWidth < 0 || screenX > layerWidth ||
			screenY + screenHeight < 0 || screenY > layerHeight) continue;

		Gdiplus::RectF destRect(screenX, screenY, screenWidth, screenHeight);

		pGraphics->DrawImage(ov->pAtlasBitmap, destRect,
			ov->sourceRect.X, ov->sourceRect.Y,
			ov->sourceRect.Width, ov->sourceRect.Height,
			Gdiplus::UnitPixel);

		// Drawing selected object highlight
		if (m_selectedObjectPtr == objData) {
			Gdiplus::Pen selectedPen(Gdiplus::Color(255, 255, 0, 0), 3.0f);
			pGraphics->DrawRectangle(&selectedPen, destRect);
		}
	}
}

void DontStarve_EditorMain::DrawPalette(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;

	// 팔레트 배경 그리기 (이 레이어 비트맵의 0,0을 기준으로 그립니다.)
	Gdiplus::SolidBrush paletteBackgroundBrush(Gdiplus::Color(100, 50, 50, 50));
	pGraphics->FillRectangle(&paletteBackgroundBrush,
		(Gdiplus::REAL)0, (Gdiplus::REAL)0,
		(Gdiplus::REAL)(m_paletteRect.right - m_paletteRect.left), (Gdiplus::REAL)(m_paletteRect.bottom - m_paletteRect.top));

	for (size_t i = 0; i < m_paletteItems.size(); ++i) {
		const PaletteItem& item = m_paletteItems[i];

		// 아이콘 배경
		Gdiplus::SolidBrush itemBackgroundBrush(Gdiplus::Color(100, 70, 70, 70));
		pGraphics->FillRectangle(&itemBackgroundBrush,
			(Gdiplus::REAL)(item.displayRect.left - m_paletteRect.left), // 팔레트 레이어 비트맵 기준 상대 좌표
			(Gdiplus::REAL)(item.displayRect.top - m_paletteRect.top),  // 팔레트 레이어 비트맵 기준 상대 좌표
			(Gdiplus::REAL)(item.displayRect.right - item.displayRect.left), (Gdiplus::REAL)(item.displayRect.bottom - item.displayRect.top));

		// 아이콘 이미지 그리기 
		if (item.hBitmap && item.hBitmap->GetLastStatus() == Gdiplus::Ok) {
			pGraphics->DrawImage(item.hBitmap,
				// 대상 사각형: 팔레트 레이어 비트맵 기준 상대 좌표
				Gdiplus::RectF((Gdiplus::REAL)(item.displayRect.left - m_paletteRect.left), (Gdiplus::REAL)(item.displayRect.top - m_paletteRect.top),
					(Gdiplus::REAL)(item.displayRect.right - item.displayRect.left), (Gdiplus::REAL)(item.displayRect.bottom - item.displayRect.top)),
				item.iconSourceRect.X, item.iconSourceRect.Y, item.iconSourceRect.Width, item.iconSourceRect.Height,
				Gdiplus::UnitPixel);
		}

		// 선택된 아이템 강조
		if ((int)i == m_selectedPaletteIndex) {
			Gdiplus::Pen highlightPen(Gdiplus::Color(255, 255, 255, 0), 3.0f);
			pGraphics->DrawRectangle(&highlightPen,
				(Gdiplus::REAL)(item.displayRect.left - m_paletteRect.left), (Gdiplus::REAL)(item.displayRect.top - m_paletteRect.top),
				(Gdiplus::REAL)(item.displayRect.right - item.displayRect.left), (Gdiplus::REAL)(item.displayRect.bottom - item.displayRect.top));
		}
	}
}

void DontStarve_EditorMain::ComposeTileLayer() {
	if (!m_tileLayerDirty || !m_tileLayerBitmap) return;

	Gdiplus::Graphics tileLayerGraphics(m_tileLayerBitmap);
	tileLayerGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

	DrawTileMap(&tileLayerGraphics);

	m_tileLayerDirty = false;

	OutputDebugStringW(L"Composing Tile Layer: END (Dirty set to false)\n");
}


void DontStarve_EditorMain::ComposeObjectLayer() {
	if (!m_objectLayerDirty || !m_objectLayerBitmap) return;

	Gdiplus::Graphics objectLayerGraphics(m_objectLayerBitmap);
	objectLayerGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

	DrawObjects(&objectLayerGraphics);

	m_objectLayerDirty = false;
	OutputDebugStringW(L"Composing Object Layer: END (Dirty set to false)\n");
}

// DrawPreview (배치 프리뷰 그리기 - 투명하고 그리드 크기에 맞춤)
void DontStarve_EditorMain::DrawPreview(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || m_selectedPaletteIndex == -1 || !m_isPlacingMode) return;

	const PaletteItem& selectedItem = m_paletteItems[m_selectedPaletteIndex];
	Gdiplus::Bitmap* previewBitmap = nullptr;
	Gdiplus::RectF previewSourceRect;

	// 프리뷰로 그릴 타일 또는 오브젝트의 ResourceVariant를 가져옴
	if (selectedItem.category == CATEGORY_TILE) {
		const TileVariant* tv = m_subPalette.getSelectedTileVariant();
		if (tv) {
			previewBitmap = tv->pAtlasBitmap;
			previewSourceRect = tv->sourceRect;
		}
	}
	else if (selectedItem.category == CATEGORY_OBJECT) {
		const ObjectVariant* ov = m_subPalette.getSelectedObjectVariant();
		if (ov) {
			previewBitmap = ov->pAtlasBitmap;
			previewSourceRect = ov->sourceRect;
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
			finalRenderWidth = (float)TILE_SIZE * m_zoomFactor;
			finalRenderHeight = (float)TILE_SIZE * m_zoomFactor;
			finalRenderX = screenPreviewPos.X;
			finalRenderY = screenPreviewPos.Y;

			// 타일 프리뷰 배경 (그리드 영역 표시)
			Gdiplus::Pen previewGridPen(Gdiplus::Color(150, 255, 255, 0), 2.0f);
			Gdiplus::RectF previewGridRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
			pGraphics->DrawRectangle(&previewGridPen, previewGridRect);

			// 3x3 모드일 때 추가 그리드 표시
			if (m_is3x3Mode) {
				Gdiplus::Pen gridPen3x3(Gdiplus::Color(100, 255, 255, 0), 1.5f);
				float tileSize = (float)TILE_SIZE * m_zoomFactor;

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
			const ObjectVariant* ov_preview = m_subPalette.getSelectedObjectVariant();
			if (ov_preview) {
				finalRenderWidth = previewSourceRect.Width * m_zoomFactor;
				finalRenderHeight = previewSourceRect.Height * m_zoomFactor;
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
		Gdiplus::RectF destRect(finalRenderX, finalRenderY, finalRenderWidth, finalRenderHeight);
		pGraphics->DrawImage(previewBitmap, destRect,
			previewSourceRect.X, previewSourceRect.Y, previewSourceRect.Width, previewSourceRect.Height,
			Gdiplus::UnitPixel, &imageAttr);

		// 프리뷰 정보 텍스트 (마우스 근처에 표시)
		Gdiplus::Font infoFont(L"Arial", 10);
		Gdiplus::SolidBrush infoBrush(Gdiplus::Color(255, 255, 255, 255));
		Gdiplus::SolidBrush infoBackBrush(Gdiplus::Color(150, 0, 0, 0));

		std::wstringstream infoSS;
		if (selectedItem.category == CATEGORY_TILE) {
			float screenTileSize = (float)TILE_SIZE * m_zoomFactor;
			infoSS << L"Tile Preview: " << (int)screenTileSize << L"px (World: " << TILE_SIZE << L"px)";
			if (m_is3x3Mode) infoSS << L" [3x3 Mode]";
		}
		else {
			float screenWidth = previewSourceRect.Width * m_zoomFactor;
			float screenHeight = previewSourceRect.Height * m_zoomFactor;
			infoSS << L"Object Preview: " << (int)screenWidth << L"x" << (int)screenHeight
				<< L"px (World: " << (int)previewSourceRect.Width << L"x" << (int)previewSourceRect.Height << L"px)";
		}

		std::wstring infoText = infoSS.str();
		Gdiplus::RectF infoRect(finalRenderX, finalRenderY - 25, 200, 40);

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
void DontStarve_EditorMain::DrawDebugInfo(Gdiplus::Graphics* pGraphics) {

	// 렉을 줄이기 위해 디버그 정보 업데이트 빈도 조절 (초당 4회)
	static ULONGLONG lastUpdateTick = 0;
	static std::wstring debugInfoString; // 정적 변수로 문자열을 캐시
	ULONGLONG currentTick = GetTickCount64();

	if (currentTick - lastUpdateTick > 250) { // 250ms (초당 4번)마다 업데이트
		lastUpdateTick = currentTick;

		std::wstringstream ss;
		ss << L"Mouse: " << m_rawMousePos.x << L", " << m_rawMousePos.y << L"\n";

		// 맵 정보 표시
		ss << L"Map Size: " << MAP_WIDTH << L"x" << MAP_HEIGHT << L" tiles (Tile: " << TILE_SIZE << L"px)\n";
		ss << L"World Size: " << (MAP_WIDTH * TILE_SIZE) << L"x" << (MAP_HEIGHT * TILE_SIZE) << L"px\n";

		ss << L"Map Offset: " << m_mapOffset.x << L", " << m_mapOffset.y << L"\n";

		// 좌표 변환 디버그 정보
		Gdiplus::PointF mouseWorldPos = ScreenToWorld(Gdiplus::PointF((float)m_rawMousePos.x, (float)m_rawMousePos.y));
		ss << L"World Pos: (" << (int)mouseWorldPos.X << L", " << (int)mouseWorldPos.Y << L")\n";

		ss << L"\nPlacing Mode: " << (m_isPlacingMode ? L"ON" : L"OFF");
		if (m_isPlacingMode && m_selectedPaletteIndex != -1) {
			const PaletteItem& item = m_paletteItems[m_selectedPaletteIndex];
			ss << L"  (Edit Mode: " << (item.category == CATEGORY_TILE ? L"Tile" : L"Object") << L")";
		}
		ss << L"\n";

		// 편집 모드 상태
		ss << L"Pivot Edit Mode: " << (m_isPivotEditMode ? L"ON" : L"OFF") << L"\n";
		ss << L"Collider Edit Mode: " << (m_isColliderEditMode ? L"ON" : L"OFF") << L"\n";
		ss << L"Walkable Area Edit Mode: " << (m_isWalkableEditMode ? L"ON" : L"OFF") << L"\n";
		ss << L"Camera Dragging: " << (m_isDraggingCamera ? L"ON" : L"OFF") << L"\n";

		if (m_isPlacingMode)
		{
			ss << L"\n Selected Tile Info\n";
			const TileVariant* tv = m_subPalette.getSelectedTileVariant();
			if (tv) {
				ss << L"선택된 타일 - Type: " << EnumUtils::GetEnumName(tv->type)
					<< L", ID: " << EnumUtils::GetEnumName(tv->id) << L"\n";
			}

			const ObjectVariant* ov = m_subPalette.getSelectedObjectVariant();
			if (ov) {
				ss << L"선택된 오브젝트 - Type: " << EnumUtils::GetEnumName(ov->type)
					<< L", ID: " << EnumUtils::GetEnumName(ov->id) << L"\n";
			}
		}



		// 선택된 오브젝트
		if (m_selectedObjectPtr) {
			ss << L"\n선택된 맵 오브젝트:" << L"\n";
			ss << L"Type: " << EnumUtils::GetEnumName(m_selectedObjectPtr->type)
				<< L", ID: " << EnumUtils::GetEnumName(m_selectedObjectPtr->id) << L"\n";
			ss << L"위치 - X: " << m_selectedObjectPtr->x << L", Y: " << m_selectedObjectPtr->y << L"\n";
			ss << L"Pivot: " << m_selectedObjectPtr->pivotX << L", " << m_selectedObjectPtr->pivotY << L"\n";
		}

		ss << L"\n성능 정보 확인" << L"\n";

		// 성능 정보
		ss << L"FPS: " << (int)m_currentFPS << L"\n";
		ss << L"Layer Memory: " << (int)GetLayerMemoryUsageMB() << L"MB\n";

		// 줌 및 화면 크기 정보
		float screenTileSize = (float)TILE_SIZE * m_zoomFactor;
		ss << L"\nScreen Tile Size: " << (int)screenTileSize << L"px (World: " << TILE_SIZE << L"px)\n";
		ss << L"Zoom Factor: " << (int)(m_zoomFactor * 100) << L"%\n";
		if (m_tileLayerBitmap) {
			ss << L"Layer Size: " << m_tileLayerBitmap->GetWidth() << L"x" << m_tileLayerBitmap->GetHeight() << L"px\n";
		}

		// 플레이어 스폰 포인트 정보
		ss << L"\nPlayer Spawn Mode: " << (m_isPlayerSpawnMode ? L"ON" : L"OFF") << L"\n";
		if (m_hasPlayerSpawn) {
			ss << L"Player Spawn: (" << (int)m_playerSpawnPoint.X << L", " << (int)m_playerSpawnPoint.Y << L")\n";
		}
		else {
			ss << L"Player Spawn: Not Set\n";
		}

		ss << L"Objects in Map: " << m_gameObjects.size() << L"\n";

		// Walkable Areas 통계
		if (m_isWalkableEditMode) {
			int walkableCount = 0;
			int blockedCount = 0;
			for (int y = 0; y < MAP_HEIGHT; ++y) {
				for (int x = 0; x < MAP_WIDTH; ++x) {
					if (m_walkableAreaMap[y][x]) walkableCount++;
					else blockedCount++;
				}
			}
			ss << L"Walkable Areas - Walkable: " << walkableCount
				<< L", Blocked: " << blockedCount << L" tiles\n";
		}

		ss << L"\n--- Hotkeys ---\n";
		ss << L"V: Pivot Edit (select object first)\n";
		ss << L"C: Collider Edit (select object first)\n";
		ss << L"R: Delete Object (select object first)\n";
		ss << L"G: Walkable Area Edit (drag to toggle)\n";
		ss << L"P: Player Spawn Mode\n";
		ss << L"F1: Toggle Debug Info\n";
		ss << L"Right Click + Drag: Camera Movement\n";
		ss << L"Right Click: Select Object\n";
		ss << L"Ctrl+N: New Map\n";
		ss << L"Ctrl+S: Save Map\n";
		ss << L"Ctrl+O: Load Map\n";

		debugInfoString = ss.str(); // 문자열 캐시
	}

	Gdiplus::Font font(L"Arial", 10);
	Gdiplus::SolidBrush brush(Gdiplus::Color(255, 0, 0, 0));
	pGraphics->DrawString(debugInfoString.c_str(), -1, &font, Gdiplus::PointF(10, 10), &brush);
}

// DrawSubPalette (하위 팔레트 내용 그리기)
void DontStarve_EditorMain::DrawSubPalette(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || !m_subPalette.isOpen) return;

	// 하위 팔레트 배경 그리기
	Gdiplus::SolidBrush subPaletteBackgroundBrush(Gdiplus::Color(100, 50, 50, 50));
	pGraphics->FillRectangle(&subPaletteBackgroundBrush,
		(Gdiplus::REAL)m_subPalette.rect.left, (Gdiplus::REAL)m_subPalette.rect.top,
		(Gdiplus::REAL)(m_subPalette.rect.right - m_subPalette.rect.left), (Gdiplus::REAL)(m_subPalette.rect.bottom - m_subPalette.rect.top));

	// 하위 팔레트 아이템 그리기
	int subItemSize = 48;
	for (size_t i = 0; i < m_subPalette.itemRects.size(); ++i)
	{
		const RECT& itemRect = m_subPalette.itemRects[i];

		Gdiplus::Bitmap* itemBitmap = nullptr;
		Gdiplus::RectF itemSourceRect;
		std::wstring itemName = L"";

		if (m_subPalette.category == CATEGORY_TILE) {
			const TileVariant* tv = m_subPalette.currentTileVariantDefs[i].second;
			if (tv) {
				itemBitmap = tv->pAtlasBitmap;
				itemSourceRect = tv->sourceRect;
				itemName = EnumUtils::GetEnumName(tv->id);
			}
		}
		else if (m_subPalette.category == CATEGORY_OBJECT) {
			const ObjectVariant* ov = m_subPalette.currentObjectVariantDefs[i].second;
			if (ov) {
				itemBitmap = ov->pAtlasBitmap;
				itemSourceRect = ov->sourceRect;
				itemName = EnumUtils::GetEnumName(ov->id);
			}
		}

		if (itemBitmap && itemBitmap->GetLastStatus() == Gdiplus::Ok)
		{
			// 아이콘 배경 (선택 여부에 따라 색상 변경 가능)
			Gdiplus::SolidBrush itemBackgroundBrush(Gdiplus::Color(100, 70, 70, 70));
			pGraphics->FillRectangle(&itemBackgroundBrush,
				(Gdiplus::REAL)itemRect.left, (Gdiplus::REAL)itemRect.top,
				(Gdiplus::REAL)(itemRect.right - itemRect.left), (Gdiplus::REAL)(itemRect.bottom - itemRect.top));

			// 아이콘 이미지 그리기 
			pGraphics->DrawImage(itemBitmap,
				Gdiplus::RectF((float)itemRect.left, (float)itemRect.top, (float)itemRect.right - itemRect.left, (float)itemRect.bottom - itemRect.top),
				itemSourceRect.X, itemSourceRect.Y, itemSourceRect.Width, itemSourceRect.Height,
				Gdiplus::UnitPixel);

			// 텍스트 그리기 (아이템 이름)
			Gdiplus::Font font(L"Arial", 7);
			Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
			pGraphics->DrawString(itemName.c_str(), -1, &font,
				Gdiplus::PointF((float)itemRect.left, (float)itemRect.bottom - 12), &textBrush);
		}

		// 선택된 하위 팔레트 아이템 강조
		if ((m_subPalette.category == CATEGORY_TILE && (int)i == m_subPalette.selectedTileVariantIndex) ||
			(m_subPalette.category == CATEGORY_OBJECT && (int)i == m_subPalette.selectedObjectVariantIndex)) {
			Gdiplus::Pen highlightPen(Gdiplus::Color(255, 255, 255, 0), 3.0f);
			pGraphics->DrawRectangle(&highlightPen,
				(Gdiplus::REAL)itemRect.left, (Gdiplus::REAL)itemRect.top,
				(Gdiplus::REAL)(itemRect.right - itemRect.left), (Gdiplus::REAL)(itemRect.bottom - itemRect.top));
		}
	}
}

// Pivot 편집 관련 함수들
void DontStarve_EditorMain::StartPivotEdit(GameObjectData* pObject)
{
	if (!pObject) return;

	m_editingObject = pObject;
	m_currentPivotX = pObject->pivotX;
	m_currentPivotY = pObject->pivotY;
	m_isPivotEditMode = true;

	// 편집 중인 오브젝트의 ObjectVariant 정의를 가져옴
	const ObjectVariant* ov = GetObjectVariant(pObject->type, pObject->id);
	if (!ov) {
		OutputDebugStringW(L"Error: ObjectVariant not found for pivot edit.\n");
		return; // ObjectVariant 없으면 피벗 편집 불가
	}

	// 오브젝트 이미지의 실제 크기 (ObjectVariant.sourceRect에서 가져옴)
	float objWidth = (float)ov->sourceRect.Width;
	float objHeight = (float)ov->sourceRect.Height;

	// 오브젝트의 월드 좌표 (pObject->x, pObject->y는 발 밑 중심)
	// 화면 좌표로 변환 (m_mapOffset, zoomFactor)
	float screenX_center = (float)pObject->x * m_zoomFactor + m_mapOffset.x;
	float screenY_center = (float)pObject->y * m_zoomFactor + m_mapOffset.y;

	// 스케일된 이미지 크기
	float scaledWidth = objWidth * m_zoomFactor;
	float scaledHeight = objHeight * m_zoomFactor;

	// 오브젝트 이미지의 좌상단 (피벗 고려 후)
	float imageRenderLeft = screenX_center - (ov->pivotX * scaledWidth);
	float imageRenderTop = screenY_center - (ov->pivotY * scaledHeight);

	// 편집기의 시작 위치 (현재 피벗 값에 해당하는 이미지의 화면상 지점)
	m_pivotEditPos.x = (LONG)(imageRenderLeft + (m_currentPivotX * scaledWidth));
	m_pivotEditPos.y = (LONG)(imageRenderTop + (m_currentPivotY * scaledHeight));
}

void DontStarve_EditorMain::UpdatePivotEdit(POINT mousePos)
{
	if (!m_editingObject || !m_isPivotEditMode) return;

	// 편집 중인 오브젝트의 ObjectVariant 정의를 가져옴
	const ObjectVariant* ov = GetObjectVariant(m_editingObject->type, m_editingObject->id);
	if (!ov) {
		OutputDebugStringW(L"Error: ObjectVariant not found during pivot update.\n");
		return;
	}

	// 오브젝트 이미지의 실제 크기
	float objWidth = (float)ov->sourceRect.Width;
	float objHeight = (float)ov->sourceRect.Height;

	// 오브젝트의 월드 좌표 (m_editingObject->x, m_editingObject->y는 발 밑 중심)
	// 화면 좌표로 변환
	float screenX_center = (float)m_editingObject->x * m_zoomFactor + m_mapOffset.x;
	float screenY_center = (float)m_editingObject->y * m_zoomFactor + m_mapOffset.y;

	float scaledWidth = objWidth * m_zoomFactor;
	float scaledHeight = objHeight * m_zoomFactor;

	// 오브젝트 이미지의 좌상단 (피벗 고려 후)
	float imageRenderLeft = screenX_center - (ov->pivotX * scaledWidth);
	float imageRenderTop = screenY_center - (ov->pivotY * scaledHeight);

	// 마우스 위치를 오브젝트 이미지의 로컬 좌표 (0.0f ~ 1.0f)로 변환
	// mousePos는 화면 절대 좌표
	float localX = (mousePos.x - imageRenderLeft) / scaledWidth;
	float localY = (mousePos.y - imageRenderTop) / scaledHeight;

	// 피벗 값 (0.0f ~ 1.0f) 범위 제한
	m_currentPivotX = max(0.0f, min(1.0f, localX));
	m_currentPivotY = max(0.0f, min(1.0f, localY));

	// GameObjectData에 피벗 값 저장
	m_editingObject->pivotX = m_currentPivotX;
	m_editingObject->pivotY = m_currentPivotY;

	// 오브젝트 레이어 다시 그리기 (피벗 변경 반영)
	m_objectLayerDirty = true;
}

void DontStarve_EditorMain::EndPivotEdit()
{
	m_isPivotEditMode = false;
	m_editingObject = nullptr;
	m_objectLayerDirty = true; // 피벗 편집 종료 시 레이어 갱신
}

void DontStarve_EditorMain::StartColliderEdit(GameObjectData* obj)
{
	m_isColliderEditMode = true;
	m_editingColliderObject = obj;
	if (m_editingColliderObject) {
		// 콜라이더가 없으면 오브젝트 크기로 초기화 (기본 Box Collider)
		if (!m_editingColliderObject->hasCollider) {
			m_editingColliderObject->hasCollider = true;
			m_editingColliderObject->colliderOffsetX = 0; // 오브젝트 좌상단에 맞춰 0
			m_editingColliderObject->colliderOffsetY = 0;

			// ObjectVariant에서 실제 오브젝트 크기 가져오기
			const ObjectVariant* ov = GetObjectVariant(obj->type, obj->id);
			if (!ov) {
				OutputDebugStringW(L"Error: ObjectVariant not found for collider edit.\n");
				m_isColliderEditMode = false; // Variant 없으면 편집 시작 못하게
				m_editingColliderObject = nullptr;
				return;
			}
			m_editingColliderObject->colliderWidth = (int)ov->sourceRect.Width;
			m_editingColliderObject->colliderHeight = (int)ov->sourceRect.Height;
		}
		// 편집 시작 시 콜라이더의 초기 상태 저장 (오브젝트 로컬 좌표계 기준)
		m_initialColliderRect = {
			m_editingColliderObject->colliderOffsetX,
			m_editingColliderObject->colliderOffsetY,
			m_editingColliderObject->colliderOffsetX + m_editingColliderObject->colliderWidth,
			m_editingColliderObject->colliderOffsetY + m_editingColliderObject->colliderHeight
		};
	}
	m_objectLayerDirty = true; // 콜라이더 편집 모드 시작 시 레이어 갱신 (선택 표시 등)
}

// GetTileVariant (m_tileVariants 맵에서 TileVariant 검색)
const TileVariant* DontStarve_EditorMain::GetTileVariant(TileType type, TileID id) const {
	auto type_it = m_tileVariants.find(type);
	if (type_it != m_tileVariants.end()) {
		auto id_it = type_it->second.find(id);
		if (id_it != type_it->second.end()) {
			return &(id_it->second);
		}
	}
	return nullptr;
}

// GetObjectVariant (m_objectVariants 맵에서 ObjectVariant 검색)
const ObjectVariant* DontStarve_EditorMain::GetObjectVariant(GameObjectType type, GameObjectID id) const {
	auto type_it = m_objectVariants.find(type);
	if (type_it != m_objectVariants.end()) {
		auto id_it = type_it->second.find(id);
		if (id_it != type_it->second.end()) {
			return &(id_it->second);
		}
	}
	return nullptr;
}

// 콜라이더 편집 종료 함수
void DontStarve_EditorMain::EndColliderEdit() {
	m_isColliderEditMode = false;
	m_editingColliderObject = nullptr;
	m_isDraggingCollider = false;
	m_draggingHandle = -1;
	ReleaseCapture(); // 혹시 모를 캡처 해제
}

// 마우스 위치에 콜라이더 핸들이 있는지 확인하는 함수
// 반환값: 0:좌상단, 1:우상단, 2:좌하단, 3:우하단, 4:중앙(이동), -1:없음
int DontStarve_EditorMain::GetColliderHandleAt(POINT screenPos) {
	if (!m_isColliderEditMode || !m_editingColliderObject || !m_editingColliderObject->hasCollider) return -1;

	float objRenderX = (float)m_editingColliderObject->x * m_zoomFactor + m_mapOffset.x;
	float objRenderY = (float)m_editingColliderObject->y * m_zoomFactor + m_mapOffset.y;

	float colliderRenderX = objRenderX + (m_editingColliderObject->colliderOffsetX * m_zoomFactor);
	float colliderRenderY = objRenderY + (m_editingColliderObject->colliderOffsetY * m_zoomFactor);
	float colliderRenderWidth = (float)m_editingColliderObject->colliderWidth * m_zoomFactor;
	float colliderRenderHeight = (float)m_editingColliderObject->colliderHeight * m_zoomFactor;

	float handleSize = 8.0f; // 핸들 크기 (화면 픽셀)
	float halfHandle = handleSize / 2.0f;
	float clickTolerance = handleSize / 2.0f; // 클릭 허용 오차 (핸들 중심으로부터)

	// 핸들 영역 (좌상단, 우상단, 좌하단, 우하단)
	Gdiplus::PointF handleCenters[4];
	handleCenters[0] = Gdiplus::PointF(colliderRenderX, colliderRenderY); // 좌상단
	handleCenters[1] = Gdiplus::PointF(colliderRenderX + colliderRenderWidth, colliderRenderY); // 우상단
	handleCenters[2] = Gdiplus::PointF(colliderRenderX, colliderRenderY + colliderRenderHeight); // 좌하단
	handleCenters[3] = Gdiplus::PointF(colliderRenderX + colliderRenderWidth, colliderRenderY + colliderRenderHeight); // 우하단

	for (int i = 0; i < 4; ++i) {
		// 마우스 클릭이 핸들 중심으로부터 허용 오차 범위 내에 있는지 확인
		if (abs(screenPos.x - handleCenters[i].X) < clickTolerance &&
			abs(screenPos.y - handleCenters[i].Y) < clickTolerance) {
			return i; // 핸들 인덱스 반환
		}
	}

	// 중앙 이동 핸들 또는 콜라이더 내부 (이동)
	Gdiplus::RectF colliderBounds(colliderRenderX, colliderRenderY, colliderRenderWidth, colliderRenderHeight);
	if (colliderBounds.Contains((float)screenPos.x, (float)screenPos.y)) {
		return 4; // 중앙 (이동) 핸들
	}

	return -1; // 핸들 없음
}

void DontStarve_EditorMain::ComposePaletteLayer() {
	if (!m_paletteLayerDirty || !m_paletteLayerBitmap) return;

	Gdiplus::Graphics paletteLayerGraphics(m_paletteLayerBitmap);
	paletteLayerGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

	DrawPalette(&paletteLayerGraphics);

	m_paletteLayerDirty = false;
}

// ComposeGridLayer (그리드 레이어 비트맵을 미리 그리는 함수)
void DontStarve_EditorMain::ComposeGridLayer() {
	// m_gridLayerDirty 플래그가 false이거나, m_gridLayerBitmap이 유효하지 않으면 다시 그리지 않습니다.
	if (!m_gridLayerDirty || !m_gridLayerBitmap) return;

	// --- 디버깅 시작: 그리드 레이어 재구성 시작 ---
	// OutputDebugStringW(L"Composing Grid Layer: START\n"); 
	// --- 디버깅 끝 ---

	Gdiplus::Graphics gridLayerGraphics(m_gridLayerBitmap);
	// 그리드 비트맵을 투명하게 클리어합니다.
	gridLayerGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

	// DrawGrid 함수를 호출하여 모든 그리드 선을 m_gridLayerBitmap 위에 그립니다.
	// DrawGrid는 맵 좌표계를 기준으로 그리드 선을 그립니다.
	// m_zoomFactor와 g_displayScaleFactor가 DrawGrid 내부에서 적용됩니다.
	DrawGrid(&gridLayerGraphics);

	// 그리드 레이어가 성공적으로 재구성되었으므로 Dirty 플래그를 false로 설정합니다.
	m_gridLayerDirty = false;

	// --- 디버깅 시작: 그리드 레이어 재구성 완료 ---
	// OutputDebugStringW(L"Composing Grid Layer: END (Dirty set to false)\n");
	// --- 디버깅 끝 ---
}

Gdiplus::RectF DontStarve_EditorMain::GetViewWorldRect(float cullingMargin) const {
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);

	// 화면 좌표(픽셀)를 월드 좌표로 역변환
	float viewWorldX = -(Gdiplus::REAL)m_mapOffset.x / (m_zoomFactor);
	float viewWorldY = -(Gdiplus::REAL)m_mapOffset.y / (m_zoomFactor);
	float viewWorldWidth = (Gdiplus::REAL)clientRect.right / (m_zoomFactor);
	float viewWorldHeight = (Gdiplus::REAL)clientRect.bottom / (m_zoomFactor);

	// 컬링 마진 적용
	if (cullingMargin > 0.0f) {
		viewWorldX -= cullingMargin;
		viewWorldY -= cullingMargin;
		viewWorldWidth += 2 * cullingMargin;
		viewWorldHeight += 2 * cullingMargin;
	}

	return Gdiplus::RectF(viewWorldX, viewWorldY, viewWorldWidth, viewWorldHeight);
}

Gdiplus::PointF DontStarve_EditorMain::WorldToScreen(Gdiplus::PointF worldPos) const {
	return Gdiplus::PointF(
		worldPos.X * m_zoomFactor + m_mapOffset.x,
		worldPos.Y * m_zoomFactor + m_mapOffset.y
	);
}

// 월드 Rect를 화면 Rect로 변환
Gdiplus::RectF DontStarve_EditorMain::WorldToScreen(Gdiplus::RectF worldRect) const {
	Gdiplus::PointF screenTopLeft = WorldToScreen(Gdiplus::PointF(worldRect.X, worldRect.Y));
	return Gdiplus::RectF(
		screenTopLeft.X,
		screenTopLeft.Y,
		worldRect.Width * m_zoomFactor,
		worldRect.Height * m_zoomFactor
	);
}

// 화면 좌표를 월드 좌표로 변환
Gdiplus::PointF DontStarve_EditorMain::ScreenToWorld(Gdiplus::PointF screenPos) const {
	return Gdiplus::PointF(
		(screenPos.X - m_mapOffset.x) / m_zoomFactor,
		(screenPos.Y - m_mapOffset.y) / m_zoomFactor
	);
}


void DontStarve_EditorMain::DrawPivotEditor(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || !m_isPivotEditMode || !m_editingObject) return;

	// 오브젝트의 ObjectVariant 가져오기
	const ObjectVariant* ov = GetObjectVariant(m_editingObject->type, m_editingObject->id);
	if (!ov || !ov->pAtlasBitmap) return;

	// 오브젝트의 월드 좌표 (m_editingObject->x,y는 발 밑 중앙)
	float objWorldX = (float)m_editingObject->x;
	float objWorldY = (float)m_editingObject->y;

	// 이미지의 실제 크기 (ObjectVariant.sourceRect에서 가져옴)
	float objWidth = (float)ov->sourceRect.Width;
	float objHeight = (float)ov->sourceRect.Height;

	// 화면상에 오브젝트가 그려질 영역의 좌상단 (피벗 고려 전) 및 크기 (m_mapOffset, zoom, g_displayScaleFactor 적용)
	// 오브젝트의 (x,y)는 발 밑 중심 월드 좌표이므로, 이를 화면 좌표로 변환
	float screenX_center = objWorldX * m_zoomFactor + m_mapOffset.x;
	float screenY_center = objWorldY * m_zoomFactor + m_mapOffset.y;

	float scaledWidth = objWidth * m_zoomFactor;
	float scaledHeight = objHeight * m_zoomFactor;

	// 피벗 포인트 그리기 (피벗 위치에 십자선 또는 점)
	// 화면상에서 오브젝트 이미지의 좌상단 위치 (피벗 고려 후)
	float imageRenderLeft = screenX_center - (ov->pivotX * scaledWidth);
	float imageRenderTop = screenY_center - (ov->pivotY * scaledHeight);

	// 피벗 지점 (스크린 좌표) - 현재 편집 중인 m_editingObject->pivotX/Y를 사용
	float pivotScreenX = imageRenderLeft + (m_editingObject->pivotX * scaledWidth);
	float pivotScreenY = imageRenderTop + (m_editingObject->pivotY * scaledHeight);

	// 십자선
	Gdiplus::Pen pivotPen(Gdiplus::Color(255, 255, 0, 0), 2.0f);
	pGraphics->DrawLine(&pivotPen, pivotScreenX - 10, pivotScreenY, pivotScreenX + 10, pivotScreenY);
	pGraphics->DrawLine(&pivotPen, pivotScreenX, pivotScreenY - 10, pivotScreenX, pivotScreenY + 10);

	// 오브젝트의 바운딩 박스 그리기 (피벗과 관계없이 오브젝트 이미지가 차지하는 전체 영역)
	Gdiplus::Pen bboxPen(Gdiplus::Color(255, 0, 255, 255), 1.0f);
	pGraphics->DrawRectangle(&bboxPen, imageRenderLeft, imageRenderTop, scaledWidth, scaledHeight);
}


void DontStarve_EditorMain::DrawColliders(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics || !m_isColliderEditMode || !m_editingColliderObject) return;

	GameObjectData& obj = *m_editingColliderObject;

	// 오브젝트의 ObjectVariant 가져오기
	const ObjectVariant* ov = GetObjectVariant(obj.type, obj.id);
	if (!ov || !ov->pAtlasBitmap) return;

	// 콜라이더의 월드 좌표와 크기
	// obj.x, obj.y (오브젝트의 발 밑 중심)을 기준으로 colliderOffsetX,Y만큼 오프셋된 콜라이더의 월드 좌표 좌상단
	float colliderWorldX_top_left = (float)obj.x + obj.colliderOffsetX;
	float colliderWorldY_top_left = (float)obj.y + obj.colliderOffsetY;
	float colliderWidth = (float)obj.colliderWidth;
	float colliderHeight = (float)obj.colliderHeight;

	// 콜라이더의 화면상 좌상단 및 크기
	float colliderScreenX = colliderWorldX_top_left * m_zoomFactor + m_mapOffset.x;
	float colliderScreenY = colliderWorldY_top_left * m_zoomFactor + m_mapOffset.y;
	float colliderScaledWidth = colliderWidth * m_zoomFactor;
	float colliderScaledHeight = colliderHeight * m_zoomFactor;

	// 콜라이더 사각형 그리기
	Gdiplus::Pen colliderPen(Gdiplus::Color(255, 255, 0, 0), 2.0f);
	pGraphics->DrawRectangle(&colliderPen,
		colliderScreenX, colliderScreenY, colliderScaledWidth, colliderScaledHeight);

	// 크기 조절 핸들 그리기
	Gdiplus::SolidBrush handleBrush(Gdiplus::Color(255, 0, 255, 255));
	int handleSize = 8;
	Gdiplus::REAL handleSizeReal = (Gdiplus::REAL)handleSize;
	Gdiplus::REAL halfHandleReal = handleSizeReal / 2.0f;

	// 좌상단 핸들
	pGraphics->FillRectangle(&handleBrush,
		colliderScreenX - halfHandleReal,
		colliderScreenY - halfHandleReal,
		handleSizeReal,
		handleSizeReal);

	// 우상단 핸들
	pGraphics->FillRectangle(&handleBrush,
		colliderScreenX + colliderScaledWidth - halfHandleReal,
		colliderScreenY - halfHandleReal,
		handleSizeReal,
		handleSizeReal);

	// 좌하단 핸들
	pGraphics->FillRectangle(&handleBrush,
		colliderScreenX - halfHandleReal,
		colliderScreenY + colliderScaledHeight - halfHandleReal,
		handleSizeReal,
		handleSizeReal);

	// 우하단 핸들
	pGraphics->FillRectangle(&handleBrush,
		colliderScreenX + colliderScaledWidth - halfHandleReal,
		colliderScreenY + colliderScaledHeight - halfHandleReal,
		handleSizeReal,
		handleSizeReal);
}

// AddObject 함수 (오브젝트 추가)
void DontStarve_EditorMain::AddObject(const GameObjectData& obj) {
	m_gameObjects.push_back(obj);
	m_objectsDirty = true;
	m_objectLayerDirty = true;
}

// RemoveObject 함수 (인덱스로 오브젝트 삭제)
void DontStarve_EditorMain::RemoveObject(size_t idx) {
	if (idx < m_gameObjects.size()) {
		m_gameObjects.erase(m_gameObjects.begin() + idx);
		m_objectsDirty = true;
		m_objectLayerDirty = true;
	}
}
// RemoveObject 함수 (포인터로 오브젝트 삭제)
void DontStarve_EditorMain::RemoveObject(GameObjectData* objToRemove) {
	auto it = std::remove_if(m_gameObjects.begin(), m_gameObjects.end(),
		[objToRemove](const GameObjectData& obj) { return &obj == objToRemove; });
	if (it != m_gameObjects.end()) {
		m_gameObjects.erase(it, m_gameObjects.end());
		m_objectsDirty = true;
		m_objectLayerDirty = true;
	}
}

// UpdateObjectPosition 함수 (오브젝트 위치 업데이트)
void DontStarve_EditorMain::UpdateObjectPosition(GameObjectData* obj, int newX, int newY) {
	if (obj) {
		obj->x = newX;
		obj->y = newY;
		m_objectsDirty = true;
		m_objectLayerDirty = true;
	}
}

// ===== 성능 모니터링 함수 =====



// 레이어 메모리 사용량 반환 (MB)
float DontStarve_EditorMain::GetLayerMemoryUsageMB() const {
	if (!m_tileLayerBitmap) return 0.0f;

	UINT totalPixels = m_tileLayerBitmap->GetWidth() * m_tileLayerBitmap->GetHeight() * 3; // 3개 레이어
	return (totalPixels * 4) / (1024.0f * 1024.0f); // 4바이트/픽셀 (ARGB)
}

// Walkable 영역 그리기
void DontStarve_EditorMain::DrawWalkableAreas(Gdiplus::Graphics* pGraphics) {
	if (!pGraphics) return;

	// Walkable 편집 모드가 아닐 때는 그리지 않음
	if (!m_isWalkableEditMode) return;

	// 현재 뷰포트의 월드 좌표 계산
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	Gdiplus::PointF viewTopLeft = ScreenToWorld(Gdiplus::PointF(0, 0));
	Gdiplus::PointF viewBottomRight = ScreenToWorld(Gdiplus::PointF((float)clientRect.right, (float)clientRect.bottom));

	// 타일 인덱스 범위 계산 (뷰포트에 보이는 타일만)
	int startX = max(0, (int)floor(viewTopLeft.X / TILE_SIZE));
	int endX = min(MAP_WIDTH, (int)ceil(viewBottomRight.X / TILE_SIZE));
	int startY = max(0, (int)floor(viewTopLeft.Y / TILE_SIZE));
	int endY = min(MAP_HEIGHT, (int)ceil(viewBottomRight.Y / TILE_SIZE));

	// 브러시 생성
	Gdiplus::SolidBrush blockedBrush(Gdiplus::Color(100, 255, 0, 0));    // 반투명 빨간색 (Blocked)
	Gdiplus::SolidBrush walkableBrush(Gdiplus::Color(50, 0, 255, 0));    // 반투명 초록색 (Walkable)
	Gdiplus::Pen gridPen(Gdiplus::Color(150, 255, 255, 255), 1.0f);      // 하얀색 격자

	// 화면상 타일 크기
	float screenTileSize = (float)TILE_SIZE * m_zoomFactor;

	// walkable 상태에 따라 타일 색칠
	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			// 월드 좌표 계산
			float worldX = (float)x * TILE_SIZE;
			float worldY = (float)y * TILE_SIZE;

			// 뷰포트 기준 화면 좌표 계산
			float screenX = (worldX - viewTopLeft.X) * m_zoomFactor;
			float screenY = (worldY - viewTopLeft.Y) * m_zoomFactor;

			// 화면 밖 컬링
			if (screenX + screenTileSize < 0 || screenX > clientRect.right ||
				screenY + screenTileSize < 0 || screenY > clientRect.bottom) continue;

			Gdiplus::RectF tileRect(screenX, screenY, screenTileSize, screenTileSize);

			// walkable 상태에 따라 색칠
			if (m_walkableAreaMap[y][x]) {
				pGraphics->FillRectangle(&walkableBrush, tileRect);
			}
			else {
				pGraphics->FillRectangle(&blockedBrush, tileRect);
			}

			// 격자 그리기
			pGraphics->DrawRectangle(&gridPen, tileRect);
		}
	}

	// 드래그 중일 때 드래그 영역 표시
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

	// 모드 안내 텍스트
	Gdiplus::Font font(L"Arial", 14, Gdiplus::FontStyleBold);
	Gdiplus::SolidBrush textBrush(Gdiplus::Color(255, 255, 255, 255));
	Gdiplus::SolidBrush backgroundBrush(Gdiplus::Color(150, 0, 0, 0));

	std::wstring modeText = L"[WALKABLE AREA EDIT MODE] Drag to toggle walkable areas (G to exit)";
	Gdiplus::RectF textRect(10, 70, 700, 30);

	// 배경
	pGraphics->FillRectangle(&backgroundBrush, textRect);
	// 텍스트
	pGraphics->DrawString(modeText.c_str(), -1, &font, textRect, nullptr, &textBrush);
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

		// 배경
		pGraphics->FillRectangle(&backgroundBrush, textRect);
		// 텍스트
		pGraphics->DrawString(modeText.c_str(), -1, &font, textRect, nullptr, &textBrush);
	}

	// 플레이어 스폰 포인트가 설정되어 있으면 그리기
	if (m_hasPlayerSpawn) {
		// 월드 좌표를 화면 좌표로 변환
		Gdiplus::PointF screenPos = WorldToScreen(m_playerSpawnPoint);

		// 플레이어 아이콘 크기 (화면 좌표 기준)
		float iconRadius = 16.0f;

		// 플레이어 스폰 포인트 원형 배경
		Gdiplus::SolidBrush spawnBrush(Gdiplus::Color(200, 0, 255, 0));
		Gdiplus::Pen spawnPen(Gdiplus::Color(255, 255, 255, 255), 3.0f);

		// 원형 배경
		pGraphics->FillEllipse(&spawnBrush,
			screenPos.X - iconRadius, screenPos.Y - iconRadius,
			iconRadius * 2, iconRadius * 2);

		// 원형 테두리
		pGraphics->DrawEllipse(&spawnPen,
			screenPos.X - iconRadius, screenPos.Y - iconRadius,
			iconRadius * 2, iconRadius * 2);

		// 플레이어 심볼 (P 문자)
		Gdiplus::Font playerFont(L"Arial", 12, Gdiplus::FontStyleBold);
		Gdiplus::SolidBrush playerTextBrush(Gdiplus::Color(255, 255, 255, 255));

		Gdiplus::RectF playerTextRect(screenPos.X - 8, screenPos.Y - 8, 16, 16);
		Gdiplus::StringFormat centerFormat;
		centerFormat.SetAlignment(Gdiplus::StringAlignmentCenter);
		centerFormat.SetLineAlignment(Gdiplus::StringAlignmentCenter);

		pGraphics->DrawString(L"P", 1, &playerFont, playerTextRect, &centerFormat, &playerTextBrush);

		// 좌표 정보 표시
		if (m_isPlayerSpawnMode) {
			Gdiplus::Font coordFont(L"Arial", 10);
			Gdiplus::SolidBrush coordBrush(Gdiplus::Color(255, 255, 255, 255));

			std::wstringstream coordSS;
			coordSS << L"(" << (int)m_playerSpawnPoint.X << L", " << (int)m_playerSpawnPoint.Y << L")";
			std::wstring coordText = coordSS.str();

			Gdiplus::RectF coordRect(screenPos.X - 40, screenPos.Y + iconRadius + 5, 80, 15);
			pGraphics->DrawString(coordText.c_str(), -1, &coordFont, coordRect, &centerFormat, &coordBrush);
		}
	}
}
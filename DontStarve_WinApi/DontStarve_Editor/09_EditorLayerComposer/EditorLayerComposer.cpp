#include "../pch.h"
#include "EditorLayerComposer.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../00_MainEditor/DontStarve_EditorMain.h"
#include "../../Header/Define.h"
#include "../../Header/Function.h"
#include "../../Header/Struct.h"
#include <algorithm>
#include <memory>

EditorLayerComposer::EditorLayerComposer()
	: m_pView(nullptr), m_pResources(nullptr), m_pMain(nullptr),
	m_gridLayerBitmap(nullptr), m_tileLayerBitmap(nullptr), m_objectLayerBitmap(nullptr),
	m_gridLayerDirty(true), m_tileLayerDirty(true), m_objectLayerDirty(true)
{
}

EditorLayerComposer::~EditorLayerComposer()
{
	DeleteLayerBitmaps();
}

void EditorLayerComposer::SetDependencies(EditorView* pView, EditorResourceManager* pResources, DontStarve_EditorMain* pMain)
{
	m_pView = pView;
	m_pResources = pResources;
	m_pMain = pMain;
}

void EditorLayerComposer::ResizeLayerBitmaps(UINT width, UINT height)
{
	DeleteLayerBitmaps();
	m_gridLayerBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	m_tileLayerBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	m_objectLayerBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
	m_gridLayerDirty = true;
	m_tileLayerDirty = true;
	m_objectLayerDirty = true;
}

void EditorLayerComposer::DeleteLayerBitmaps()
{
	Utils::SafeDelete(m_gridLayerBitmap);
	Utils::SafeDelete(m_tileLayerBitmap);
	Utils::SafeDelete(m_objectLayerBitmap);
}

void EditorLayerComposer::ComposeGridLayer()
{
	if (!m_gridLayerDirty || !m_gridLayerBitmap || !m_pView) return;

	Gdiplus::Graphics gridLayerGraphics(m_gridLayerBitmap);
	gridLayerGraphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	gridLayerGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
	gridLayerGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
	gridLayerGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

	DrawGrid(&gridLayerGraphics);

	m_gridLayerDirty = false;
}

void EditorLayerComposer::ComposeTileLayer()
{
	if (!m_tileLayerDirty || !m_tileLayerBitmap || !m_pView || !m_pMain) return;
	
	Gdiplus::Graphics tileLayerGraphics(m_tileLayerBitmap);
	tileLayerGraphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	tileLayerGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
	tileLayerGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
	tileLayerGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

	DrawTileMap(&tileLayerGraphics);

	m_tileLayerDirty = false;
}

void EditorLayerComposer::ComposeObjectLayer()
{
	if (!m_objectLayerDirty || !m_objectLayerBitmap || !m_pView || !m_pResources || !m_pMain) return;
	
	Gdiplus::Graphics objectLayerGraphics(m_objectLayerBitmap);
	objectLayerGraphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	objectLayerGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
	objectLayerGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
	objectLayerGraphics.Clear(Gdiplus::Color(0, 0, 0, 0));

	DrawObjects(&objectLayerGraphics);

	m_objectLayerDirty = false;
}

void EditorLayerComposer::DrawLayers(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics) return;

	// 레이어 비트맵은 이미 뷰포트 기반으로 렌더링되어 있으므로 (0, 0)에 그리기
	if (m_gridLayerBitmap) {
		pGraphics->DrawImage(m_gridLayerBitmap, 0, 0);
	}
	if (m_tileLayerBitmap) {
		pGraphics->DrawImage(m_tileLayerBitmap, 0, 0);
	}
	if (m_objectLayerBitmap) {
		pGraphics->DrawImage(m_objectLayerBitmap, 0, 0);
	}
}

void EditorLayerComposer::DrawGrid(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics || !m_pView || !m_gridLayerBitmap) return;

	// 그리드 펜 설정
	Gdiplus::Pen gridPen(Gdiplus::Color(120, 150, 150, 150), 1.0f);        // 일반 그리드 선
	Gdiplus::Pen majorGridPen(Gdiplus::Color(180, 100, 100, 100), 1.5f);   // 10타일마다 굵은 선
	Gdiplus::Pen mapBoundaryPen(Gdiplus::Color(255, 255, 0, 0), 3.0f);     // 맵 경계선

	// 줌 팩터가 적용된 화면상 타일 크기
	float screenTileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();

	// 그리드 선이 너무 작을 때는 건너뛰기
	if (screenTileSize < 6.0f) {
		return;
	}

	// 레이어 비트맵 크기 (현재 뷰포트 크기)
	UINT layerWidth = m_gridLayerBitmap->GetWidth();
	UINT layerHeight = m_gridLayerBitmap->GetHeight();

	// 현재 뷰포트의 월드 좌표 계산
	Gdiplus::PointF viewTopLeft = m_pView->ScreenToWorld(Gdiplus::PointF(0, 0));

	// 그리드 간격 최적화 (줌 레벨에 따라)
	int gridSpacing = TILE_SIZE;
	if (screenTileSize < 16.0f) gridSpacing *= 4;      // 매우 작을 때는 4칸마다
	else if (screenTileSize < 32.0f) gridSpacing *= 2;  // 작을 때는 2칸마다

	// 시작 그리드 인덱스 계산
	int startGridX = (int)floor(viewTopLeft.X / gridSpacing);
	int startGridY = (int)floor(viewTopLeft.Y / gridSpacing);

	// 화면에 그려질 그리드 개수 계산
	int maxGridLines = max(layerWidth / max(8, (int)screenTileSize), layerHeight / max(8, (int)screenTileSize));
	maxGridLines = min(maxGridLines, 100);

	// 세로선 그리기
	for (int i = 0; i <= maxGridLines; ++i) {
		int gridX = startGridX + i;
		if (gridX < 0 || gridX > m_pMain->GetMapWidth()) continue;

		float worldX = (float)(gridX * gridSpacing);
		float screenX = (worldX - viewTopLeft.X) * m_pView->GetZoomFactor();

		if (screenX >= 0 && screenX <= layerWidth) {
			bool isMajor = (gridX % 10 == 0) && (gridX != 0) && (gridX != m_pMain->GetMapWidth());
			Gdiplus::Pen* currentPen = isMajor ? &majorGridPen : &gridPen;

			pGraphics->DrawLine(currentPen, screenX, 0.0f, screenX, (float)layerHeight);
		}
	}

	// 가로선 그리기
	for (int i = 0; i <= maxGridLines; ++i) {
		int gridY = startGridY + i;
		if (gridY < 0 || gridY > m_pMain->GetMapHeight()) continue;

		float worldY = (float)(gridY * gridSpacing);
		float screenY = (worldY - viewTopLeft.Y) * m_pView->GetZoomFactor();

		if (screenY >= 0 && screenY <= layerHeight) {
			bool isMajor = (gridY % 10 == 0) && (gridY != 0) && (gridY != m_pMain->GetMapHeight());
			Gdiplus::Pen* currentPen = isMajor ? &majorGridPen : &gridPen;

			pGraphics->DrawLine(currentPen, 0.0f, screenY, (float)layerWidth, screenY);
		}
	}

	// 맵 경계선 그리기
	float mapLeftScreen = (0 - viewTopLeft.X) * m_pView->GetZoomFactor();
	float mapTopScreen = (0 - viewTopLeft.Y) * m_pView->GetZoomFactor();
	float mapWidthScreen = (m_pMain->GetMapWidth() * TILE_SIZE) * m_pView->GetZoomFactor();
	float mapHeightScreen = (m_pMain->GetMapHeight() * TILE_SIZE) * m_pView->GetZoomFactor();

	if (mapLeftScreen < layerWidth && mapTopScreen < layerHeight &&
		mapLeftScreen + mapWidthScreen > 0 && mapTopScreen + mapHeightScreen > 0) {

		Gdiplus::RectF mapRect(mapLeftScreen, mapTopScreen, mapWidthScreen, mapHeightScreen);
		pGraphics->DrawRectangle(&mapBoundaryPen, mapRect);
	}
}

void EditorLayerComposer::DrawTileMap(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics || !m_pView || !m_pMain || !m_tileLayerBitmap) return;

	// 레이어 비트맵 크기
	UINT layerWidth = m_tileLayerBitmap->GetWidth();
	UINT layerHeight = m_tileLayerBitmap->GetHeight();

	// 현재 뷰포트의 월드 좌표 계산
	Gdiplus::PointF viewTopLeft = m_pView->ScreenToWorld(Gdiplus::PointF(0, 0));
	Gdiplus::PointF viewBottomRight = m_pView->ScreenToWorld(Gdiplus::PointF((float)layerWidth, (float)layerHeight));

	// 타일 인덱스 범위 계산
	int startX = max(0, (int)floor(viewTopLeft.X / TILE_SIZE));
	int endX = min(m_pMain->GetMapWidth(), (int)ceil(viewBottomRight.X / TILE_SIZE));
	int startY = max(0, (int)floor(viewTopLeft.Y / TILE_SIZE));
	int endY = min(m_pMain->GetMapHeight(), (int)ceil(viewBottomRight.Y / TILE_SIZE));

	// 화면상 타일 크기
	float screenTileSize = (float)TILE_SIZE * m_pView->GetZoomFactor();

	// Main의 타일 맵에 접근 (friend) - [행][열] 형식
	const ResourcePathUtils::TileResourceDef(&tileMap)[MAP_HEIGHT][MAP_WIDTH] = m_pMain->m_tileMap;
	
	for (int y = startY; y < endY; ++y) {
		for (int x = startX; x < endX; ++x) {
			const ResourcePathUtils::TileResourceDef& tile = tileMap[y][x];
			if (tile.type == TILE_NONE || tile.id == TILEID_NONE || tile.imageName.empty()) continue;

			// 캐시된 비트맵 로드
			std::wstring fullPath = tile.baseDir;
			if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
				fullPath += L"\\";
			}
			fullPath += tile.imageName;
			std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
			if (!pBitmap) continue;

			// 월드 좌표 계산
			float worldX = (float)x * TILE_SIZE;
			float worldY = (float)y * TILE_SIZE;

			// 뷰포트 기준 화면 좌표 계산
			float screenX = (worldX - viewTopLeft.X) * m_pView->GetZoomFactor();
			float screenY = (worldY - viewTopLeft.Y) * m_pView->GetZoomFactor();

			// 화면 밖 컬링
			if (screenX + screenTileSize < 0 || screenX > layerWidth ||
				screenY + screenTileSize < 0 || screenY > layerHeight) continue;

			Gdiplus::RectF destRect(screenX, screenY, screenTileSize, screenTileSize);
			pGraphics->DrawImage(pBitmap.get(), destRect, 0, 0, (float)pBitmap->GetWidth(), (float)pBitmap->GetHeight(), Gdiplus::UnitPixel);
		}
	}
}

void EditorLayerComposer::DrawObjects(Gdiplus::Graphics* pGraphics)
{
	if (!pGraphics || !m_pView || !m_pResources || !m_pMain || !m_objectLayerBitmap) return;

	// Main의 멤버에 접근 (friend)
	const std::vector<ResourcePathUtils::ObjectResourceDef>& gameObjects = m_pMain->m_gameObjects;
	bool& objectsDirty = m_pMain->m_objectsDirty;
	std::vector<const ResourcePathUtils::ObjectResourceDef*>& sortedObjects = m_pMain->m_sortedObjects;
	std::vector<const ResourcePathUtils::ObjectResourceDef*>& visibleObjectsCache = m_pMain->m_visibleObjectsCache;
	Gdiplus::RectF& lastViewportWorldRect = m_pMain->m_lastViewportWorldRect;
	ResourcePathUtils::ObjectResourceDef*& selectedObjectPtr = m_pMain->m_selectedObjectPtr;

	const bool hadObjectsDirty = objectsDirty;
	if (objectsDirty) {
		sortedObjects.clear();
		for (const auto& obj : gameObjects) {
			sortedObjects.push_back(&obj);
		}
		std::sort(sortedObjects.begin(), sortedObjects.end(),
			[](const ResourcePathUtils::ObjectResourceDef* a, const ResourcePathUtils::ObjectResourceDef* b) {
				return a->y < b->y;
			});
		objectsDirty = false;
	}

	// 레이어 비트맵 크기
	UINT layerWidth = m_objectLayerBitmap->GetWidth();
	UINT layerHeight = m_objectLayerBitmap->GetHeight();

	// 현재 뷰포트의 월드 좌표 (Client처럼 뷰포트 기반)
	Gdiplus::PointF viewTopLeft = m_pView->ScreenToWorld(Gdiplus::PointF(0, 0));
	Gdiplus::PointF viewBottomRight = m_pView->ScreenToWorld(Gdiplus::PointF((float)layerWidth, (float)layerHeight));
	const float CULL_MARGIN = 100.0f;
	Gdiplus::RectF viewWorldRect(
		viewTopLeft.X - CULL_MARGIN, viewTopLeft.Y - CULL_MARGIN,
		(viewBottomRight.X - viewTopLeft.X) + 2 * CULL_MARGIN,
		(viewBottomRight.Y - viewTopLeft.Y) + 2 * CULL_MARGIN
	);

	// Client CameraManager처럼: 뷰포트/오브젝트 목록이 바뀐 경우에만 visible 캐시 갱신
	auto rectChanged = [](const Gdiplus::RectF& a, const Gdiplus::RectF& b, float threshold = 0.1f) {
		return std::abs(a.X - b.X) > threshold || std::abs(a.Y - b.Y) > threshold ||
		       std::abs(a.Width - b.Width) > threshold || std::abs(a.Height - b.Height) > threshold;
	};
	
	bool viewportChanged = rectChanged(viewWorldRect, lastViewportWorldRect);
	if (hadObjectsDirty || viewportChanged || visibleObjectsCache.empty()) {
		lastViewportWorldRect = viewWorldRect;
		visibleObjectsCache.clear();
		visibleObjectsCache.reserve(sortedObjects.size() / 4 + 32);  // 대략 보이는 비율만 예약
		for (const ResourcePathUtils::ObjectResourceDef* objData : sortedObjects) {
			if (objData->imageName.empty()) continue;
			
			// 캐시된 비트맵 로드하여 크기 확인
			std::wstring fullPath = objData->baseDir;
			if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
				fullPath += L"\\";
			}
			fullPath += objData->imageName;
			std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
			if (!pBitmap) continue;
			
			float objWidth = (float)pBitmap->GetWidth();
			float objHeight = (float)pBitmap->GetHeight();
			float objRenderLeftWorld = (float)objData->x - (objData->pivotX * objWidth);
			float objRenderTopWorld = (float)objData->y - (objData->pivotY * objHeight);
			Gdiplus::RectF objWorldRect(objRenderLeftWorld, objRenderTopWorld, objWidth, objHeight);
			
			if (objWorldRect.IntersectsWith(viewWorldRect))
				visibleObjectsCache.push_back(objData);
		}
	}

	// 보이는 오브젝트만 그리기 (Client RenderVisibleGameObjects와 동일 사상)
	for (const ResourcePathUtils::ObjectResourceDef* objData : visibleObjectsCache) {
		if (objData->imageName.empty()) continue;
		
		// 캐시된 비트맵 로드
		std::wstring fullPath = objData->baseDir;
		if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
			fullPath += L"\\";
		}
		fullPath += objData->imageName;
		std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
		if (!pBitmap) continue;

		float objWidth = (float)pBitmap->GetWidth();
		float objHeight = (float)pBitmap->GetHeight();
		float objRenderLeftWorld = (float)objData->x - (objData->pivotX * objWidth);
		float objRenderTopWorld = (float)objData->y - (objData->pivotY * objHeight);

		float screenX = (objRenderLeftWorld - viewTopLeft.X) * m_pView->GetZoomFactor();
		float screenY = (objRenderTopWorld - viewTopLeft.Y) * m_pView->GetZoomFactor();
		float screenWidth = objWidth * m_pView->GetZoomFactor();
		float screenHeight = objHeight * m_pView->GetZoomFactor();

		if (screenX + screenWidth < 0 || screenX > layerWidth ||
			screenY + screenHeight < 0 || screenY > layerHeight) continue;

		Gdiplus::RectF destRect(screenX, screenY, screenWidth, screenHeight);
		pGraphics->DrawImage(pBitmap.get(), destRect, 0, 0, objWidth, objHeight, Gdiplus::UnitPixel);

		if (selectedObjectPtr == objData) {
			Gdiplus::Pen selectedPen(Gdiplus::Color(255, 255, 0, 0), 3.0f);
			pGraphics->DrawRectangle(&selectedPen, destRect);
		}
	}
}

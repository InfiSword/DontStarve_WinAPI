#include "../../99_Default/pch.h"
#include "CameraManager.h"
#include "../InputManager/InputManager.h" 
#include "../ObjectManager/ObjectManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../RenderManager/RenderManager.h"
#include "../../02_GameObject/GameObject/GameObject.h"
#include "../../02_GameObject/Player/Player.h"

CameraManager::CameraManager()
    : m_cameraPos({ 0,0 }), m_zoomFactor(1.0f), m_target(nullptr),
    m_followMode(true), m_viewportChanged(true),
    m_lastStartTileX(0), m_lastStartTileY(0), m_lastEndTileX(0), m_lastEndTileY(0),
    m_tileViewportChanged(true)
{
}

CameraManager::~CameraManager()
{
	Release();
}

void CameraManager::Init()
{
    m_cameraPos = { 0,0 }; 
    m_zoomFactor = 1.0f; 
    m_viewportChanged = true;
    m_visibleObjects.clear();
    m_visibleObjectSet.clear();
    
    m_tileCache.clear();
    m_visibleTileIndices.clear();
    m_tileViewportChanged = true;
}

void CameraManager::LateInit()
{

}

void CameraManager::Update(float deltaTime)
{
    if (!InputManager::GetInstance()) return;

    // 플레이어 추적 모드가 활성화되어 있으면 플레이어를 따라감
    if (m_followMode && m_target) {
        FollowTarget(deltaTime);
    }
    
    // 뷰포트 변경 감지
    CheckViewportChanged();
    
    // 뷰포트가 변경되었거나 처음 실행되는 경우에만 업데이트
    if (m_viewportChanged) {
        UpdateVisibleObjects();

    }
}

void CameraManager::LateUpdate()
{

}

void CameraManager::Render()
{

}

void CameraManager::Release()
{
	m_visibleObjects.clear();
	m_visibleObjectSet.clear();
	
	// 타일 캐시 정리
	ClearTileCache();
}

// 월드 좌표를 화면 픽셀 좌표로 변환
Gdiplus::PointF CameraManager::WorldToScreen(float worldX, float worldY)
{

    float transformedX = (worldX - m_cameraPos.X);
    float transformedY = (worldY - m_cameraPos.Y);

    transformedX += WINCX / 2.0f;
    transformedY += WINCY / 2.0f;

    return Gdiplus::PointF(transformedX, transformedY);
}

// 화면 픽셀 좌표를 월드 좌표로 변환
Gdiplus::PointF CameraManager::ScreenToWorld(float screenX, float screenY) 
{
    float uncenteredScreenX = screenX - WINCX / 2.0f;
    float uncenteredScreenY = screenY - WINCY / 2.0f;

    // 카메라 오프셋 역산
    float worldX = uncenteredScreenX + m_cameraPos.X;
    float worldY = uncenteredScreenY + m_cameraPos.Y;

    return Gdiplus::PointF(worldX, worldY);
}

Gdiplus::PointF CameraManager::GetCamerPos()
{
    return m_cameraPos;
}

// 뷰포트의 월드 좌표 범위를 반환
Gdiplus::RectF CameraManager::GetViewportWorldRect() const
{
    // 카메라 위치를 중심으로 한 뷰포트 계산
    float halfWidth = WINCX / 2.0f;
    float halfHeight = WINCY / 2.0f;
    
    float left = m_cameraPos.X - halfWidth;
    float top = m_cameraPos.Y - halfHeight;
    float right = m_cameraPos.X + halfWidth;
    float bottom = m_cameraPos.Y + halfHeight;
    
    return Gdiplus::RectF(
        left,
        top,
        right - left,
        bottom - top
    );
}


void CameraManager::SetTarget(GameObject* target)
{
    m_target = target;
}

const GameObject* CameraManager::GetTarget()
{
    return m_target;
}

void CameraManager::FollowTarget(float deltaTime)
{
	if (!m_target)
		return;
	// 플레이어의 (x, y)가 발 밑 중앙이면, 그대로 사용
	m_cameraPos.X = m_target->GetX();
	m_cameraPos.Y = m_target->GetY();
}

void CameraManager::SetCameraPosition(float x, float y)
{
    m_cameraPos.X = x;
    m_cameraPos.Y = y;
}

// === 화면에 보이는 오브젝트 관리 기능 (ViewportManager 통합) ===

void CameraManager::UpdateVisibleObjects()
{
	m_visibleObjects.clear();
	m_visibleObjectSet.clear();
	
	// ObjectManager에서 모든 오브젝트 가져오기 (공유 포인터)
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;
	
	const std::vector<GameObject*>& allObjects = objectManager->GetGameObjects();
	
	// 현재 뷰포트 정보 가져오기
	Gdiplus::RectF viewportRect = GetViewportWorldRect();
	
	// 여유 공간을 포함한 검사 범위 (오브젝트 크기를 고려하여 여유 공간 추가)
	const float MARGIN = 200.0f; // 충분한 여유 공간
	float startX = viewportRect.X - MARGIN;
	float endX = viewportRect.X + viewportRect.Width + MARGIN;
	float startY = viewportRect.Y - MARGIN;
	float endY = viewportRect.Y + viewportRect.Height + MARGIN;
	
	// 활성화된 오브젝트 중에서 화면에 보이는 것만 필터링
	
	// 중복 체크를 위한 임시 set (문자열 키 사용)
	std::unordered_set<std::wstring> addedIngredients;
	
	for (GameObject* obj : allObjects) {
		if (obj && obj->GetActive()) {
			// Ingredient인 경우 중복 체크
			if (obj->GetType() == GOBJ_ITEM) {
				std::wstring ingredientKey = std::to_wstring(obj->GetID()) + L"_" + 
					std::to_wstring(obj->GetX()) + L"_" + std::to_wstring(obj->GetY());
				if (addedIngredients.find(ingredientKey) != addedIngredients.end()) {
					continue;
				}
				addedIngredients.insert(ingredientKey);
			}
			
			// 오브젝트의 월드 바운딩 박스 계산
			Gdiplus::RectF objBounds = obj->GetWorldBoundingBox();
			
			// 화면 범위와 겹치는지 확인
			if (objBounds.X < endX && objBounds.X + objBounds.Width > startX &&
				objBounds.Y < endY && objBounds.Y + objBounds.Height > startY) {
				
				m_visibleObjects.push_back(obj);
				m_visibleObjectSet.insert(obj);
				
				// 디버그 출력 (너무 자주 출력하지 않도록 제한)
				static int debugCounter = 0;
				if (++debugCounter % 100 == 0) {
					OutputDebugStringW((L"CameraManager: 보이는 오브젝트 추가 - ID: " + 
						std::to_wstring(obj->GetID()) + L", 위치: (" + 
						std::to_wstring(obj->GetX()) + L", " + std::to_wstring(obj->GetY()) + 
						L"), 뷰포트 내 오브젝트 수: " + std::to_wstring(m_visibleObjects.size()) + L"\n").c_str());
				}
			}
		}
	}
	

}

GameObject* CameraManager::FindObjectAtPosition(float worldX, float worldY)
{
	
	// 화면에 보이는 오브젝트만 검사 (성능 최적화)
	for (int i = (int)m_visibleObjects.size() - 1; i >= 0; --i) {
		GameObject* obj = m_visibleObjects[i];
		if (!obj) {
			continue;
		}
		
		// 상호작용 가능한 오브젝트인지 먼저 확인
		if (!obj->CanInteract()) {
			continue;
		}
		
		// 오브젝트의 월드 바운딩 박스 계산
		Gdiplus::RectF objBounds = obj->GetWorldBoundingBox();
		
		// 클릭한 위치가 오브젝트 영역 안에 있는지 확인
		if (objBounds.Contains(worldX, worldY)) {
			return obj;
		}
	}
	
	return nullptr;
}

bool CameraManager::IsObjectVisible(GameObject* obj) const
{
	return m_visibleObjectSet.find(obj) != m_visibleObjectSet.end();
}

bool CameraManager::IsObjectInViewport(GameObject* obj) const
{
	if (!obj || !obj->GetActive()) return false;
	
	// 오브젝트의 월드 바운딩 박스 계산
	Gdiplus::RectF objBounds = obj->GetWorldBoundingBox();
	
	// 현재 뷰포트 정보 가져오기
	Gdiplus::RectF viewportRect = GetViewportWorldRect();
	
	// 여유 공간을 포함한 검사 범위
	const float MARGIN = 200.0f;
	float startX = viewportRect.X - MARGIN;
	float endX = viewportRect.X + viewportRect.Width + MARGIN;
	float startY = viewportRect.Y - MARGIN;
	float endY = viewportRect.Y + viewportRect.Height + MARGIN;
	
	// 화면 범위와 겹치는지 확인
	return (objBounds.X < endX && objBounds.X + objBounds.Width > startX &&
			objBounds.Y < endY && objBounds.Y + objBounds.Height > startY);
}

void CameraManager::CheckViewportChanged()
{
	Gdiplus::RectF currentViewport = GetViewportWorldRect();
	
	// 뷰포트가 변경되었는지 확인
	if (currentViewport.X != m_lastViewportRect.X || 
		currentViewport.Y != m_lastViewportRect.Y ||
		currentViewport.Width != m_lastViewportRect.Width || 
		currentViewport.Height != m_lastViewportRect.Height) {
		
		m_viewportChanged = true;
		m_lastViewportRect = currentViewport;
	}
}

// === 타일 렌더링 관리 기능 ===

void CameraManager::RenderVisibleTiles(RenderManager* renderManager, const MapData* mapData)
{
	if (!mapData || !renderManager) {
		return;
	}

	// 기존 CheckViewportChanged() 함수의 결과를 사용
	// Update()에서 이미 호출되어 m_viewportChanged가 설정됨
	if (m_viewportChanged) {
		m_tileViewportChanged = true;
	}

	// 뷰포트가 변경되지 않았으면 이전 범위 사용
	if (!m_tileViewportChanged) {
		// 이전에 계산된 범위로 렌더링
		for (int y = m_lastStartTileY; y < m_lastEndTileY; ++y) {
			float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
			for (int x = m_lastStartTileX; x < m_lastEndTileX; ++x) {
				RenderSingleTile(renderManager, mapData, x, y, worldY);
			}
		}
		return;
	}

	// 뷰포트가 변경되었을 때만 새로운 범위 계산
	Gdiplus::RectF currentViewport = GetViewportWorldRect();
	const float MARGIN = TILE_SIZE;
	float startX = currentViewport.X - MARGIN;
	float endX = currentViewport.X + currentViewport.Width + MARGIN;
	float startY = currentViewport.Y - MARGIN;
	float endY = currentViewport.Y + currentViewport.Height + MARGIN;

	// 타일 인덱스 범위 계산
	int startTileX = max(0, (int)floor(startX / TILE_SIZE));
	int endTileX = min(MAP_WIDTH, (int)ceil(endX / TILE_SIZE));
	int startTileY = max(0, (int)floor(startY / TILE_SIZE));
	int endTileY = min(MAP_HEIGHT, (int)ceil(endY / TILE_SIZE));

	// 범위 업데이트
	m_lastStartTileX = startTileX;
	m_lastStartTileY = startTileY;
	m_lastEndTileX = endTileX;
	m_lastEndTileY = endTileY;
	m_tileViewportChanged = false;

	// 최적화된 타일 렌더링
	for (int y = startTileY; y < endTileY; ++y) {
		float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
		for (int x = startTileX; x < endTileX; ++x) {
			RenderSingleTile(renderManager, mapData, x, y, worldY);
		}
	}
}

// 개별 타일 렌더링 헬퍼 함수
void CameraManager::RenderSingleTile(RenderManager* renderManager, const MapData* mapData, int x, int y, float worldY)
{
	const TileData& tileData = mapData->tiles[x][y];
	
	// 빈 타일이나 TILE_NONE인 타일 스킵
	if (tileData.id == TILEID_NONE || tileData.type == TILE_NONE) {
		return;
	}

	// 타일 캐시에서 데이터 가져오기 (최적화된 검색)
	auto cacheIt = m_tileCache.find(tileData.id);
	if (cacheIt == m_tileCache.end()) {
		// 캐시에 없는 타일만 로드
		TileCacheData newCacheData;
		newCacheData.id = tileData.id;
		LoadTileBitmap(tileData.id, newCacheData);
		if (newCacheData.bitmap) {
			m_tileCache[tileData.id] = newCacheData;
			cacheIt = m_tileCache.find(tileData.id);
		}
		else {
			return; // 로드 실패한 타일은 스킵
		}
	}

	TileCacheData& cacheData = cacheIt->second;
	Gdiplus::Bitmap* tileBitmap = cacheData.bitmap;

	// 비트맵이 로드되지 않은 경우만 로드
	if (!tileBitmap) {
		LoadTileBitmap(tileData.id, cacheData);
		tileBitmap = cacheData.bitmap;
		if (!tileBitmap) return;
	}

	// 타일 렌더링
	float worldX = x * TILE_SIZE + TILE_SIZE / 2.0f;
	renderManager->RenderTile(tileBitmap, worldX, worldY, TILE_SIZE, TILE_SIZE);
}

void CameraManager::ClearTileCache()
{
	// 타일 캐시의 비트맵들 해제
	for (auto& pair : m_tileCache) {
		if (pair.second.bitmap) {
			delete pair.second.bitmap;
			pair.second.bitmap = nullptr;
		}
	}
	m_tileCache.clear();
	m_visibleTileIndices.clear();
}

void CameraManager::LoadTileBitmap(TileID tileID, TileCacheData& cacheData)
{
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	const TileData* resourceTile = resourceManager->GetTileResourceInfo(tileID);
	if (resourceTile) {
		// TileData의 tileImageName을 사용하여 경로 구성
		std::wstring fullPath = resourceManager->BuildTileResourcePath(tileID, L"", resourceTile->tileImageName);
		
		cacheData.bitmap = new Gdiplus::Bitmap(fullPath.c_str());
		if (cacheData.bitmap && cacheData.bitmap->GetLastStatus() == Gdiplus::Ok) {
			cacheData.isAtlasBased = false;
		}
		else {
			delete cacheData.bitmap;
			cacheData.bitmap = nullptr;
		}
	}
}
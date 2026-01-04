#include "../../99_Default/pch.h"
#include "CameraManager.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Transform/RectTransform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Sprite/Image.h"
#include "../InputManager/InputManager.h" 
#include "../ObjectManager/ObjectManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../RenderManager/RenderManager.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/GameObject.h"

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

    // 플레이어 추적 모드가 활성화되어 있으면 플레이어를 추적
    if (m_followMode && m_target) {
        FollowTarget(deltaTime);
    }
    
    // 뷰포트 변경 확인
    CheckViewportChanged();
    
    // 카메라 뷰포트 내에 있는 게임오브젝트를 찾아서 저장
    // 게임오브젝트 생성/삭제/이동으로 인한 변경 처리
    UpdateVisibleObjects();
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
	
	// 타일 캐시 해제
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

    // 카메라 오프셋을 더함
    float worldX = uncenteredScreenX + m_cameraPos.X;
    float worldY = uncenteredScreenY + m_cameraPos.Y;

    return Gdiplus::PointF(worldX, worldY);
}

Gdiplus::PointF CameraManager::GetCameraPos()
{
    return m_cameraPos;
}

// 뷰포트의 월드 좌표 영역을 반환
Gdiplus::RectF CameraManager::GetViewportWorldRect() const
{
    // 카메라 위치를 중심으로 한 뷰포트 영역
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
	Transform* transform = m_target->GetComponent<Transform>();
	m_cameraPos.X = transform->GetX();
	m_cameraPos.Y = transform->GetY();
}

void CameraManager::SetCameraPosition(float x, float y)
{
    m_cameraPos.X = x;
    m_cameraPos.Y = y;
}

// === 화면에 보이는 게임오브젝트 관리 기능 ===

void CameraManager::UpdateVisibleObjects()
{
	m_visibleObjects.clear();
	m_visibleObjectSet.clear();
	
	// ObjectManager에서 모든 게임오브젝트 가져오기
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;
	
	const std::vector<GameObject*>& allObjects = objectManager->GetGameObjects();
	if (allObjects.empty()) return;
	
	// 현재 뷰포트 영역 계산하기
	Gdiplus::RectF viewportRect = GetViewportWorldRect();
	
	// 넓은 범위의 검색 영역 (게임오브젝트 크기나 애니메이션 범위 고려)
	const float MARGIN = 200.0f;
	float startX = viewportRect.X - MARGIN;
	float endX = viewportRect.X + viewportRect.Width + MARGIN;
	float startY = viewportRect.Y - MARGIN;
	float endY = viewportRect.Y + viewportRect.Height + MARGIN;
	
	// 중복 체크를 위한 임시 set (같은 아이템 중복 방지)
	std::unordered_set<std::wstring> addedIngredients;
	
	// 활성화된 게임오브젝트 중에서 화면에 보이는 것만 추가
	for (GameObject* obj : allObjects) {
		if (!obj || !obj->IsEnabled()) {
			continue;
		}
		SpriteRenderer* spriteRenderer = obj->GetComponent<SpriteRenderer>();
		Transform* transform = obj->GetComponent<Transform>();
		// 렌더링 가능한 게임오브젝트인지 확인
		// Player는 항상 Animator를 가지고 있으므로 항상 확인
		if (!spriteRenderer->GetSprite() && !obj->GetComponent<Animator>()) {
			continue;
		}
		
		// Ingredient의 중복 체크
		if (obj->GetType() == GOBJ_ITEM) {
			std::wstring ingredientKey = std::to_wstring(obj->GetID()) + L"_" + 
				std::to_wstring(transform->GetX()) + L"_" + std::to_wstring(transform->GetY());
			if (addedIngredients.find(ingredientKey) != addedIngredients.end()) {
				continue;
			}
			addedIngredients.insert(ingredientKey);
		}
		
		// Sprite 크기 기반 바운딩 박스 계산
		Gdiplus::RectF objBounds = GetSpriteBoundingBox(obj);
		
		// 화면 영역과 위치 겹침 확인 (AABB 충돌 검사)
		if (objBounds.X < endX && objBounds.X + objBounds.Width > startX &&
			objBounds.Y < endY && objBounds.Y + objBounds.Height > startY) {
			
			m_visibleObjects.push_back(obj);
			m_visibleObjectSet.insert(obj);
		}
	}
	
	// 디버그: 현재 게임오브젝트 개수 확인 (30프레임마다 한 번씩만 출력)
	static int updateCounter = 0;
	if (++updateCounter % 30 == 0) {
		OutputDebugStringW((L"[CameraManager] 현재 게임오브젝트 개수: " + 
			std::to_wstring(m_visibleObjects.size()) + L"개 (카메라: " + 
			std::to_wstring((int)m_cameraPos.X) + L", " + std::to_wstring((int)m_cameraPos.Y) + L")\n").c_str());
	}
}

GameObject* CameraManager::FindObjectAtPosition(float worldX, float worldY)
{
	
	// 화면에 보이는 게임오브젝트만 검색 (성능 최적화)
	for (int i = (int)m_visibleObjects.size() - 1; i >= 0; --i) {
		GameObject* obj = m_visibleObjects[i];
		if (!obj) {
			continue;
		}
		
		// 상호작용 가능한 게임오브젝트만 확인
		if (!obj->IsEnabled()) {
			continue;
		}
		// Sprite 크기 기반 바운딩 박스 계산
		Gdiplus::RectF objBounds = GetSpriteBoundingBox(obj);
		
		// 클릭한 위치가 게임오브젝트 영역 안에 있는지 확인
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

Gdiplus::RectF CameraManager::GetSpriteBoundingBox(GameObject* obj) const
{
	if (!obj || !obj->IsEnabled()) {
		return Gdiplus::RectF(0, 0, 0, 0);
	}
	
	Transform* transform = obj->GetComponent<Transform>();
	if (!transform) {
		return Gdiplus::RectF(0, 0, 0, 0);
	}
	
	float worldX = transform->GetX();
	float worldY = transform->GetY();
	float width = 0.0f;
	float height = 0.0f;
	float pivotX = 0.5f;
	float pivotY = 0.5f;
	
	// Animator가 있으면 currentFrame의 크기 사용
	Animator* animator = obj->GetComponent<Animator>();
	if (animator && animator->GetClip()) {
		const AnimationFrame& currentFrame = animator->GetCurrentFrame();
		width = static_cast<float>(currentFrame.width);
		height = static_cast<float>(currentFrame.height);
		pivotX = currentFrame.pivotX;
		pivotY = currentFrame.pivotY;
	}
	// SpriteRenderer만 있으면 비트맵 크기 사용
	else {
		SpriteRenderer* spriteRenderer = obj->GetComponent<SpriteRenderer>();
		if (spriteRenderer && spriteRenderer->GetSprite()) {
			Gdiplus::Bitmap* bitmap = spriteRenderer->GetSprite();
			width = static_cast<float>(bitmap->GetWidth());
			height = static_cast<float>(bitmap->GetHeight());
			pivotX = transform->GetPivotX();
			pivotY = transform->GetPivotY();
		}
		// 둘 다 없으면 기본값 사용 (fallback)
		else {
			width = 32.0f;  // 기본 크기
			height = 32.0f;
			pivotX = transform->GetPivotX();
			pivotY = transform->GetPivotY();
		}
	}
	
	// 피벗을 고려한 바운딩 박스 계산
	float left = worldX - width * pivotX;
	float top = worldY - height * pivotY;
	
	return Gdiplus::RectF(left, top, width, height);
}

bool CameraManager::IsObjectInViewport(GameObject* obj) const
{
	if (!obj || !obj->IsEnabled()) return false;
	
	// Sprite 크기 기반 바운딩 박스 사용
	Gdiplus::RectF objBounds = GetSpriteBoundingBox(obj);
	
	// 현재 뷰포트 영역 계산하기
	Gdiplus::RectF viewportRect = GetViewportWorldRect();
	
	// 넓은 범위의 검색 영역
	const float MARGIN = 200.0f;
	float startX = viewportRect.X - MARGIN;
	float endX = viewportRect.X + viewportRect.Width + MARGIN;
	float startY = viewportRect.Y - MARGIN;
	float endY = viewportRect.Y + viewportRect.Height + MARGIN;
	
	// 화면 영역과 위치 겹침 확인
	return (objBounds.X < endX && objBounds.X + objBounds.Width > startX &&
			objBounds.Y < endY && objBounds.Y + objBounds.Height > startY);
}

void CameraManager::CheckViewportChanged()
{
	Gdiplus::RectF currentViewport = GetViewportWorldRect();
	
	// 뷰포트가 변경되었는지 확인 (카메라 위치나 크기가 변경되었을 때)
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

	// 이미 CheckViewportChanged() 함수에서 처리된 결과
	// Update()에서 이미 호출되어 m_viewportChanged가 설정됨
	if (m_viewportChanged) {
		m_tileViewportChanged = true;
		m_viewportChanged = false; // 타일 렌더링에서 사용 후 클리어
	}

	// 뷰포트가 변경되지 않았으면 캐시된 타일 렌더링
	if (!m_tileViewportChanged) {
		// 이전에 계산된 타일 범위 렌더링
		for (int y = m_lastStartTileY; y < m_lastEndTileY; ++y) {
			float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
			for (int x = m_lastStartTileX; x < m_lastEndTileX; ++x) {
				RenderSingleTile(renderManager, mapData, x, y, worldY);
			}
		}
		return;
	}

	// 뷰포트가 변경되었으면 새로운 타일 렌더링
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

	// 캐시 업데이트
	m_lastStartTileX = startTileX;
	m_lastStartTileY = startTileY;
	m_lastEndTileX = endTileX;
	m_lastEndTileY = endTileY;
	m_tileViewportChanged = false;

	// 캐시된 타일 렌더링
	for (int y = startTileY; y < endTileY; ++y) {
		float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
		for (int x = startTileX; x < endTileX; ++x) {
			RenderSingleTile(renderManager, mapData, x, y, worldY);
		}
	}
}

// 단일 타일 렌더링 관련 함수
void CameraManager::RenderSingleTile(RenderManager* renderManager, const MapData* mapData, int x, int y, float worldY)
{
	const TileData& tileData = mapData->tiles[x][y];
	
	// 빈 타일이거나 TILE_NONE 타입은 스킵
	if (tileData.id == TILEID_NONE || tileData.type == TILE_NONE) {
		return;
	}

	// 타일 캐시에서 비트맵 찾기 (캐시된 타일만 검색)
	auto cacheIt = m_tileCache.find(tileData.id);
	if (cacheIt == m_tileCache.end()) {
		// 캐시에 없으면 타일 로드
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

	// 비트맵이 로드되지 않았으면 다시 로드 시도
	// 로드 실패 시 캐시에서 제거하여 매 프레임마다 재시도하지 않도록 최적화
	if (!tileBitmap) {
		LoadTileBitmap(tileData.id, cacheData);
		tileBitmap = cacheData.bitmap;
		if (!tileBitmap) {
			// 로드 실패한 타일은 캐시에서 제거 (최적화)
			m_tileCache.erase(cacheIt);
			return;
		}
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
		// TileData의 tileImageName을 사용하여 경로 생성
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

#include "99_Default/pch.h"
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
    m_tileViewportChanged = true;
}

void CameraManager::Update(float deltaTime)
{
    if (!InputManager::GetInstance()) return;

    // 플레이어 추적 모드가 활성화되어 있으면 플레이어를 추적
    if (m_followMode && m_target) {
        FollowTarget();
    }
    
    // 뷰포트 변경 확인
    CheckViewportChanged();
    
    // 카메라 뷰포트 내에 있는 게임오브젝트를 찾아서 저장
    // 게임오브젝트 생성/삭제/이동으로 인한 변경 처리
    UpdateVisibleObjects();
}

void CameraManager::Release()
{
	m_visibleObjects.clear();
	m_visibleObjectSet.clear();
	
	// 타일 캐시 해제
	ClearTileCache();
}

// 월드 좌표를 화면 픽셀 좌표로 변환
Gdiplus::PointF CameraManager::WorldToScreen(float worldX, float worldY) const
{

    float transformedX = (worldX - m_cameraPos.X);
    float transformedY = (worldY - m_cameraPos.Y);

    transformedX += WINCX / 2.0f;
    transformedY += WINCY / 2.0f;

    return Gdiplus::PointF(transformedX, transformedY);
}

// 화면 픽셀 좌표를 월드 좌표로 변환
Gdiplus::PointF CameraManager::ScreenToWorld(float screenX, float screenY) const 
{
    float uncenteredScreenX = screenX - WINCX / 2.0f;
    float uncenteredScreenY = screenY - WINCY / 2.0f;

    // 카메라 오프셋을 더함
    float worldX = uncenteredScreenX + m_cameraPos.X;
    float worldY = uncenteredScreenY + m_cameraPos.Y;

    return Gdiplus::PointF(worldX, worldY);
}

Gdiplus::PointF CameraManager::GetCameraPos() const
{
    return m_cameraPos;
}

Gdiplus::RectF CameraManager::GetViewportWorldRect() const
{
    float halfWidth = WINCX / 2.0f;
    float halfHeight = WINCY / 2.0f;
    float left = m_cameraPos.X - halfWidth;
    float top = m_cameraPos.Y - halfHeight;
    return Gdiplus::RectF(left, top, WINCX, WINCY);
}


void CameraManager::SetTarget(GameObject* target)
{
    m_target = target;
}

const GameObject* CameraManager::GetTarget() const
{
    return m_target;
}

void CameraManager::FollowTarget()
{
	if (!m_target) return;
	Transform* transform = m_target->GetComponent<Transform>();
	if (transform) {
		m_cameraPos.X = transform->GetX();
		m_cameraPos.Y = transform->GetY();
	}
}

void CameraManager::SetCameraPosition(float x, float y)
{
    m_cameraPos.X = x;
    m_cameraPos.Y = y;
}

void CameraManager::UpdateVisibleObjects()
{
	m_visibleObjects.clear();
	m_visibleObjectSet.clear();
	
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;
	
	const std::vector<GameObject*>& allObjects = objectManager->GetGameObjects();
	if (allObjects.empty()) return;
	
	Gdiplus::RectF viewportRect = GetViewportWorldRect();
	const float MARGIN = 200.0f;
	float startX = viewportRect.X - MARGIN;
	float endX = viewportRect.X + viewportRect.Width + MARGIN;
	float startY = viewportRect.Y - MARGIN;
	float endY = viewportRect.Y + viewportRect.Height + MARGIN;
	
	std::unordered_set<std::wstring> addedIngredients; // Ingredient 중복 방지
	
	for (GameObject* obj : allObjects) {
		if (!obj || !obj->IsEnabled()) continue;
		
		Transform* transform = obj->GetComponent<Transform>();
		if (!transform) continue;
		
		// 렌더링 가능한 오브젝트인지 확인
		SpriteRenderer* spriteRenderer = obj->GetComponent<SpriteRenderer>();
		bool hasSprite = (spriteRenderer && spriteRenderer->GetSprite());
		bool hasAnimator = (obj->GetComponent<Animator>() != nullptr);
		if (!hasSprite && !hasAnimator) continue;
		
		// Ingredient 중복 체크
		if (obj->GetType() == GOBJ_ITEM) {
			std::wstring key = std::to_wstring(obj->GetID()) + L"_" + 
				std::to_wstring(transform->GetX()) + L"_" + std::to_wstring(transform->GetY());
			if (addedIngredients.find(key) != addedIngredients.end()) continue;
			addedIngredients.insert(key);
		}
		
		// 뷰포트 내에 있는지 확인
		Gdiplus::RectF objBounds = GetSpriteBoundingBox(obj);
		if (objBounds.X < endX && objBounds.X + objBounds.Width > startX &&
			objBounds.Y < endY && objBounds.Y + objBounds.Height > startY) {
			m_visibleObjects.push_back(obj);
			m_visibleObjectSet.insert(obj);
		}
	}
}

GameObject* CameraManager::FindObjectAtPosition(float worldX, float worldY)
{
	// 화면에 보이는 게임오브젝트만 검색 (성능 최적화)
	for (int i = (int)m_visibleObjects.size() - 1; i >= 0; --i) {
		GameObject* obj = m_visibleObjects[i];
		if (!obj || !obj->IsEnabled()) {
			continue;
		}
		
		Gdiplus::RectF objBounds = GetSpriteBoundingBox(obj);
		if (objBounds.Contains(worldX, worldY)) {
			return obj;
		}
	}
	return nullptr;
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

void CameraManager::CheckViewportChanged()
{
	Gdiplus::RectF currentViewport = GetViewportWorldRect();
	if (currentViewport.X != m_lastViewportRect.X || 
		currentViewport.Y != m_lastViewportRect.Y ||
		currentViewport.Width != m_lastViewportRect.Width || 
		currentViewport.Height != m_lastViewportRect.Height) {
		m_viewportChanged = true;
		m_lastViewportRect = currentViewport;
	}
}

void CameraManager::RenderVisibleTiles(const MapData* mapData)
{
	if (!mapData) {
		return;
	}

	RenderManager* renderManager = RenderManager::GetInstance();
	if (!renderManager) return;

	// 뷰포트 변경 확인
	if (m_viewportChanged) {
		m_tileViewportChanged = true;
		m_viewportChanged = false;
	}

	// 뷰포트가 변경되지 않았으면 캐시된 타일 렌더링
	if (!m_tileViewportChanged) {
		for (int y = m_lastStartTileY; y < m_lastEndTileY; ++y) {
			float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
			for (int x = m_lastStartTileX; x < m_lastEndTileX; ++x) {
				RenderSingleTile(mapData, x, y, worldY);
			}
		}
		return;
	}

	// 뷰포트 변경 시 새로운 타일 범위 계산
	Gdiplus::RectF viewport = GetViewportWorldRect();
	const float MARGIN = TILE_SIZE;
	float startX = viewport.X - MARGIN;
	float endX = viewport.X + viewport.Width + MARGIN;
	float startY = viewport.Y - MARGIN;
	float endY = viewport.Y + viewport.Height + MARGIN;

	int startTileX = max(0, (int)floor(startX / TILE_SIZE));
	int endTileX = min(MAP_WIDTH, (int)ceil(endX / TILE_SIZE));
	int startTileY = max(0, (int)floor(startY / TILE_SIZE));
	int endTileY = min(MAP_HEIGHT, (int)ceil(endY / TILE_SIZE));

	m_lastStartTileX = startTileX;
	m_lastStartTileY = startTileY;
	m_lastEndTileX = endTileX;
	m_lastEndTileY = endTileY;
	m_tileViewportChanged = false;

	// 타일 렌더링
	for (int y = startTileY; y < endTileY; ++y) {
		float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
		for (int x = startTileX; x < endTileX; ++x) {
			RenderSingleTile(mapData, x, y, worldY);
		}
	}
}

void CameraManager::RenderVisibleGameObjects()
{
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!renderManager) return;

	for (GameObject* obj : m_visibleObjects) {
		if (!obj || !obj->IsEnabled() || obj->GetType() == GOBJ_UI) {
			continue;
		}
		renderManager->RenderGameObject(obj);
	}
}

void CameraManager::RenderSingleTile(const MapData* mapData, int x, int y, float worldY)
{
	const TileData& tileData = mapData->tiles[x][y];
	if (tileData.id == TILEID_NONE || tileData.type == TILE_NONE) {
		return;
	}

	// 타일 캐시에서 비트맵 찾기
	auto cacheIt = m_tileCache.find(tileData.id);
	if (cacheIt == m_tileCache.end()) {
		TileCacheData newCacheData;
		newCacheData.id = tileData.id;
		LoadTileBitmap(tileData.id, newCacheData);
		if (newCacheData.bitmap) {
			m_tileCache[tileData.id] = newCacheData;
			cacheIt = m_tileCache.find(tileData.id);
		} else {
			return;
		}
	}

	Gdiplus::Bitmap* tileBitmap = cacheIt->second.bitmap;
	if (!tileBitmap) {
		LoadTileBitmap(tileData.id, cacheIt->second);
		tileBitmap = cacheIt->second.bitmap;
		if (!tileBitmap) {
			m_tileCache.erase(cacheIt);
			return;
		}
	}

	// 타일 렌더링
	RenderManager* renderManager = RenderManager::GetInstance();
	if (renderManager) {
		float worldX = x * TILE_SIZE + TILE_SIZE / 2.0f;
		renderManager->RenderTile(tileBitmap, worldX, worldY, TILE_SIZE, TILE_SIZE);
	}
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

#include "99_Default/pch.h"
#include "CameraManager.h"
#include <set>
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Collider/Collider.h"
#include "../InputManager/InputManager.h" 
#include "../ObjectManager/ObjectManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../RenderManager/RenderManager.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/GameObject.h"

CameraManager::CameraManager()
    : m_cameraPos({ 0,0 }), m_target(nullptr), m_followMode(true), m_viewportChanged(true),
    m_lastStartTileX(0), m_lastStartTileY(0), m_lastEndTileX(0), m_lastEndTileY(0),
    m_tileRangeInitialized(false)
{
}

CameraManager::~CameraManager()
{
	Release();
}

void CameraManager::Init()
{
    m_cameraPos = { 0,0 };
    m_viewportChanged = true;
    m_visibleObjects.clear();
    m_visibleObjectSet.clear();
    m_tileCache.clear();
    m_tileRangeInitialized = false;
}

void CameraManager::Update(float deltaTime)
{
    // 플레이어 추적 모드가 활성화되어 있으면 플레이어를 추적
    if (m_followMode && m_target) {
        FollowTarget();
    }
    
    // 뷰포트 변경 확인
    CheckViewportChanged();
    
    // 카메라 뷰포트 내에 있는 게임오브젝트를 찾아서 저장
    // 뷰포트가 변경되었을 때만 업데이트 (성능 최적화)
    if (m_viewportChanged) {
        UpdateVisibleObjects();
    }
}

void CameraManager::Release()
{
	m_visibleObjects.clear();
	m_visibleObjects.shrink_to_fit();
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
		float newX = transform->GetX();
		float newY = transform->GetY();
		// 부동소수점 비교를 사용하여 미세한 변경은 무시 (성능 최적화)
		const float EPSILON = 0.1f;
		if (std::abs(newX - m_cameraPos.X) > EPSILON || std::abs(newY - m_cameraPos.Y) > EPSILON) {
			m_cameraPos.X = newX;
			m_cameraPos.Y = newY;
		}
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
	
	std::unordered_set<size_t> addedIngredients; // Ingredient 중복 방지 (성능 최적화: 문자열 대신 해시 사용)
	
	// 성능 최적화: 미리 reserve로 메모리 재할당 방지
	m_visibleObjects.reserve(allObjects.size() / 4); // 예상 크기의 1/4 정도로 예약
	for (GameObject* obj : allObjects) {
		if (!obj || !obj->IsEnabled()) continue;
		
		Transform* transform = obj->GetComponent<Transform>();
		if (!transform) continue;
		
		// 간단한 위치 체크로 먼저 필터링 (성능 최적화)
		float objX = transform->GetX();
		float objY = transform->GetY();
		if (objX < startX || objX > endX || objY < startY || objY > endY) {
			continue; // 뷰포트 밖이면 스킵
		}
		
		// 렌더링 가능한 오브젝트인지 확인
		SpriteRenderer* spriteRenderer = obj->GetComponent<SpriteRenderer>();
		bool hasSprite = (spriteRenderer && spriteRenderer->GetSprite());
		bool hasAnimator = (obj->GetComponent<Animator>() != nullptr);
		if (!hasSprite && !hasAnimator) continue;
		
		// Ingredient 중복 체크 (성능 최적화: 문자열 대신 해시 사용)
		if (obj->GetType() == GOBJ_ITEM) {
			// 해시 기반 키 생성 (문자열 변환보다 빠름)
			size_t key = std::hash<GameObjectID>()(obj->GetID()) ^ 
				(std::hash<float>()(objX) << 1) ^ 
				(std::hash<float>()(objY) << 2);
			if (addedIngredients.find(key) != addedIngredients.end()) continue;
			addedIngredients.insert(key);
		}
		
		// 정확한 바운딩 박스 체크 (필요한 경우에만)
		Gdiplus::RectF objBounds = GetSpriteBoundingBox(obj);
		bool boundsIntersect = (objBounds.X < endX && objBounds.X + objBounds.Width > startX &&
		                        objBounds.Y < endY && objBounds.Y + objBounds.Height > startY);
		
		if (!boundsIntersect) {
			continue; // 월드 좌표 기준으로 뷰포트 밖이면 스킵
		}
		
		// 화면 영역과의 실제 교차 체크 (월드 좌표 -> 화면 좌표 변환)
		// 바운딩 박스의 4개 모서리를 모두 화면 좌표로 변환하여 정확한 교차 체크
		Gdiplus::PointF screenTopLeft = WorldToScreen(objBounds.X, objBounds.Y);
		Gdiplus::PointF screenTopRight = WorldToScreen(objBounds.X + objBounds.Width, objBounds.Y);
		Gdiplus::PointF screenBottomLeft = WorldToScreen(objBounds.X, objBounds.Y + objBounds.Height);
		Gdiplus::PointF screenBottomRight = WorldToScreen(objBounds.X + objBounds.Width, objBounds.Y + objBounds.Height);
		
		// 4개 모서리로 화면 AABB 계산 후 화면 영역과 교차 여부 확인
		float screenLeft = (std::min)((std::min)(screenTopLeft.X, screenTopRight.X), (std::min)(screenBottomLeft.X, screenBottomRight.X));
		float screenRight = (std::max)((std::max)(screenTopLeft.X, screenTopRight.X), (std::max)(screenBottomLeft.X, screenBottomRight.X));
		float screenTop = (std::min)((std::min)(screenTopLeft.Y, screenTopRight.Y), (std::min)(screenBottomLeft.Y, screenBottomRight.Y));
		float screenBottom = (std::max)((std::max)(screenTopLeft.Y, screenTopRight.Y), (std::max)(screenBottomLeft.Y, screenBottomRight.Y));
		bool screenIntersects = !(screenRight < 0 || screenLeft > WINCX || screenBottom < 0 || screenTop > WINCY);
		
		// 화면 영역과 교차하는 경우에만 visibleObjects에 추가
		if (screenIntersects) {
			m_visibleObjects.push_back(obj);
			m_visibleObjectSet.insert(obj);
		}
	}
	
	// 성능 최적화: 업데이트 완료 후 플래그 리셋 (다음 프레임에서 불필요한 호출 방지)
	m_viewportChanged = false;
}

void CameraManager::RemoveFromVisibleObjects(GameObject* obj)
{
	if (!obj) return;
	auto setIt = m_visibleObjectSet.find(obj);
	if (setIt != m_visibleObjectSet.end()) {
		m_visibleObjectSet.erase(setIt);
	}
	auto it = std::find(m_visibleObjects.begin(), m_visibleObjects.end(), obj);
	if (it != m_visibleObjects.end()) {
		m_visibleObjects.erase(it);
	}
}

GameObject* CameraManager::FindInteractableObjectAtPosition(float worldX, float worldY)
{
	for (int i = (int)m_visibleObjects.size() - 1; i >= 0; --i) {
		GameObject* obj = m_visibleObjects[i];
		if (!obj || !obj->IsEnabled() || !obj->CanInteract()) continue;  // m_isInteractive가 false면 건너뜀 (플레이어 등)

		// 상호작용 범위: 콜라이더가 있고 활성화된 경우에만 콜라이더 영역으로 판정 (스프라이트 바운딩 미사용)
		Collider* collider = obj->GetComponent<Collider>();
		if (!collider || !collider->IsEnabled())
			continue;
		if (collider->ContainsPoint(worldX, worldY))
			return obj;
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
	const float EPSILON = 0.1f; // 부동소수점 비교를 위한 작은 오차 범위
	
	if (std::abs(currentViewport.X - m_lastViewportRect.X) > EPSILON || 
		std::abs(currentViewport.Y - m_lastViewportRect.Y) > EPSILON ||
		std::abs(currentViewport.Width - m_lastViewportRect.Width) > EPSILON || 
		std::abs(currentViewport.Height - m_lastViewportRect.Height) > EPSILON) {
		m_viewportChanged = true;
		m_lastViewportRect = currentViewport;
	} else {
		// 뷰포트가 변경되지 않았으면 플래그 리셋
		m_viewportChanged = false;
	}
}

void CameraManager::RenderVisibleTiles(const MapData* mapData)
{
	if (!mapData) {
		return;
	}

	RenderManager* renderManager = RenderManager::GetInstance();
	if (!renderManager) return;

	// 카메라 위치 기반으로 현재 뷰포트 계산 (매 프레임마다)
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

	// 타일 범위가 변경되었을 때만 캐시 정리
	bool tileRangeChanged = (startTileX != m_lastStartTileX || endTileX != m_lastEndTileX ||
	                         startTileY != m_lastStartTileY || endTileY != m_lastEndTileY);
	
	if (tileRangeChanged || !m_tileRangeInitialized) {
		// 사용하지 않는 타일 캐시 정리 (메모리 최적화)
		CleanupUnusedTileCache(mapData, startTileX, endTileX, startTileY, endTileY);

		m_lastStartTileX = startTileX;
		m_lastStartTileY = startTileY;
		m_lastEndTileX = endTileX;
		m_lastEndTileY = endTileY;
		m_tileRangeInitialized = true;
	}
	
	// 카메라 변환 정보 미리 계산
	Gdiplus::PointF cameraPos = GetCameraPos();
	float halfScreenWidth = WINCX / 2.0f;
	float halfScreenHeight = WINCY / 2.0f;
	
	for (int y = m_lastStartTileY; y < m_lastEndTileY; ++y) {
		float worldY = y * TILE_SIZE + TILE_SIZE / 2.0f;
		// Y 좌표 변환을 미리 계산 (같은 행의 모든 타일에 대해 동일)
		float screenYBase = worldY - cameraPos.Y + halfScreenHeight;
		
		for (int x = m_lastStartTileX; x < m_lastEndTileX; ++x) {
			const ResourcePathUtils::TileResourceDef& tileData = mapData->tiles[x][y];
			if (tileData.id == TILEID_NONE || tileData.type == TILE_NONE) {
				continue;
			}

			// 타일 캐시에서 비트맵 찾기
			auto cacheIt = m_tileCache.find(tileData.id);
			if (cacheIt == m_tileCache.end()) {
				TileCacheData newCacheData;
				newCacheData.id = tileData.id;
				LoadTileBitmap(tileData, newCacheData);
				if (newCacheData.bitmap) {
					m_tileCache[tileData.id] = newCacheData;
					cacheIt = m_tileCache.find(tileData.id);
				} else {
					continue;
				}
			}

			Gdiplus::Bitmap* tileBitmap = cacheIt->second.bitmap;
			if (!tileBitmap) {
				LoadTileBitmap(tileData, cacheIt->second);
				tileBitmap = cacheIt->second.bitmap;
				if (!tileBitmap) {
					m_tileCache.erase(cacheIt);
					continue;
				}
			}

			// 성능 최적화: WorldToScreen 변환을 직접 계산 (함수 호출 오버헤드 제거)
			float worldX = x * TILE_SIZE + TILE_SIZE / 2.0f;
			float screenX = worldX - cameraPos.X + halfScreenWidth;
			float screenY = screenYBase;
			
			// RenderManager에 직접 명령 추가 (RenderTile 함수 호출 오버헤드 제거)
			float renderX = screenX - TILE_SIZE * 0.5f;
			float renderY = screenY - TILE_SIZE * 0.5f;
			Gdiplus::RectF destRect(renderX, renderY, TILE_SIZE, TILE_SIZE);
			Gdiplus::RectF sourceRect(0, 0, static_cast<float>(tileBitmap->GetWidth()), static_cast<float>(tileBitmap->GetHeight()));
			Gdiplus::PointF screenPos(screenX, screenY);
			
			renderManager->AddDrawCommand(tileBitmap, destRect, sourceRect, Gdiplus::UnitPixel, screenPos, LAYER_WORLD_TILE, LAYER_WORLD_TILE, DIR_DOWN);
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

void CameraManager::CleanupUnusedTileCache(const MapData* mapData, int startTileX, int endTileX, int startTileY, int endTileY)
{
	if (!mapData) return;

	// 현재 뷰포트에 보이는 타일 ID 수집
	std::set<UINT> visibleTileIDs;
	for (int y = startTileY; y < endTileY; ++y) {
		for (int x = startTileX; x < endTileX; ++x) {
			if (x >= 0 && x < mapData->mapWidth && 
				y >= 0 && y < mapData->mapHeight) {
				const ResourcePathUtils::TileResourceDef& tileData = mapData->tiles[x][y];
				if (tileData.id != TILEID_NONE && tileData.type != TILE_NONE) {
					visibleTileIDs.insert(static_cast<UINT>(tileData.id));
				}
			}
		}
	}

	// 사용하지 않는 타일 캐시 제거 (메모리 최적화)
	auto it = m_tileCache.begin();
	while (it != m_tileCache.end()) {
		// 현재 뷰포트에 보이지 않는 타일이면 제거
		if (visibleTileIDs.find(it->first) == visibleTileIDs.end()) {
			if (it->second.bitmap) {
				delete it->second.bitmap;
				it->second.bitmap = nullptr;
			}
			it = m_tileCache.erase(it);
		}
		else {
			++it;
		}
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

void CameraManager::LoadTileBitmap(const ResourcePathUtils::TileResourceDef& tileData, TileCacheData& cacheData)
{
	if (tileData.baseDir.empty() || tileData.imageName.empty()) {
		return;
	}
	std::wstring fullPath = tileData.baseDir;
	if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
		fullPath += L"\\";
	}
	fullPath += tileData.imageName;

	cacheData.bitmap = new Gdiplus::Bitmap(fullPath.c_str());
	if (!(cacheData.bitmap && cacheData.bitmap->GetLastStatus() == Gdiplus::Ok)) {
		if (cacheData.bitmap) {
			delete cacheData.bitmap;
			cacheData.bitmap = nullptr;
		}
	}
}

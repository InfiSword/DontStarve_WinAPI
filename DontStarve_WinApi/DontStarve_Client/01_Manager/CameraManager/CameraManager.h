#pragma once

namespace ResourcePathUtils { struct TileResourceDef; }

// 타일 캐시 데이터 구조체
struct TileCacheData {
	TileID id;
	Gdiplus::Bitmap* bitmap;

	TileCacheData() : id(TILEID_NONE), bitmap(nullptr) {}
};

class GameObject;
class Collider;

class CameraManager : public CSingleTon<CameraManager>
{
	friend class CSingleTon<CameraManager>;
public:
	CameraManager();
	~CameraManager();

	void Init();
	void Update(float deltaTime);
	void Release();

	// 월드 좌표 <-> 화면 좌표 변환 함수
	Gdiplus::PointF WorldToScreen(float worldX, float worldY) const;
	Gdiplus::PointF ScreenToWorld(float screenX, float screenY) const;
	
	// 뷰포트 월드 좌표 반환 함수
	Gdiplus::RectF GetViewportWorldRect() const;
	
	Gdiplus::PointF GetCameraPos() const;
	void SetCameraPos(float x, float y);

	// 타겟 설정
	void SetTarget(GameObject* target);
	const GameObject* GetTarget() const;
	GameObject* GetTarget() { return m_target; }
	void FollowTarget();

	void SetFollowMode(bool enabled) { m_followMode = enabled; }
	bool IsFollowMode() const { return m_followMode; }
	
	// 뷰포트 내 보이는 오브젝트 업데이트
	void UpdateVisibleObjects();

	// 제거된 오브젝트를 visible 목록에서 즉시 제거 (삭제 전 호출하여 댕글링 포인터 방지)
	void RemoveFromVisibleObjects(GameObject* obj);

	// 새 오브젝트 추가 시 호출. 현재 뷰포트에 잡히면 visible 목록에만 추가, 아니면 미추가(다른 오브젝트와 동일)
	void TryAddToVisibleIfInViewport(GameObject* obj);

	// 월드 좌표에 있는 상호작용 가능 오브젝트 찾기 (화면 역순, AABB, CanInteract()만 후보)
	GameObject* FindInteractableObjectAtPosition(float worldX, float worldY);

	// 공격 콜라이더와 겹치는 뷰포트 내 오브젝트 목록 반환 (타입 필터 없음)
	void FindObjectsIntersectingCollider(Collider* pCollider, std::vector<GameObject*>& outObjects);

	// Sprite 바운딩 박스 계산
	Gdiplus::RectF GetSpriteBoundingBox(GameObject* obj) const;

	// === 렌더링 함수 ===
	// 타일 렌더링
	void RenderVisibleTiles(const MapData* mapData);
	
	// 월드 오브젝트 렌더링 (UI 제외)
	void RenderVisibleGameObjects();
	
	void ClearTileCache();

	// Walkable 경계 정보 조회
	bool HasWalkableBounds() const { return m_hasWalkableBounds; }
	void GetWalkableBounds(float& outMinX, float& outMinY, float& outMaxX, float& outMaxY) const {
		outMinX = m_walkableMinX; outMinY = m_walkableMinY;
		outMaxX = m_walkableMaxX; outMaxY = m_walkableMaxY;
	}

	// Walkable 영역으로 카메라 이동 제한 (맵 로드 후 호출)
	void SetWalkableBoundsFromMapData(const MapData* mapData);
	// 외부에서 계산된 Walkable 경계를 설정 (GameScene 등에서 사용)
	void SetWalkableBounds(float minX, float minY, float maxX, float maxY);

private:
    GameObject* m_target;
	Gdiplus::PointF m_cameraPos;

	bool m_followMode;

	// Walkable 영역 경계 (카메라 클램프용, 월드 좌표)
	bool m_hasWalkableBounds;
	float m_walkableMinX, m_walkableMinY, m_walkableMaxX, m_walkableMaxY;
	
	// 뷰포트 관리
	std::vector<GameObject*> m_visibleObjects;
	Gdiplus::RectF m_lastViewportRect;
	bool m_viewportChanged;
	
	void CheckViewportChanged();
	bool IsObjectInViewport(GameObject* obj) const;

	// 타일 캐시 관리
	std::map<UINT, TileCacheData> m_tileCache;
	int m_lastStartTileX, m_lastStartTileY, m_lastEndTileX, m_lastEndTileY;
	bool m_tileRangeInitialized;
	void LoadTileBitmap(const ResourcePathUtils::TileResourceDef& tileData, TileCacheData& cacheData);
	void CleanupUnusedTileCache(const MapData* mapData, int startTileX, int endTileX, int startTileY, int endTileY);
};

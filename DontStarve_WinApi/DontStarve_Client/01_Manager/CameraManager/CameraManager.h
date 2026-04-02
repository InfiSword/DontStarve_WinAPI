#pragma once

namespace ResourcePathUtils { struct TileResourceDef; }

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
	~CameraManager() { Release(); }

	void Init();
	void Update(float deltaTime);
	void Release();

	Gdiplus::RectF GetViewportWorldRect() const;
	Gdiplus::PointF GetCameraPos() const { return m_cameraPos; }
	void SetCameraPos(float x, float y) { m_cameraPos = { x, y }; }

	void SetTarget(GameObject* target) { m_target = target; }
	GameObject* GetTarget() { return m_target; }
	void FollowTarget();
	void SetFollowMode(bool enabled) { m_followMode = enabled; }

	void UpdateVisibleObjects();
	void RemoveFromVisibleObjects(GameObject* obj);
	void TryAddToVisibleIfInViewport(GameObject* obj);
	
	GameObject* FindInteractableObjectAtPosition(float worldX, float worldY);
	void FindObjectsIntersectingCollider(Collider* pCollider, std::vector<GameObject*>& outObjects, bool onlyInteraction = false);

	void RenderVisibleTiles(const MapData* mapData);
	void RenderVisibleGameObjects();
	void ClearTileCache();

	void SetWalkableBoundsFromMapData(const MapData* mapData);
	void SetWalkableBounds(float minX, float minY, float maxX, float maxY);

	Gdiplus::PointF WorldToScreen(float worldX, float worldY) const;
	Gdiplus::PointF ScreenToWorld(float screenX, float screenY) const;

private:
	GameObject* m_target = nullptr;
	Gdiplus::PointF m_cameraPos = { 0, 0 };
	bool m_followMode = true;

	bool m_hasWalkableBounds = false;
	float m_walkableMinX = 0, m_walkableMinY = 0, m_walkableMaxX = 0, m_walkableMaxY = 0;
	
	std::vector<GameObject*> m_visibleObjects;
	Gdiplus::RectF m_lastViewportRect = { 0, 0, 0, 0 };

	std::unordered_map<UINT, TileCacheData> m_tileCache;
	int m_lastStartTileX = -1, m_lastStartTileY = -1, m_lastEndTileX = -1, m_lastEndTileY = -1;

	bool IsObjectInViewport(GameObject* obj) const;
	void LoadTileBitmap(const ResourcePathUtils::TileResourceDef& tileData, TileCacheData& cacheData);
	void CleanupUnusedTileCache(const MapData* mapData, int startX, int endX, int startY, int endY);
};

#pragma once

struct TileData;

// 타일 캐시 데이터 구조체
struct TileCacheData {
	TileID id;
	Gdiplus::Bitmap* bitmap;
	Gdiplus::RectF sourceRect;
	bool isAtlasBased;
	
	TileCacheData() : id(TILEID_NONE), bitmap(nullptr), isAtlasBased(false) {}
	TileCacheData(TileID _id, Gdiplus::Bitmap* _bitmap, const Gdiplus::RectF& _rect, bool _isAtlas) 
		: id(_id), bitmap(_bitmap), sourceRect(_rect), isAtlasBased(_isAtlas) {}
};

class GameObject;

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

	// 타겟 설정
	void SetTarget(GameObject* target);
	const GameObject* GetTarget() const;
	void FollowTarget();
	void SetFollowMode(bool enabled) { m_followMode = enabled; }
	bool IsFollowMode() const { return m_followMode; }
	
	// 카메라 위치 설정 함수
	void SetCameraPosition(float x, float y);

	// 뷰포트 내 보이는 오브젝트 업데이트
	void UpdateVisibleObjects();
	
	// 월드 좌표에 있는 오브젝트 찾기
	GameObject* FindObjectAtPosition(float worldX, float worldY);
	
	// Sprite 바운딩 박스 계산
	Gdiplus::RectF GetSpriteBoundingBox(GameObject* obj) const;

	// === 렌더링 함수 ===
	// 타일 렌더링
	void RenderVisibleTiles(const MapData* mapData);
	
	// 월드 오브젝트 렌더링 (UI 제외)
	void RenderVisibleGameObjects();
	
	void ClearTileCache();

private:
    GameObject* m_target;
	Gdiplus::PointF m_cameraPos;

	float m_zoomFactor;             // 줌 인수
	
	bool m_followMode;
	
	// 뷰포트 관리
	std::vector<GameObject*> m_visibleObjects;
	std::unordered_set<GameObject*> m_visibleObjectSet;
	Gdiplus::RectF m_lastViewportRect;
	bool m_viewportChanged;
	void CheckViewportChanged();
	
	// 타일 캐시 관리
	std::map<UINT, TileCacheData> m_tileCache;
	int m_lastStartTileX, m_lastStartTileY, m_lastEndTileX, m_lastEndTileY;
	bool m_tileViewportChanged;
	void LoadTileBitmap(const TileData& tileData, TileCacheData& cacheData);
	void RenderSingleTile(const MapData* mapData, int x, int y, float worldY);
};

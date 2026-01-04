#pragma once

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
class RenderManager;
class Player;

class CameraManager : public CSingleTon<CameraManager>
{
	friend class CSingleTon<CameraManager>;
public:
	CameraManager();
	~CameraManager();

	void Init();
	void LateInit();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	// 월드 좌표 <-> 화면 좌표 변환 함수
	Gdiplus::PointF WorldToScreen(float worldX, float worldY);
	Gdiplus::PointF ScreenToWorld(float screenX, float screenY);
	
	// 뷰포트 영역 관련 함수
	Gdiplus::RectF GetViewportWorldRect() const;
	
	Gdiplus::PointF GetCameraPos();

	// 카메라 추적 기능
	void SetTarget(GameObject* target);
	const GameObject* GetTarget();
	void FollowTarget(float deltaTime);
	void SetFollowMode(bool enabled) { m_followMode = enabled; }
	bool IsFollowMode() const { return m_followMode; }
	
	// 카메라 위치 직접 설정
	void SetCameraPosition(float x, float y);

	// === 화면에 보이는 게임오브젝트 관리 기능 (ViewportManager 역할) ===
	// 화면에 보이는 게임오브젝트 업데이트 (ObjectManager의 게임오브젝트들을 기반으로)
	void UpdateVisibleObjects();
	const std::vector<GameObject*>& GetVisibleObjects() const { return m_visibleObjects; }
	
	// 특정 위치의 게임오브젝트 찾기 (화면에 보이는 게임오브젝트만 검색)
	GameObject* FindObjectAtPosition(float worldX, float worldY);
	
	// 화면에 보이는 게임오브젝트인지 확인
	bool IsObjectVisible(GameObject* obj) const;
	
	// 뷰포트 변경 확인
	bool HasViewportChanged() const { return m_viewportChanged; }
	void ClearViewportChanged() { m_viewportChanged = false; }
	
	// Sprite 크기 기반 바운딩 박스 가져오기 (Animator/SpriteRenderer에서 sprite 크기 사용)
	Gdiplus::RectF GetSpriteBoundingBox(GameObject* obj) const;

	// === 타일 렌더링 관리 기능 ===
	// 화면에 보이는 타일 렌더링 (맵 데이터를 파라미터로 받음)
	void RenderVisibleTiles(RenderManager* renderManager, const MapData* mapData);
	
	// 타일 캐시 비우기
	void ClearTileCache();

private:
    GameObject* m_target;
	Gdiplus::PointF m_cameraPos;

	float m_zoomFactor;             // 줌 배율 설정
	
	bool m_followMode;              // 카메라 추적 모드
	
	// === 화면에 보이는 게임오브젝트 관리 (ViewportManager 역할) ===
	std::vector<GameObject*> m_visibleObjects;
	std::unordered_set<GameObject*> m_visibleObjectSet; // 빠른 검색을 위한 해시셋
	
	// 뷰포트 상태
	Gdiplus::RectF m_lastViewportRect;
	bool m_viewportChanged;
	
	// 화면에 보이는 게임오브젝트인지 확인하는 내부 함수
	bool IsObjectInViewport(GameObject* obj) const;
	
	// 뷰포트 변경 확인
	void CheckViewportChanged();
	
	// === 타일 렌더링 관리 ===
	std::map<UINT, TileCacheData> m_tileCache;
	std::vector<std::pair<int, int>> m_visibleTileIndices;
	
	// 타일 렌더링 최적화 관련 변수
	int m_lastStartTileX, m_lastStartTileY, m_lastEndTileX, m_lastEndTileY;
	bool m_tileViewportChanged;
	
	// 타일 렌더링 관련 함수들
	void LoadTileBitmap(TileID tileID, TileCacheData& cacheData);
	void RenderSingleTile(RenderManager* renderManager, const MapData* mapData, int x, int y, float worldY);
	// bool IsTileInViewport(int tileX, int tileY) const; // 현재 미사용
};

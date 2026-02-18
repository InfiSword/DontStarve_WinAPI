#pragma once

class EditorView;
class EditorResourceManager;
class EditorMap;
class EditorPalette;
class EditorPivotEditor;
class EditorColliderEditor;
class EditorLayerComposer;

class DontStarve_EditorMain
{
	friend class EditorMap;
	friend class EditorColliderEditor;
	friend class EditorWalkableEditor;
	friend class EditorDebugPanel;
	friend class EditorLayerComposer;

public:
	DontStarve_EditorMain();
	~DontStarve_EditorMain();

public:
	void Initialize();
	void Update();
	void Render();
	void Release();

	LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam);

	// Map save/load/new functions
	void NewMap();                          
	bool SaveMap(const WCHAR* filename);
	bool LoadMap(const WCHAR* filename);
	bool ShowSaveFileDialog(WCHAR* fileName, DWORD fileNameSize);
	bool ShowOpenFileDialog(WCHAR* fileName, DWORD fileNameSize);

	// 맵 크기 (타일 단위, 1~MAP_WIDTH / 1~MAP_HEIGHT)
	int GetMapWidth() const { return m_mapWidth; }
	int GetMapHeight() const { return m_mapHeight; }
	void SetMapSize(int width, int height);
	void ShowMapSizeDialog(HWND parent);

	void AddObject(const ResourcePathUtils::ObjectResourceDef& obj);
	void RemoveObject(size_t idx); // Remove by index
	void RemoveObject(ResourcePathUtils::ObjectResourceDef* objToRemove); // Remove by pointer (fast search)
	void UpdateObjectPosition(ResourcePathUtils::ObjectResourceDef* obj, int newX, int newY); // Update object position

	// Performance info get/set (set from Editor.cpp)
	void SetCurrentFPS(float fps) { m_currentFPS = fps; }
	float GetCurrentFPS() const { return m_currentFPS; }
	float GetLayerMemoryUsageMB() const;

	// Debug info
	void SetDebugInfoVisible(bool visible);
	bool IsDebugInfoVisible() const;

private:
	// GDI+ rendering related member variables
	Gdiplus::Graphics* m_pGraphics;
	Gdiplus::Bitmap* m_pDoubleBufferBitmap;

	// Map data related member variables
	int m_mapWidth;   // 현재 맵 가로 타일 수 (1 ~ MAP_WIDTH)
	int m_mapHeight; // 현재 맵 세로 타일 수 (1 ~ MAP_HEIGHT)
	ResourcePathUtils::TileResourceDef m_tileMap[MAP_HEIGHT][MAP_WIDTH]; // Tile map data ([행][열] 형식)
	std::vector<ResourcePathUtils::ObjectResourceDef> m_gameObjects;

	// Player spawn point related
	bool m_hasPlayerSpawn = false;				// Whether player spawn point is set
	Gdiplus::PointF m_playerSpawnPoint;			// Player spawn point coordinates (map center)
	bool m_isPlayerSpawnMode = false;			// Player spawn point edit mode

	// Placement mode related member variables
	bool m_isPlacingMode;                       // Whether currently in placement mode
	bool m_is3x3Mode;                           // 3x3 tile placement mode flag
	POINT m_rawMousePos;                        // Raw mouse cursor screen coordinates
	Gdiplus::PointF m_snappedPreviewPos;        // Snapped preview position screen coordinates (float precision)

	// Pivot edit (owned by EditorPivotEditor; access via m_pPivotEditor)
	std::unique_ptr<EditorPivotEditor> m_pPivotEditor;

	// Collider edit (owned by EditorColliderEditor; access via m_pColliderEditor)
	std::unique_ptr<EditorColliderEditor> m_pColliderEditor;

	// Collider template (영구 저장): 타입별 기본 콜라이더, 맵 저장/로드 시 파일로 유지
	struct ColliderTemplate {
		bool hasCollider = false;
		ColliderType colliderType = COLLIDER_BOX;
		int colliderOffsetX = 0, colliderOffsetY = 0, colliderWidth = 0, colliderHeight = 0;
		float colliderCenterX = 0.0f, colliderCenterY = 0.0f, colliderRadius = 0.0f;
	};
	std::map<std::pair<int, int>, ColliderTemplate> m_colliderTemplates; // key = (GameObjectType, GameObjectID)
	std::wstring m_lastMapDirectory;            // 마지막 맵 파일 디렉터리 (템플릿 파일 경로용)

	// View (map offset, zoom, coordinate conversion)
	std::unique_ptr<EditorView> m_pView;

	// Resources (tile/object variants, atlases)
	std::unique_ptr<EditorResourceManager> m_pResources;

	// Palette (items, sub-palette, selection)
	std::unique_ptr<EditorPalette> m_pPalette;

	// Layer composer (grid/tile/object layer bitmap management)
	std::unique_ptr<EditorLayerComposer> m_pLayerComposer;

	// Object layer optimization related (Client CameraManager 방식: visible만 그리기)
	std::vector<const ResourcePathUtils::ObjectResourceDef*> m_sortedObjects;          // Y 좌표로 정렬된 오브젝트 포인터 목록
	std::vector<const ResourcePathUtils::ObjectResourceDef*> m_visibleObjectsCache;   // 뷰포트 내 오브젝트만 (갱신 시에만 재계산)
	Gdiplus::RectF m_lastViewportWorldRect;                     // visible 캐시 갱신 시점의 뷰포트
	bool m_objectsDirty = true;
	ResourcePathUtils::ObjectResourceDef* m_selectedObjectPtr;

	// Palette layer bitmap (owned by Main, managed by EditorPalette)
	Gdiplus::Bitmap* m_paletteLayerBitmap;
	bool m_paletteLayerDirty = true;

	// Performance info member variables (set from Editor.cpp)
	float m_currentFPS = 0.0f;		// Current FPS (updated from outside)

	// Debug panel (owned by EditorDebugPanel; access via m_pDebugPanel)
	std::unique_ptr<EditorDebugPanel> m_pDebugPanel;

	// Walkable edit (owned by EditorWalkableEditor; access via m_pWalkableEditor)
	std::unique_ptr<EditorWalkableEditor> m_pWalkableEditor;
	bool m_walkableAreaMap[MAP_HEIGHT][MAP_WIDTH];	// Per tile walkable area (owned by Main, accessed by EditorWalkableEditor via friend)

	// Camera dragging related member variables
	bool m_isDraggingCamera = false;		// Camera dragging state flag
	POINT m_cameraDragStart;				// Camera drag start point (screen coordinates)
	POINT m_initialMapOffset;				// Map offset at drag start

private:
	// Initialization related functions
	void InitPalette();                             // Palette initialization

	// Drawing related functions
	void DrawPreview(Gdiplus::Graphics* pGraphics);
	void DrawPlayerSpawn(Gdiplus::Graphics* pGraphics);	// Draw player spawn point

	// HandleMessage helper functions
	void HandlePlacingModeClick(POINT clickPoint, HWND hWnd);
	void HandleObjectSelectionClick(POINT clickPoint, HWND hWnd);
	void DeselectObject(HWND hWnd);
	void ExitAllEditModes();
	
	// 유틸리티 헬퍼 함수
	bool IsPointInDebugPanel(POINT clickPoint) const;

	// Coordinate conversion (delegate to EditorView; client size from g_hWnd)
	Gdiplus::RectF GetViewWorldRect(float cullingMargin = 0.0f) const;
	Gdiplus::PointF WorldToScreen(Gdiplus::PointF worldPos) const;
	Gdiplus::RectF WorldToScreen(Gdiplus::RectF worldRect) const;
	Gdiplus::PointF ScreenToWorld(Gdiplus::PointF screenPos) const;
};

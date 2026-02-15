#pragma once

class DontStarve_EditorMain
{
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
	void NewMap();                                  // Create new map (initialize)
	bool SaveMap(const WCHAR* filename);
	bool LoadMap(const WCHAR* filename);

	// File dialog functions
	bool ShowSaveFileDialog(WCHAR* fileName, DWORD fileNameSize);
	bool ShowOpenFileDialog(WCHAR* fileName, DWORD fileNameSize);

	// Object add/remove/position update functions (m_objectsDirty flag management)
	void AddObject(const GameObjectData& obj);
	void RemoveObject(size_t idx); // Remove by index
	void RemoveObject(GameObjectData* objToRemove); // Remove by pointer (fast search)
	void UpdateObjectPosition(GameObjectData* obj, int newX, int newY); // Update object position

	// Performance info get/set (set from Editor.cpp)
	void SetCurrentFPS(float fps) { m_currentFPS = fps; }
	float GetCurrentFPS() const { return m_currentFPS; }
	float GetLayerMemoryUsageMB() const;

	// Debug info
	void SetDebugInfoVisible(bool visible) { m_showDebugInfo = visible; }
	bool IsDebugInfoVisible() const { return m_showDebugInfo; }

private:
	// GDI+ rendering related member variables
	Gdiplus::Graphics* m_pGraphics;
	Gdiplus::Bitmap* m_pDoubleBufferBitmap;

	// Map data related member variables
	TileData m_tileMap[MAP_WIDTH][MAP_HEIGHT]; // Tile map data
	std::vector<GameObjectData> m_gameObjects;

	// Player spawn point related
	bool m_hasPlayerSpawn = false;				// Whether player spawn point is set
	Gdiplus::PointF m_playerSpawnPoint;			// Player spawn point coordinates (map center)
	bool m_isPlayerSpawnMode = false;			// Player spawn point edit mode

	// Palette related member variables
	std::vector<PaletteItem> m_paletteItems;    // Palette item list
	RECT m_paletteRect;                         // Palette area
	int m_selectedPaletteIndex;                 // Currently selected palette item index (-1 means not selected)

	// Placement mode related member variables
	bool m_isPlacingMode;                       // Whether currently in placement mode
	bool m_is3x3Mode;                           // 3x3 tile placement mode flag
	POINT m_rawMousePos;                        // Raw mouse cursor screen coordinates
	Gdiplus::PointF m_snappedPreviewPos;        // Snapped preview position screen coordinates (float precision)

	// Pivot edit mode related member variables
	bool m_isPivotEditMode;                     // Pivot edit mode flag
	POINT m_pivotEditPos;                       // Pivot edit position
	float m_currentPivotX;                      // Currently editing Pivot X
	float m_currentPivotY;                      // Currently editing Pivot Y
	GameObjectData* m_editingObject;            // Currently editing object

	const int MIN_COLLIDER_SIZE = 4;
	const float MIN_COLLIDER_RADIUS = 2.0f;    // CircleCollider minimum radius
	bool m_isColliderEditMode = false;          // Collider edit mode flag
	GameObjectData* m_editingColliderObject = nullptr; // Currently editing collider object
	bool m_isDraggingCollider = false;          // Collider dragging state flag
	POINT m_colliderEditStartMousePos;          // Collider size/position edit start mouse position (screen coordinates)
	RECT m_initialColliderRect;                 // Collider edit start initial collider position/size (object local coordinates) - BoxCollider
	float m_initialColliderCenterX;            // Collider edit start CircleCollider initial center X
	float m_initialColliderCenterY;            // Collider edit start CircleCollider initial center Y
	float m_initialColliderRadius;             // Collider edit start CircleCollider initial radius
	int m_draggingHandle = -1;                  // Dragging handle index (0:top-left, 1:top-right, 2:bottom-left, 3:bottom-right, 4:move, 5:radius-adjust)

	// Map offset related member variables
	POINT m_mapOffset;                          // Map rendering offset (camera position)

	// Sub palette related structure
	struct SubPaletteData {
		bool isOpen = false;
		ItemCategory category = CATEGORY_NONE;
		int targetCategoryId = -1;
		// TileType or GameObjectType (category ID selected from main palette)

		RECT rect = { 0 };			 // Sub palette UI area
		std::vector<RECT> itemRects; // Sub palette item UI Rect

		std::vector<std::pair<TileID, const TileVariant*>> currentTileVariantDefs;
		std::vector<std::pair<GameObjectID, const ObjectVariant*>> currentObjectVariantDefs;

		int selectedTileVariantIndex = -1;
		int selectedObjectVariantIndex = -1;

		const TileVariant* getSelectedTileVariant() const {
			if (selectedTileVariantIndex != -1 && category == CATEGORY_TILE && selectedTileVariantIndex < currentTileVariantDefs.size())
			{
				return currentTileVariantDefs[selectedTileVariantIndex].second;
			}
			return nullptr;
		}
		const ObjectVariant* getSelectedObjectVariant() const {
			if (selectedObjectVariantIndex != -1 && category == CATEGORY_OBJECT && selectedObjectVariantIndex < currentObjectVariantDefs.size()) {
				return currentObjectVariantDefs[selectedObjectVariantIndex].second;
			}
			return nullptr;
		}

		TileID getSelectedTileID() const {
			if (selectedTileVariantIndex != -1 &&
				selectedTileVariantIndex < currentTileVariantDefs.size())
			{
				return currentTileVariantDefs[selectedTileVariantIndex].first;
			}
			return TILEID_NONE;
		}

		GameObjectID getSelectedGameObjectID() const {
			if (selectedObjectVariantIndex != -1 && selectedObjectVariantIndex < currentObjectVariantDefs.size()) {
				return currentObjectVariantDefs[selectedObjectVariantIndex].first;
			}
			return GOID_NONE;
		}
	} m_subPalette;


	// Resource variant related member variables	
	std::map<TileType, std::map<TileID, TileVariant>> m_tileVariants;
	std::map<GameObjectType, std::map<GameObjectID, ObjectVariant>> m_objectVariants;

	// Atlas bitmap owners
	std::unique_ptr<Gdiplus::Bitmap> m_tileAtlasBitmapOwner;
	std::unique_ptr<Gdiplus::Bitmap> m_objectAtlasBitmapOwner;

	// Zoom related member variables
	float m_zoomFactor;
	const float m_minZoom;
	const float m_maxZoom;
	const float m_zoomStep;

	// Object layer optimization related
	std::vector<const GameObjectData*> m_sortedObjects;
	bool m_objectsDirty = true;
	GameObjectData* m_selectedObjectPtr;

	// Layer bitmap caching related member variables
	Gdiplus::Bitmap* m_tileLayerBitmap;
	bool m_tileLayerDirty = true;
	Gdiplus::Bitmap* m_objectLayerBitmap;
	bool m_objectLayerDirty = true;
	Gdiplus::Bitmap* m_paletteLayerBitmap;
	bool m_paletteLayerDirty = true;
	Gdiplus::Bitmap* m_gridLayerBitmap;
	bool m_gridLayerDirty = true;

	// Performance info member variables (set from Editor.cpp)
	float m_currentFPS = 0.0f;		// Current FPS (updated from outside)

	// Debug info
	bool m_showDebugInfo = true;			// Debug info display flag

	// Walkable Area edit mode related member variables
	bool m_isWalkableEditMode = false;		// Walkable area edit mode flag
	bool m_isDraggingWalkable = false;		// Walkable area dragging state flag
	POINT m_walkableDragStart;				// Drag start point (screen coordinates)
	POINT m_walkableDragEnd;				// Drag end point (screen coordinates)
	bool m_walkableAreaMap[MAP_HEIGHT][MAP_WIDTH];	// Per tile walkable area

	// Camera dragging related member variables
	bool m_isDraggingCamera = false;		// Camera dragging state flag
	POINT m_cameraDragStart;				// Camera drag start point (screen coordinates)
	POINT m_initialMapOffset;				// Map offset at drag start

private:
	// Initialization related functions
	void InitPalette();                             // Palette initialization
	void LoadResources();                           // Image resource loading
	void ReleaseResources();                        // Image resource release

	// Drawing related functions
	void ComposeTileLayer();
	void ComposeObjectLayer();
	void ComposePaletteLayer();
	void ComposeGridLayer();

	// Performance info function (memory usage)

	void DrawGrid(Gdiplus::Graphics* pGraphics);
	void DrawTileMap(Gdiplus::Graphics* pGraphics);
	void DrawObjects(Gdiplus::Graphics* pGraphics);
	void DrawPalette(Gdiplus::Graphics* pGraphics);
	void DrawPreview(Gdiplus::Graphics* pGraphics);
	void DrawSubPalette(Gdiplus::Graphics* pGraphics);
	void DrawPivotEditor(Gdiplus::Graphics* pGraphics); // Draw pivot editor (as mentioned in comments)
	void DrawColliders(Gdiplus::Graphics* pGraphics);
	void DrawPlayerSpawn(Gdiplus::Graphics* pGraphics);	// Draw player spawn point
	void DrawWalkableAreas(Gdiplus::Graphics* pGraphics);	// Draw walkable areas
	void DrawDebugInfo(Gdiplus::Graphics* pGraphics);

	const TileVariant* GetTileVariant(TileType type, TileID id) const;
	const ObjectVariant* GetObjectVariant(GameObjectType type, GameObjectID id)const;

	// Pivot edit mode related functions (as mentioned in comments)
	void UpdatePivotEdit(POINT clickPoint);
	void StartPivotEdit(GameObjectData* obj);
	void EndPivotEdit();

	// Collider edit
	void StartColliderEdit(GameObjectData* obj);
	void EndColliderEdit();
	int GetColliderHandleAt(POINT screenPos); // Check if collider handle exists at mouse position

	// Convert world coordinates visible on screen (map coordinates) to screen coordinates.
	Gdiplus::RectF GetViewWorldRect(float cullingMargin = 0.0f) const;
	Gdiplus::PointF WorldToScreen(Gdiplus::PointF worldPos) const;
	Gdiplus::RectF WorldToScreen(Gdiplus::RectF worldRect) const;
	Gdiplus::PointF ScreenToWorld(Gdiplus::PointF screenPos) const;
};

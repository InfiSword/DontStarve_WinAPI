#pragma once

#include "IEditorScreen.h"

// MapEditor: 런처에서 "Map Editor" 선택 시 진입. 타일/오브젝트 배치·이동·삭제, 플레이어 스폰, 워커블, 맵 I/O.
// 팔레트: 타일 + 오브젝트. 피벗/콜라이더 편집 없음 (ObjectEditor에서 담당).

class EditorView;
class EditorResourceManager;
class EditorMapFileIO;
class EditorPalette;
class EditorLayerComposer;
class EditorWalkableEditor;
class EditorDebugPanel;

class MapEditor : public IEditorScreen
{
	friend class EditorMapFileIO;
	friend class EditorWalkableEditor;
	friend class EditorLayerComposer;

public:
	MapEditor();
	~MapEditor();

	void Initialize() override;
	void Update() override;
	void Render() override;
	void Release() override;
	LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;

	// Map save/load/new
	void NewMap();
	bool SaveMap(const WCHAR* filename);
	bool LoadMap(const WCHAR* filename);
	bool ShowSaveFileDialog(WCHAR* fileName, DWORD fileNameSize);
	bool ShowOpenFileDialog(WCHAR* fileName, DWORD fileNameSize);

	int GetMapWidth() const { return m_mapWidth; }
	int GetMapHeight() const { return m_mapHeight; }
	int GetMapSizeDlgControlIdWidth() const { return m_idcMapWidth; }
	int GetMapSizeDlgControlIdHeight() const { return m_idcMapHeight; }
	void SetMapSize(int width, int height);
	void ShowMapSizeDialog(HWND parent);

	void AddObject(const ResourcePathUtils::ObjectResourceDef& obj);
	void RemoveObject(size_t idx);
	void RemoveObject(ResourcePathUtils::ObjectResourceDef* objToRemove);
	void UpdateObjectPosition(ResourcePathUtils::ObjectResourceDef* obj, int newX, int newY);

	void SetCurrentFPS(float fps) { m_currentFPS = fps; }
	float GetLayerMemoryUsageMB() const override;
	float GetCurrentFPS() const override { return m_currentFPS; }
	EditorScreenSwitch GetRequestedSwitch() override;

	void SetDebugInfoVisible(bool visible);
	bool IsDebugInfoVisible() const;

	const POINT& GetRawMousePos() const { return m_rawMousePos; }
	bool IsPlacingMode() const { return m_isPlacingMode; }
	bool IsErasingMode() const { return m_isErasingMode; }
	bool IsDraggingCamera() const { return m_isDraggingCamera; }
	const ResourcePathUtils::ObjectResourceDef* GetSelectedObjectPtr() const { return m_selectedObjectPtr; }
	Gdiplus::Bitmap* GetTileLayerBitmap() const;
	bool IsPlayerSpawnMode() const { return m_isPlayerSpawnMode; }
	bool HasPlayerSpawn() const { return m_hasPlayerSpawn; }
	const Gdiplus::PointF& GetPlayerSpawnPoint() const { return m_playerSpawnPoint; }
	const std::vector<ResourcePathUtils::ObjectResourceDef>& GetGameObjects() const { return m_gameObjects; }
	bool GetWalkableAt(int x, int y) const;
	size_t GetBitmapCacheSize() const;

private:
	EditorScreenSwitch m_requestedSwitch = EditorScreenSwitch::None;
	Gdiplus::Graphics* m_pGraphics = nullptr;
	Gdiplus::Bitmap* m_pDoubleBufferBitmap = nullptr;
	
	int m_mapWidth = MAP_WIDTH;
	int m_mapHeight = MAP_HEIGHT;
	int m_idcMapWidth = 1000;   // 맵 크기 다이얼로그 Width Edit 컨트롤 ID
	int m_idcMapHeight = 1001;  // 맵 크기 다이얼로그 Height Edit 컨트롤 ID
	ResourcePathUtils::TileResourceDef m_tileMap[MAP_HEIGHT][MAP_WIDTH];
	std::vector<ResourcePathUtils::ObjectResourceDef> m_gameObjects;

	bool m_hasPlayerSpawn = false;
	Gdiplus::PointF m_playerSpawnPoint;  // 월드 좌표, 맵 중앙 타일 중심으로 초기화
	bool m_isPlayerSpawnMode = false;

	bool m_isPlacingMode = false;
	bool m_is3x3Mode = false;
	bool m_isErasingMode = false;
	POINT m_rawMousePos = { 0, 0 };
	Gdiplus::PointF m_snappedPreviewPos;

	std::wstring m_lastMapDirectory;

	std::unique_ptr<EditorView> m_pView;
	std::unique_ptr<EditorResourceManager> m_pResources;
	std::unique_ptr<EditorPalette> m_pPalette;
	std::unique_ptr<EditorLayerComposer> m_pLayerComposer;

	std::vector<const ResourcePathUtils::ObjectResourceDef*> m_sortedObjects;
	std::vector<const ResourcePathUtils::ObjectResourceDef*> m_visibleObjectsCache;
	Gdiplus::RectF m_lastViewportWorldRect;
	bool m_objectsDirty = true;
	ResourcePathUtils::ObjectResourceDef* m_selectedObjectPtr = nullptr;

	Gdiplus::Bitmap* m_paletteLayerBitmap = nullptr;
	bool m_paletteLayerDirty = true;

	float m_currentFPS = 0.0f;

	std::unique_ptr<EditorDebugPanel> m_pDebugPanel;
	std::unique_ptr<EditorWalkableEditor> m_pWalkableEditor;
	bool m_walkableAreaMap[MAP_HEIGHT][MAP_WIDTH];

	bool m_isDraggingCamera = false;
	POINT m_cameraDragStart = { 0, 0 };
	POINT m_initialMapOffset = { 0, 0 };

	Gdiplus::RectF m_rectLauncherButton = { 0, 0, 0, 0 };  // 좌측 하단 Launcher 버튼
	void UpdateLauncherButtonRect(int clientW, int clientH);
	bool IsPointInLauncherButton(POINT pt) const;

	void InitPalette();
	void DrawPreview(Gdiplus::Graphics* pGraphics);
	void DrawPlayerSpawn(Gdiplus::Graphics* pGraphics);
	void HandlePlacingModeClick(POINT clickPoint, HWND hWnd);
	void HandleErasingModeClick(POINT clickPoint, HWND hWnd);
	void HandleObjectSelectionClick(POINT clickPoint, HWND hWnd);
	void DeselectObject(HWND hWnd);
	void ExitAllEditModes();
	bool IsPointInDebugPanel(POINT clickPoint) const;

	Gdiplus::RectF GetViewWorldRect(float cullingMargin = 0.0f) const;
	Gdiplus::PointF WorldToScreen(Gdiplus::PointF worldPos) const;
	Gdiplus::RectF WorldToScreen(Gdiplus::RectF worldRect) const;
	Gdiplus::PointF ScreenToWorld(Gdiplus::PointF screenPos) const;
};

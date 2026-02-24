#pragma once

#include "IEditorScreen.h"

class ObjectView;
class EditorResourceManager;
class EditorPalette;
class EditorPivotEditor;
class EditorColliderEditor;
class EditorDebugPanel;

class ObjectEditor : public IEditorScreen {
public:
	ObjectEditor();
	virtual ~ObjectEditor();

	void Initialize() override;
	void Update() override;
	void Render() override;
	void Release() override;
	LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	EditorScreenSwitch GetRequestedSwitch() override;
	void SetCurrentFPS(float fps) override { m_currentFPS = fps; }
	float GetCurrentFPS() const override { return m_currentFPS; }
	float GetLayerMemoryUsageMB() const override;

	std::vector<ResourcePathUtils::ObjectResourceDef>& GetGameObjects() { return m_gameObjects; }
	const std::vector<ResourcePathUtils::ObjectResourceDef>& GetGameObjects() const { return m_gameObjects; }

	const POINT& GetRawMousePos() const { return m_rawMousePos; }
	bool IsPlacingMode() const { return m_isPlacingMode; }
	ResourcePathUtils::ObjectResourceDef* GetSelectedObjectPtr() { return m_selectedObjectPtr; }
	const ResourcePathUtils::ObjectResourceDef* GetSelectedObjectPtr() const { return m_selectedObjectPtr; }

	/// 오브젝트 리소스(pivot/콜라이더)를 GameData에 저장. 성공 시 윈도우에 "저장했습니다" 문구 표시.
	bool SaveObjects();

private:
	static const int OBJECT_EDITOR_MAP_W = 100;  // 뷰 클램프용 가상 맵 폭(타일)
	static const int OBJECT_EDITOR_MAP_H = 100;  // 뷰 클램프용 가상 맵 높이(타일)

	EditorScreenSwitch m_requestedSwitch = EditorScreenSwitch::None;
	Gdiplus::Graphics* m_pGraphics = nullptr;
	Gdiplus::Bitmap* m_pDoubleBufferBitmap = nullptr;

	std::unique_ptr<ObjectView> m_pView;
	std::unique_ptr<EditorResourceManager> m_pResources;
	std::unique_ptr<EditorPalette> m_pPalette;
	std::unique_ptr<EditorPivotEditor> m_pPivotEditor;
	std::unique_ptr<EditorColliderEditor> m_pColliderEditor;
	std::unique_ptr<EditorDebugPanel> m_pDebugPanel;

	std::vector<ResourcePathUtils::ObjectResourceDef> m_gameObjects;
	bool m_objectsDirty = true;
	ResourcePathUtils::ObjectResourceDef* m_selectedObjectPtr = nullptr;

	Gdiplus::Bitmap* m_paletteLayerBitmap = nullptr;
	bool m_paletteLayerDirty = true;

	bool m_isPlacingMode = false;
	POINT m_rawMousePos = { 0, 0 };
	Gdiplus::PointF m_snappedPreviewPos = { 0.0f, 0.0f };

	bool m_isDraggingCamera = false;
	POINT m_cameraDragStart = { 0, 0 };
	POINT m_initialMapOffset = { 0, 0 };

	Gdiplus::RectF m_rectLauncherButton = { 0, 0, 0, 0 };  // 좌측 하단 Launcher 버튼
	Gdiplus::RectF m_rectCenterButton = { 0, 0, 0, 0 };    // 가운데 상단 맵 중앙으로 버튼
	Gdiplus::RectF m_rectSaveButton = { 0, 0, 0, 0 };      // 상단 저장하기 버튼

	ULONGLONG m_savedMessageShowUntil = 0;  // 이 시간(GetTickCount64)까지 윈도우에 "저장했습니다" 표시
	void UpdateLauncherButtonRect(int clientW, int clientH);
	void UpdateCenterButtonRect(int clientW, int clientH);
	void UpdateSaveButtonRect(int clientW, int clientH);
	bool IsPointInLauncherButton(POINT pt) const;
	bool IsPointInCenterButton(POINT pt) const;
	bool IsPointInSaveButton(POINT pt) const;
	void CenterViewOnMap(HWND hWnd);

	float m_currentFPS = 0.0f;

	void InitPalette();
	void DrawObjects(Gdiplus::Graphics* pGraphics);
	void DrawPreview(Gdiplus::Graphics* pGraphics);
	void HandlePlacingModeClick(POINT clickPoint, HWND hWnd);
	void HandleObjectSelectionClick(POINT clickPoint, HWND hWnd);
	void SpawnSelectedPaletteObjectAtViewCenter(HWND hWnd);
	/// 선택된 오브젝트를 팔레트에서 고른 오브젝트로 교체(위치 유지). 선택 없으면 무시.
	void ReplaceSelectedObjectWithPaletteSelection(HWND hWnd);
	void DeselectObject(HWND hWnd);
	void ExitAllEditModes();
	void AddObject(const ResourcePathUtils::ObjectResourceDef& obj);
	void RemoveObject(ResourcePathUtils::ObjectResourceDef* objToRemove);

	Gdiplus::PointF ScreenToWorld(Gdiplus::PointF screenPos) const;
	Gdiplus::PointF WorldToScreen(Gdiplus::PointF worldPos) const;
};

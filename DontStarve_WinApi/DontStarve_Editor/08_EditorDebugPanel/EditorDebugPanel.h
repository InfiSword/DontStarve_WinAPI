#pragma once

class EditorView;
class EditorPalette;
class EditorPivotEditor;
class EditorColliderEditor;
class EditorWalkableEditor;
class MapEditor;
class ObjectEditor;

class EditorDebugPanel
{
public:
	EditorDebugPanel() = default;
	~EditorDebugPanel() = default;

	// MapEditor용
	void SetDependencies(EditorView* pView, EditorPalette* pPalette, EditorPivotEditor* pPivotEditor,
		EditorColliderEditor* pColliderEditor, EditorWalkableEditor* pWalkableEditor, MapEditor* pMain);

	// ObjectEditor용
	void SetDependencies(EditorView* pView, EditorPalette* pPalette, EditorPivotEditor* pPivotEditor,
		EditorColliderEditor* pColliderEditor, ObjectEditor* pMain);

	void DrawDebugInfo(Gdiplus::Graphics* pGraphics);
	void HandleMouseWheel(int zDelta, int mx, int my);
	void ToggleVisibility();

	bool IsVisible() const { return m_showDebugInfo; }
	float GetScrollY() const { return m_debugInfoScrollY; }
	void SetScrollY(float y) { m_debugInfoScrollY = y; }
	Gdiplus::RectF GetViewportRect() const { return m_debugInfoViewportRect; }

private:
	EditorView* m_pView = nullptr;
	EditorPalette* m_pPalette = nullptr;
	EditorPivotEditor* m_pPivotEditor = nullptr;
	EditorColliderEditor* m_pColliderEditor = nullptr;
	EditorWalkableEditor* m_pWalkableEditor = nullptr;
	MapEditor* m_pMain = nullptr;
	ObjectEditor* m_pObjectMain = nullptr;

	bool m_showDebugInfo = true;
	float m_debugInfoScrollY = 0.0f;
	float m_debugInfoContentHeight = 0.0f;
	Gdiplus::RectF m_debugInfoViewportRect;
};

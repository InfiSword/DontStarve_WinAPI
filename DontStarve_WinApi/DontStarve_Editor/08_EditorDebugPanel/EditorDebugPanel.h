#pragma once

class EditorView;
class EditorPalette;
class EditorPivotEditor;
class EditorColliderEditor;
class EditorWalkableEditor;
class DontStarve_EditorMain;

class EditorDebugPanel
{
	friend class DontStarve_EditorMain;

public:
	EditorDebugPanel() = default;
	~EditorDebugPanel() = default;

	void SetDependencies(EditorView* pView, EditorPalette* pPalette, EditorPivotEditor* pPivotEditor,
		EditorColliderEditor* pColliderEditor, EditorWalkableEditor* pWalkableEditor, DontStarve_EditorMain* pMain);

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
	DontStarve_EditorMain* m_pMain = nullptr;

	bool m_showDebugInfo = true;
	float m_debugInfoScrollY = 0.0f;
	float m_debugInfoContentHeight = 0.0f;
	Gdiplus::RectF m_debugInfoViewportRect;
};

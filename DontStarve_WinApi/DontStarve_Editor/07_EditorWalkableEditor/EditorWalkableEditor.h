#pragma once

class EditorView;
class DontStarve_EditorMain;

class EditorWalkableEditor
{
	friend class DontStarve_EditorMain;

public:
	EditorWalkableEditor() = default;
	~EditorWalkableEditor() = default;

	void SetDependencies(EditorView* pView, DontStarve_EditorMain* pMain);

	void StartWalkableEdit();
	void EndWalkableEdit();
	void ToggleWalkableEditMode();

	void OnLeftButtonDown(POINT clickPoint, HWND hWnd);
	void OnLeftButtonUp();
	void OnMouseMove(POINT mousePos, HWND hWnd);

	void DrawWalkableAreas(Gdiplus::Graphics* pGraphics) const;

	bool IsWalkableEditMode() const { return m_isWalkableEditMode; }
	bool IsDraggingWalkable() const { return m_isDraggingWalkable; }

private:
	EditorView* m_pView = nullptr;
	DontStarve_EditorMain* m_pMain = nullptr;

	bool m_isWalkableEditMode = false;
	bool m_isDraggingWalkable = false;
	POINT m_walkableDragStart = { 0, 0 };
	POINT m_walkableDragEnd = { 0, 0 };
};

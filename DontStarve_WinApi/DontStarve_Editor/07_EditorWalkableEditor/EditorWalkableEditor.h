#pragma once

class EditorView;
class MapEditor;

enum class WalkablePaintMode { PaintBlocked, PaintWalkable };

class EditorWalkableEditor
{
	friend class MapEditor;

public:
	EditorWalkableEditor() = default;
	~EditorWalkableEditor() = default;

	void SetDependencies(EditorView* pView, MapEditor* pMain);

	void StartWalkableEdit();
	void EndWalkableEdit();
	void ToggleWalkableEditMode();

	void SetAllMapBlocked();
	bool HandleToolbarClick(POINT pt, int clientW, int clientH);

	void OnLeftButtonDown(POINT clickPoint, HWND hWnd);
	void OnLeftButtonUp();
	void OnMouseMove(POINT mousePos, HWND hWnd);

	void DrawWalkableAreas(Gdiplus::Graphics* pGraphics) const;

	bool IsWalkableEditMode() const { return m_isWalkableEditMode; }
	bool IsDraggingWalkable() const { return m_isDraggingWalkable; }

private:
	EditorView* m_pView = nullptr;
	MapEditor* m_pMain = nullptr;

	WalkablePaintMode m_paintMode = WalkablePaintMode::PaintBlocked;
	bool m_isWalkableEditMode = false;
	bool m_isDraggingWalkable = false;
	POINT m_walkableDragStart = { 0, 0 };
	POINT m_walkableDragEnd = { 0, 0 };
};

#pragma once

namespace ResourcePathUtils { struct ObjectResourceDef; }
class EditorView;
class EditorResourceManager;

class EditorPivotEditor
{
public:
	EditorPivotEditor() = default;
	~EditorPivotEditor() = default;

	void SetDependencies(EditorView* pView, const EditorResourceManager* pResources);

	void StartPivotEdit(ResourcePathUtils::ObjectResourceDef* pObject);
	void EndPivotEdit();
	void UpdatePivotEdit(POINT mousePos);

	void DrawPivotEditor(Gdiplus::Graphics* pGraphics) const;

	bool IsPivotEditMode() const { return m_isPivotEditMode; }
	ResourcePathUtils::ObjectResourceDef* GetEditingObject() const { return m_editingObject; }

	void ShowPivotDialog(HWND parent);

private:
	EditorView* m_pView = nullptr;
	const EditorResourceManager* m_pResources = nullptr;

	bool m_isPivotEditMode = false;
	POINT m_pivotEditPos = { 0, 0 };
	float m_currentPivotX = 0.5f;
	float m_currentPivotY = 1.0f;
	ResourcePathUtils::ObjectResourceDef* m_editingObject = nullptr;
};

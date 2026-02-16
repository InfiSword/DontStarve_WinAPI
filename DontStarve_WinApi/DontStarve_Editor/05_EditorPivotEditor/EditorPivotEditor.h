#pragma once

struct GameObjectData;
class EditorView;
class EditorResourceManager;

class EditorPivotEditor
{
public:
	EditorPivotEditor() = default;
	~EditorPivotEditor() = default;

	void SetDependencies(EditorView* pView, const EditorResourceManager* pResources);

	void StartPivotEdit(GameObjectData* pObject);
	void EndPivotEdit();
	void UpdatePivotEdit(POINT mousePos);

	void DrawPivotEditor(Gdiplus::Graphics* pGraphics) const;

	bool IsPivotEditMode() const { return m_isPivotEditMode; }
	GameObjectData* GetEditingObject() const { return m_editingObject; }

private:
	EditorView* m_pView = nullptr;
	const EditorResourceManager* m_pResources = nullptr;

	bool m_isPivotEditMode = false;
	POINT m_pivotEditPos = { 0, 0 };
	float m_currentPivotX = 0.5f;
	float m_currentPivotY = 1.0f;
	GameObjectData* m_editingObject = nullptr;
};

#pragma once

struct GameObjectData;
class EditorView;
class EditorResourceManager;
class DontStarve_EditorMain;

class EditorColliderEditor
{
	friend class DontStarve_EditorMain;

public:
	EditorColliderEditor() = default;
	~EditorColliderEditor() = default;

	void SetDependencies(EditorView* pView, const EditorResourceManager* pResources, DontStarve_EditorMain* pMain);

	void StartColliderEdit(GameObjectData* obj);
	void EndColliderEdit();
	int GetColliderHandleAt(POINT screenPos);
	void UpdateColliderDrag(POINT mousePos);
	void ApplyColliderToSameType(GameObjectData* source = nullptr);

	void DrawColliders(Gdiplus::Graphics* pGraphics) const;

	bool IsColliderEditMode() const { return m_isColliderEditMode; }
	GameObjectData* GetEditingColliderObject() const { return m_editingColliderObject; }
	bool IsDraggingCollider() const { return m_isDraggingCollider; }
	int GetDraggingHandle() const { return m_draggingHandle; }

	void OnLeftButtonDown(POINT clickPoint, HWND hWnd);
	void OnLeftButtonUp();
	void OnMouseMove(POINT mousePos, HWND hWnd);

	void ToggleColliderType();

private:
	static const int MIN_COLLIDER_SIZE = 4;
	static const float MIN_COLLIDER_RADIUS;

	EditorView* m_pView = nullptr;
	const EditorResourceManager* m_pResources = nullptr;
	DontStarve_EditorMain* m_pMain = nullptr;

	bool m_isColliderEditMode = false;
	GameObjectData* m_editingColliderObject = nullptr;
	bool m_isDraggingCollider = false;
	POINT m_colliderEditStartMousePos = { 0, 0 };
	RECT m_initialColliderRect = { 0 };
	float m_initialColliderCenterX = 0.0f;
	float m_initialColliderCenterY = 0.0f;
	float m_initialColliderRadius = 0.0f;
	int m_draggingHandle = -1;

	const ObjectVariant* GetObjectVariant(GameObjectType type, GameObjectID id) const;
};

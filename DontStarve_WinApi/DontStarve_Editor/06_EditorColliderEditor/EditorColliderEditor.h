#pragma once

namespace ResourcePathUtils { struct ObjectResourceDef; }
class EditorView;
class EditorResourceManager;
class ObjectEditor;

class EditorColliderEditor
{
public:
	EditorColliderEditor() = default;
	~EditorColliderEditor() = default;

	void SetDependencies(EditorView* pView, EditorResourceManager* pResources, ObjectEditor* pContext);

	void StartColliderEdit(ResourcePathUtils::ObjectResourceDef* obj);
	void EndColliderEdit();
	int GetColliderHandleAt(POINT screenPos);
	void UpdateColliderDrag(POINT mousePos);
	void ApplyColliderToSameType(ResourcePathUtils::ObjectResourceDef* source = nullptr);

	void DrawColliders(Gdiplus::Graphics* pGraphics) const;

	bool IsColliderEditMode() const { return m_isColliderEditMode; }
	ResourcePathUtils::ObjectResourceDef* GetEditingColliderObject() const { return m_editingColliderObject; }
	bool IsDraggingCollider() const { return m_isDraggingCollider; }
	int GetDraggingHandle() const { return m_draggingHandle; }

	void OnLeftButtonDown(POINT clickPoint, HWND hWnd);
	void OnLeftButtonUp();
	void OnMouseMove(POINT mousePos, HWND hWnd);

	void ToggleColliderType();
	void ShowColliderDialog(HWND parent);
	void SaveEditingObjectToGameData();

	int GetMinColliderSize() const { return MIN_COLLIDER_SIZE; }
	float GetMinColliderRadius() const { return MIN_COLLIDER_RADIUS; }

private:
	static const int MIN_COLLIDER_SIZE = 4;
	static const float MIN_COLLIDER_RADIUS;

	EditorView* m_pView = nullptr;
	EditorResourceManager* m_pResources = nullptr;
	ObjectEditor* m_pContext = nullptr;

	bool m_isColliderEditMode = false;
	ResourcePathUtils::ObjectResourceDef* m_editingColliderObject = nullptr;
	bool m_isDraggingCollider = false;
	POINT m_colliderEditStartMousePos = { 0, 0 };
	RECT m_initialColliderRect = { 0 };
	float m_initialColliderCenterX = 0.0f;
	float m_initialColliderCenterY = 0.0f;
	float m_initialColliderRadius = 0.0f;
	int m_draggingHandle = -1;

	const ResourcePathUtils::ObjectResourceDef* GetObjectVariant(GameObjectType type, GameObjectID id) const;
};

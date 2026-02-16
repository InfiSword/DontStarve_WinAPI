#include "../pch.h"
#include "EditorColliderEditor.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "../03_EditorMapFileIO/EditorMapFileIO.h"
#include "../00_MainEditor/DontStarve_EditorMain.h"
#include "Struct.h"
#include <algorithm>
#include <cmath>

const float EditorColliderEditor::MIN_COLLIDER_RADIUS = 2.0f;

void EditorColliderEditor::SetDependencies(EditorView* pView, const EditorResourceManager* pResources, DontStarve_EditorMain* pMain) {
	m_pView = pView;
	m_pResources = pResources;
	m_pMain = pMain;
}

const ObjectVariant* EditorColliderEditor::GetObjectVariant(GameObjectType type, GameObjectID id) const {
	if (!m_pResources) return nullptr;
	return m_pResources->GetObjectVariant(type, id);
}

void EditorColliderEditor::StartColliderEdit(GameObjectData* obj) {
	if (!obj || !m_pResources) return;

	m_isColliderEditMode = true;
	m_editingColliderObject = obj;

	if (!m_editingColliderObject->hasCollider) {
		m_editingColliderObject->hasCollider = true;
		m_editingColliderObject->colliderType = COLLIDER_BOX;

		const ObjectVariant* ov = GetObjectVariant(obj->type, obj->id);
		if (!ov) {
			OutputDebugStringW(L"Error: ObjectVariant not found for collider edit.\n");
			m_isColliderEditMode = false;
			m_editingColliderObject = nullptr;
			return;
		}

		int imageWidth = (int)ov->sourceRect.Width;
		int imageHeight = (int)ov->sourceRect.Height;

		m_editingColliderObject->colliderOffsetX = -(int)(ov->pivotX * imageWidth);
		m_editingColliderObject->colliderOffsetY = -(int)(ov->pivotY * imageHeight);
		m_editingColliderObject->colliderWidth = imageWidth;
		m_editingColliderObject->colliderHeight = imageHeight;

		m_editingColliderObject->colliderCenterX = imageWidth * (0.5f - ov->pivotX);
		m_editingColliderObject->colliderCenterY = imageHeight * (0.5f - ov->pivotY);
		float smallerSize = (imageWidth < imageHeight) ? (float)imageWidth : (float)imageHeight;
		m_editingColliderObject->colliderRadius = smallerSize * 0.5f;
	}

	if (m_editingColliderObject->colliderType == COLLIDER_BOX) {
		m_initialColliderRect = {
			m_editingColliderObject->colliderOffsetX,
			m_editingColliderObject->colliderOffsetY,
			m_editingColliderObject->colliderOffsetX + m_editingColliderObject->colliderWidth,
			m_editingColliderObject->colliderOffsetY + m_editingColliderObject->colliderHeight
		};
	}
	else if (m_editingColliderObject->colliderType == COLLIDER_CIRCLE) {
		m_initialColliderCenterX = m_editingColliderObject->colliderCenterX;
		m_initialColliderCenterY = m_editingColliderObject->colliderCenterY;
		m_initialColliderRadius = m_editingColliderObject->colliderRadius;
	}
}

void EditorColliderEditor::EndColliderEdit() {
	m_isColliderEditMode = false;
	m_editingColliderObject = nullptr;
	m_isDraggingCollider = false;
	m_draggingHandle = -1;
}

int EditorColliderEditor::GetColliderHandleAt(POINT screenPos) {
	if (!m_isColliderEditMode || !m_editingColliderObject || !m_editingColliderObject->hasCollider || !m_pView) return -1;

	float handleSize = 8.0f;
	float halfHandle = handleSize / 2.0f;
	float clickTolerance = handleSize / 2.0f;

	if (m_editingColliderObject->colliderType == COLLIDER_BOX) {
		float objRenderX = (float)m_editingColliderObject->x * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
		float objRenderY = (float)m_editingColliderObject->y * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;

		float colliderRenderX = objRenderX + (m_editingColliderObject->colliderOffsetX * m_pView->GetZoomFactor());
		float colliderRenderY = objRenderY + (m_editingColliderObject->colliderOffsetY * m_pView->GetZoomFactor());
		float colliderRenderWidth = (float)m_editingColliderObject->colliderWidth * m_pView->GetZoomFactor();
		float colliderRenderHeight = (float)m_editingColliderObject->colliderHeight * m_pView->GetZoomFactor();

		Gdiplus::PointF handleCenters[4];
		handleCenters[0] = Gdiplus::PointF(colliderRenderX, colliderRenderY);
		handleCenters[1] = Gdiplus::PointF(colliderRenderX + colliderRenderWidth, colliderRenderY);
		handleCenters[2] = Gdiplus::PointF(colliderRenderX, colliderRenderY + colliderRenderHeight);
		handleCenters[3] = Gdiplus::PointF(colliderRenderX + colliderRenderWidth, colliderRenderY + colliderRenderHeight);

		for (int i = 0; i < 4; ++i) {
			if (abs(screenPos.x - handleCenters[i].X) < clickTolerance &&
				abs(screenPos.y - handleCenters[i].Y) < clickTolerance) {
				return i;
			}
		}

		Gdiplus::RectF colliderBounds(colliderRenderX, colliderRenderY, colliderRenderWidth, colliderRenderHeight);
		if (colliderBounds.Contains((float)screenPos.x, (float)screenPos.y)) {
			return 4;
		}
	}
	else if (m_editingColliderObject->colliderType == COLLIDER_CIRCLE) {
		float worldCenterX = m_editingColliderObject->x + m_editingColliderObject->colliderCenterX;
		float worldCenterY = m_editingColliderObject->y + m_editingColliderObject->colliderCenterY;
		float radius = m_editingColliderObject->colliderRadius;

		float screenCenterX = worldCenterX * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
		float screenCenterY = worldCenterY * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;
		float screenRadius = radius * m_pView->GetZoomFactor();

		float radiusHandleX = screenCenterX + screenRadius;
		float radiusHandleY = screenCenterY;
		float dx = (float)screenPos.x - radiusHandleX;
		float dy = (float)screenPos.y - radiusHandleY;
		float distance = sqrtf(dx * dx + dy * dy);
		if (distance < clickTolerance) {
			return 5;
		}

		dx = (float)screenPos.x - screenCenterX;
		dy = (float)screenPos.y - screenCenterY;
		distance = sqrtf(dx * dx + dy * dy);
		if (distance < clickTolerance) {
			return 4;
		}

		if (distance <= screenRadius) {
			return 4;
		}
	}

	return -1;
}

void EditorColliderEditor::UpdateColliderDrag(POINT mousePos) {
	if (!m_isDraggingCollider || !m_editingColliderObject || !m_pView) return;

	int deltaX = mousePos.x - m_colliderEditStartMousePos.x;
	int deltaY = mousePos.y - m_colliderEditStartMousePos.y;

	float unzoomedDeltaX = deltaX / m_pView->GetZoomFactor();
	float unzoomedDeltaY = deltaY / m_pView->GetZoomFactor();

	if (m_editingColliderObject->colliderType == COLLIDER_BOX) {
		int unzoomedDeltaXInt = (int)unzoomedDeltaX;
		int unzoomedDeltaYInt = (int)unzoomedDeltaY;

		if (m_draggingHandle == 4) {
			m_editingColliderObject->colliderOffsetX = m_initialColliderRect.left + unzoomedDeltaXInt;
			m_editingColliderObject->colliderOffsetY = m_initialColliderRect.top + unzoomedDeltaYInt;
		}
		else {
			int newLeft = m_initialColliderRect.left;
			int newTop = m_initialColliderRect.top;
			int newRight = m_initialColliderRect.right;
			int newBottom = m_initialColliderRect.bottom;

			if (m_draggingHandle == 0 || m_draggingHandle == 2) {
				newLeft = m_initialColliderRect.left + unzoomedDeltaXInt;
			}
			if (m_draggingHandle == 0 || m_draggingHandle == 1) {
				newTop = m_initialColliderRect.top + unzoomedDeltaYInt;
			}
			if (m_draggingHandle == 1 || m_draggingHandle == 3) {
				newRight = m_initialColliderRect.right + unzoomedDeltaXInt;
			}
			if (m_draggingHandle == 2 || m_draggingHandle == 3) {
				newBottom = m_initialColliderRect.bottom + unzoomedDeltaYInt;
			}

			if (newRight - newLeft < MIN_COLLIDER_SIZE) {
				if (m_draggingHandle == 0 || m_draggingHandle == 2) newLeft = newRight - MIN_COLLIDER_SIZE;
				else newRight = newLeft + MIN_COLLIDER_SIZE;
			}
			if (newBottom - newTop < MIN_COLLIDER_SIZE) {
				if (m_draggingHandle == 0 || m_draggingHandle == 1) newTop = newBottom - MIN_COLLIDER_SIZE;
				else newBottom = newTop + MIN_COLLIDER_SIZE;
			}

			m_editingColliderObject->colliderOffsetX = newLeft;
			m_editingColliderObject->colliderOffsetY = newTop;
			m_editingColliderObject->colliderWidth = newRight - newLeft;
			m_editingColliderObject->colliderHeight = newBottom - newTop;
		}
	}
	else if (m_editingColliderObject->colliderType == COLLIDER_CIRCLE) {
		if (m_draggingHandle == 4) {
			m_editingColliderObject->colliderCenterX = m_initialColliderCenterX + unzoomedDeltaX;
			m_editingColliderObject->colliderCenterY = m_initialColliderCenterY + unzoomedDeltaY;
		}
		else if (m_draggingHandle == 5) {
			float objRenderX = (float)m_editingColliderObject->x * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
			float objRenderY = (float)m_editingColliderObject->y * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;
			float worldCenterX = m_editingColliderObject->x + m_initialColliderCenterX;
			float worldCenterY = m_editingColliderObject->y + m_initialColliderCenterY;
			float screenCenterX = worldCenterX * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
			float screenCenterY = worldCenterY * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;

			float dx = (float)mousePos.x - screenCenterX;
			float dy = (float)mousePos.y - screenCenterY;
			float screenDistance = sqrtf(dx * dx + dy * dy);
			float newRadius = screenDistance / m_pView->GetZoomFactor();

			if (newRadius < MIN_COLLIDER_RADIUS) {
				newRadius = MIN_COLLIDER_RADIUS;
			}

			m_editingColliderObject->colliderRadius = newRadius;
		}
	}
}

void EditorColliderEditor::ApplyColliderToSameType(GameObjectData* source) {
	if (!m_pMain) return;
	GameObjectData* src = source ? source : m_editingColliderObject;
	if (!src) return;
	if (!source && (!m_isColliderEditMode || !m_editingColliderObject)) return;

	// Main의 m_gameObjects와 m_colliderTemplates에 friend로 접근
	std::vector<GameObjectData>& gameObjects = m_pMain->m_gameObjects;
	std::map<std::pair<int, int>, DontStarve_EditorMain::ColliderTemplate>& colliderTemplates = m_pMain->m_colliderTemplates;

	int appliedCount = 0;
	for (GameObjectData& obj : gameObjects) {
		if (&obj == src) continue;
		if (obj.type != src->type || obj.id != src->id) continue;
		obj.hasCollider = src->hasCollider;
		obj.colliderType = src->colliderType;
		obj.colliderOffsetX = src->colliderOffsetX;
		obj.colliderOffsetY = src->colliderOffsetY;
		obj.colliderWidth = src->colliderWidth;
		obj.colliderHeight = src->colliderHeight;
		obj.colliderCenterX = src->colliderCenterX;
		obj.colliderCenterY = src->colliderCenterY;
		obj.colliderRadius = src->colliderRadius;
		appliedCount++;
	}

	DontStarve_EditorMain::ColliderTemplate t;
	t.hasCollider = src->hasCollider;
	t.colliderType = src->colliderType;
	t.colliderOffsetX = src->colliderOffsetX;
	t.colliderOffsetY = src->colliderOffsetY;
	t.colliderWidth = src->colliderWidth;
	t.colliderHeight = src->colliderHeight;
	t.colliderCenterX = src->colliderCenterX;
	t.colliderCenterY = src->colliderCenterY;
	t.colliderRadius = src->colliderRadius;
	colliderTemplates[std::make_pair((int)src->type, (int)src->id)] = t;
	EditorMapFileIO::SaveColliderTemplates(m_pMain);

	std::wstringstream ss;
	ss << L"ApplyColliderToSameType: applied to " << appliedCount << L" object(s)\n";
	OutputDebugStringW(ss.str().c_str());
}

void EditorColliderEditor::DrawColliders(Gdiplus::Graphics* pGraphics) const {
	if (!pGraphics || !m_isColliderEditMode || !m_editingColliderObject || !m_pView || !m_pResources) return;

	GameObjectData& obj = *m_editingColliderObject;
	const ObjectVariant* ov = GetObjectVariant(obj.type, obj.id);
	if (!ov || !ov->pAtlasBitmap) return;

	Gdiplus::Pen colliderPen(Gdiplus::Color(255, 0, 255, 0), 2.0f);
	Gdiplus::SolidBrush handleBrush(Gdiplus::Color(255, 0, 255, 255));
	int handleSize = 8;
	Gdiplus::REAL handleSizeReal = (Gdiplus::REAL)handleSize;
	Gdiplus::REAL halfHandleReal = handleSizeReal / 2.0f;

	if (obj.colliderType == COLLIDER_BOX) {
		float colliderWorldX_top_left = (float)obj.x + obj.colliderOffsetX;
		float colliderWorldY_top_left = (float)obj.y + obj.colliderOffsetY;
		float colliderWidth = (float)obj.colliderWidth;
		float colliderHeight = (float)obj.colliderHeight;

		float colliderScreenX = colliderWorldX_top_left * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
		float colliderScreenY = colliderWorldY_top_left * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;
		float colliderScaledWidth = colliderWidth * m_pView->GetZoomFactor();
		float colliderScaledHeight = colliderHeight * m_pView->GetZoomFactor();

		pGraphics->DrawRectangle(&colliderPen, colliderScreenX, colliderScreenY, colliderScaledWidth, colliderScaledHeight);

		pGraphics->FillRectangle(&handleBrush, colliderScreenX - halfHandleReal, colliderScreenY - halfHandleReal, handleSizeReal, handleSizeReal);
		pGraphics->FillRectangle(&handleBrush, colliderScreenX + colliderScaledWidth - halfHandleReal, colliderScreenY - halfHandleReal, handleSizeReal, handleSizeReal);
		pGraphics->FillRectangle(&handleBrush, colliderScreenX - halfHandleReal, colliderScreenY + colliderScaledHeight - halfHandleReal, handleSizeReal, handleSizeReal);
		pGraphics->FillRectangle(&handleBrush, colliderScreenX + colliderScaledWidth - halfHandleReal, colliderScreenY + colliderScaledHeight - halfHandleReal, handleSizeReal, handleSizeReal);
	}
	else if (obj.colliderType == COLLIDER_CIRCLE) {
		float worldCenterX = obj.x + obj.colliderCenterX;
		float worldCenterY = obj.y + obj.colliderCenterY;
		float radius = obj.colliderRadius;

		float screenCenterX = worldCenterX * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
		float screenCenterY = worldCenterY * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;
		float screenRadius = radius * m_pView->GetZoomFactor();

		Gdiplus::RectF circleRect(screenCenterX - screenRadius, screenCenterY - screenRadius, screenRadius * 2.0f, screenRadius * 2.0f);
		pGraphics->DrawEllipse(&colliderPen, circleRect);

		float centerHandleX = screenCenterX - halfHandleReal;
		float centerHandleY = screenCenterY - halfHandleReal;
		pGraphics->FillRectangle(&handleBrush, centerHandleX, centerHandleY, handleSizeReal, handleSizeReal);

		float radiusHandleX = screenCenterX + screenRadius - halfHandleReal;
		float radiusHandleY = screenCenterY - halfHandleReal;
		pGraphics->FillRectangle(&handleBrush, radiusHandleX, radiusHandleY, handleSizeReal, handleSizeReal);
	}
}

void EditorColliderEditor::OnLeftButtonDown(POINT clickPoint, HWND hWnd) {
	if (!m_isColliderEditMode || !m_editingColliderObject) return;

	m_draggingHandle = GetColliderHandleAt(clickPoint);
	if (m_draggingHandle != -1) {
		m_isDraggingCollider = true;
		m_colliderEditStartMousePos = clickPoint;
		if (m_editingColliderObject->colliderType == COLLIDER_BOX) {
			m_initialColliderRect = {
				m_editingColliderObject->colliderOffsetX,
				m_editingColliderObject->colliderOffsetY,
				m_editingColliderObject->colliderOffsetX + m_editingColliderObject->colliderWidth,
				m_editingColliderObject->colliderOffsetY + m_editingColliderObject->colliderHeight
			};
		}
		else if (m_editingColliderObject->colliderType == COLLIDER_CIRCLE) {
			m_initialColliderCenterX = m_editingColliderObject->colliderCenterX;
			m_initialColliderCenterY = m_editingColliderObject->colliderCenterY;
			m_initialColliderRadius = m_editingColliderObject->colliderRadius;
		}
		SetCapture(hWnd);
	}
}

void EditorColliderEditor::OnLeftButtonUp() {
	if (m_isDraggingCollider) {
		m_isDraggingCollider = false;
		m_draggingHandle = -1;
	}
}

void EditorColliderEditor::OnMouseMove(POINT mousePos, HWND hWnd) {
	if (m_isDraggingCollider && m_editingColliderObject) {
		UpdateColliderDrag(mousePos);
		InvalidateRect(hWnd, NULL, FALSE);
	}
}

void EditorColliderEditor::ToggleColliderType() {
	if (!m_isColliderEditMode || !m_editingColliderObject || !m_pResources) return;

	if (m_editingColliderObject->colliderType == COLLIDER_BOX) {
		m_editingColliderObject->colliderType = COLLIDER_CIRCLE;
		const ObjectVariant* ov = GetObjectVariant(m_editingColliderObject->type, m_editingColliderObject->id);
		if (ov) {
			int imageWidth = (int)ov->sourceRect.Width;
			int imageHeight = (int)ov->sourceRect.Height;
			m_editingColliderObject->colliderCenterX = imageWidth * (0.5f - ov->pivotX);
			m_editingColliderObject->colliderCenterY = imageHeight * (0.5f - ov->pivotY);
			float smallerSize = (imageWidth < imageHeight) ? (float)imageWidth : (float)imageHeight;
			m_editingColliderObject->colliderRadius = smallerSize * 0.5f;
		}
	}
	else {
		m_editingColliderObject->colliderType = COLLIDER_BOX;
		const ObjectVariant* ov = GetObjectVariant(m_editingColliderObject->type, m_editingColliderObject->id);
		if (ov) {
			int imageWidth = (int)ov->sourceRect.Width;
			int imageHeight = (int)ov->sourceRect.Height;
			m_editingColliderObject->colliderOffsetX = -(int)(ov->pivotX * imageWidth);
			m_editingColliderObject->colliderOffsetY = -(int)(ov->pivotY * imageHeight);
			m_editingColliderObject->colliderWidth = imageWidth;
			m_editingColliderObject->colliderHeight = imageHeight;
		}
	}
}

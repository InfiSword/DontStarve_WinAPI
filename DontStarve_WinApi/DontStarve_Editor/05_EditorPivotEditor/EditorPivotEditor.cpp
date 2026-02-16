#include "../pch.h"
#include "EditorPivotEditor.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "Struct.h"

void EditorPivotEditor::SetDependencies(EditorView* pView, const EditorResourceManager* pResources) {
	m_pView = pView;
	m_pResources = pResources;
}

void EditorPivotEditor::StartPivotEdit(GameObjectData* pObject) {
	if (!pObject || !m_pView || !m_pResources) return;

	m_editingObject = pObject;
	m_currentPivotX = pObject->pivotX;
	m_currentPivotY = pObject->pivotY;
	m_isPivotEditMode = true;

	const ObjectVariant* ov = m_pResources->GetObjectVariant(pObject->type, pObject->id);
	if (!ov) {
		OutputDebugStringW(L"Error: ObjectVariant not found for pivot edit.\n");
		m_isPivotEditMode = false;
		m_editingObject = nullptr;
		return;
	}

	float objWidth = (float)ov->sourceRect.Width;
	float objHeight = (float)ov->sourceRect.Height;

	float screenX_center = (float)pObject->x * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
	float screenY_center = (float)pObject->y * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;

	float scaledWidth = objWidth * m_pView->GetZoomFactor();
	float scaledHeight = objHeight * m_pView->GetZoomFactor();

	float imageRenderLeft = screenX_center - (ov->pivotX * scaledWidth);
	float imageRenderTop = screenY_center - (ov->pivotY * scaledHeight);

	m_pivotEditPos.x = (LONG)(imageRenderLeft + (m_currentPivotX * scaledWidth));
	m_pivotEditPos.y = (LONG)(imageRenderTop + (m_currentPivotY * scaledHeight));
}

void EditorPivotEditor::UpdatePivotEdit(POINT mousePos) {
	if (!m_editingObject || !m_isPivotEditMode || !m_pView || !m_pResources) return;

	const ObjectVariant* ov = m_pResources->GetObjectVariant(m_editingObject->type, m_editingObject->id);
	if (!ov) return;

	float objWidth = (float)ov->sourceRect.Width;
	float objHeight = (float)ov->sourceRect.Height;

	float screenX_center = (float)m_editingObject->x * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
	float screenY_center = (float)m_editingObject->y * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;

	float scaledWidth = objWidth * m_pView->GetZoomFactor();
	float scaledHeight = objHeight * m_pView->GetZoomFactor();

	float imageRenderLeft = screenX_center - (ov->pivotX * scaledWidth);
	float imageRenderTop = screenY_center - (ov->pivotY * scaledHeight);

	float localX = (mousePos.x - imageRenderLeft) / scaledWidth;
	float localY = (mousePos.y - imageRenderTop) / scaledHeight;

	m_currentPivotX = max(0.0f, min(1.0f, localX));
	m_currentPivotY = max(0.0f, min(1.0f, localY));

	m_editingObject->pivotX = m_currentPivotX;
	m_editingObject->pivotY = m_currentPivotY;
}

void EditorPivotEditor::EndPivotEdit() {
	m_isPivotEditMode = false;
	m_editingObject = nullptr;
}

void EditorPivotEditor::DrawPivotEditor(Gdiplus::Graphics* pGraphics) const {
	if (!pGraphics || !m_isPivotEditMode || !m_editingObject || !m_pView || !m_pResources) return;

	const ObjectVariant* ov = m_pResources->GetObjectVariant(m_editingObject->type, m_editingObject->id);
	if (!ov || !ov->pAtlasBitmap) return;

	float objWorldX = (float)m_editingObject->x;
	float objWorldY = (float)m_editingObject->y;

	float objWidth = (float)ov->sourceRect.Width;
	float objHeight = (float)ov->sourceRect.Height;

	float screenX_center = objWorldX * m_pView->GetZoomFactor() + m_pView->GetMapOffset().x;
	float screenY_center = objWorldY * m_pView->GetZoomFactor() + m_pView->GetMapOffset().y;

	float scaledWidth = objWidth * m_pView->GetZoomFactor();
	float scaledHeight = objHeight * m_pView->GetZoomFactor();

	float imageRenderLeft = screenX_center - (ov->pivotX * scaledWidth);
	float imageRenderTop = screenY_center - (ov->pivotY * scaledHeight);

	float pivotScreenX = imageRenderLeft + (m_editingObject->pivotX * scaledWidth);
	float pivotScreenY = imageRenderTop + (m_editingObject->pivotY * scaledHeight);

	Gdiplus::Pen pivotPen(Gdiplus::Color(255, 255, 0, 0), 2.0f);
	pGraphics->DrawLine(&pivotPen, pivotScreenX - 10, pivotScreenY, pivotScreenX + 10, pivotScreenY);
	pGraphics->DrawLine(&pivotPen, pivotScreenX, pivotScreenY - 10, pivotScreenX, pivotScreenY + 10);

	Gdiplus::Pen bboxPen(Gdiplus::Color(255, 0, 255, 255), 1.0f);
	pGraphics->DrawRectangle(&bboxPen, imageRenderLeft, imageRenderTop, scaledWidth, scaledHeight);
}

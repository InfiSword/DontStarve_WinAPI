#include "../pch.h"
#include "EditorPivotEditor.h"
#include "../Resource.h"
#include "../01_EditorView/EditorView.h"
#include "../02_EditorResourceManager/EditorResourceManager.h"
#include "Struct.h"

void EditorPivotEditor::SetDependencies(EditorView* pView, EditorResourceManager* pResources) {
	m_pView = pView;
	m_pResources = pResources;
}

void EditorPivotEditor::StartPivotEdit(ResourcePathUtils::ObjectResourceDef* pObject) {
	if (!pObject || !m_pView || !m_pResources) return;

	m_editingObject = pObject;
	m_currentPivotX = pObject->pivotX;
	m_currentPivotY = pObject->pivotY;
	m_isPivotEditMode = true;

	const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(pObject->id);
	if (!ov || ov->imageName.empty()) {
		m_isPivotEditMode = false;
		m_editingObject = nullptr;
		return;
	}
	std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
	std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
	if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) {
		m_isPivotEditMode = false;
		m_editingObject = nullptr;
		return;
	}
	float objWidth = (float)pBitmap->GetWidth();
	float objHeight = (float)pBitmap->GetHeight();

	float screenX_center = (float)pObject->x * m_pView->GetZoomFactor() + (float)m_pView->GetMapOffset().x;
	float screenY_center = (float)pObject->y * m_pView->GetZoomFactor() + (float)m_pView->GetMapOffset().y;

	float scaledWidth = objWidth * m_pView->GetZoomFactor();
	float scaledHeight = objHeight * m_pView->GetZoomFactor();

	float imageRenderLeft = screenX_center - (ov->pivotX * scaledWidth);
	float imageRenderTop = screenY_center - (ov->pivotY * scaledHeight);

	m_pivotEditPos.x = (LONG)(imageRenderLeft + (m_currentPivotX * scaledWidth));
	m_pivotEditPos.y = (LONG)(imageRenderTop + (m_currentPivotY * scaledHeight));
}

void EditorPivotEditor::UpdatePivotEdit(POINT mousePos) {
	if (!m_editingObject || !m_isPivotEditMode || !m_pView || !m_pResources) return;

	const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(m_editingObject->id);
	if (!ov || ov->imageName.empty()) return;

	std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
	std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
	if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) return;

	float objWidth = (float)pBitmap->GetWidth();
	float objHeight = (float)pBitmap->GetHeight();

	float screenX_center = (float)m_editingObject->x * m_pView->GetZoomFactor() + (float)m_pView->GetMapOffset().x;
	float screenY_center = (float)m_editingObject->y * m_pView->GetZoomFactor() + (float)m_pView->GetMapOffset().y;

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

	const ResourcePathUtils::ObjectResourceDef* ov = m_pResources->GetObjectVariant(m_editingObject->id);
	if (!ov || ov->imageName.empty()) return;

	std::wstring fullPath = ResourcePathUtils::BuildResourcePath(ov->baseDir, ov->imageName);
	std::shared_ptr<Gdiplus::Bitmap> pBitmap = m_pResources->GetCachedBitmap(fullPath);
	if (!pBitmap || pBitmap->GetLastStatus() != Gdiplus::Ok) return;

	float objWorldX = (float)m_editingObject->x;
	float objWorldY = (float)m_editingObject->y;

	float objWidth = (float)pBitmap->GetWidth();
	float objHeight = (float)pBitmap->GetHeight();

	float screenX_center = objWorldX * m_pView->GetZoomFactor() + (float)m_pView->GetMapOffset().x;
	float screenY_center = objWorldY * m_pView->GetZoomFactor() + (float)m_pView->GetMapOffset().y;

	float scaledWidth = objWidth * m_pView->GetZoomFactor();
	float scaledHeight = objHeight * m_pView->GetZoomFactor();

	float imageRenderLeft = screenX_center - (ov->pivotX * scaledWidth);
	float imageRenderTop = screenY_center - (ov->pivotY * scaledHeight);

	float pivotScreenX = imageRenderLeft + (m_editingObject->pivotX * scaledWidth);
	float pivotScreenY = imageRenderTop + (m_editingObject->pivotY * scaledHeight);

	Gdiplus::Pen pivotPen(Gdiplus::Color(255, 0, 200, 0), 2.0f);
	pGraphics->DrawLine(&pivotPen, pivotScreenX - 10, pivotScreenY, pivotScreenX + 10, pivotScreenY);
	pGraphics->DrawLine(&pivotPen, pivotScreenX, pivotScreenY - 10, pivotScreenX, pivotScreenY + 10);

	Gdiplus::Pen bboxPen(Gdiplus::Color(255, 0, 180, 0), 1.0f);
	pGraphics->DrawRectangle(&bboxPen, imageRenderLeft, imageRenderTop, scaledWidth, scaledHeight);
}

struct PivotDlgParam {
	float curX, curY;
	float outX, outY;
	bool ok;
};

static LRESULT CALLBACK PivotDlgProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam) {
	PivotDlgParam* p = (PivotDlgParam*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
	switch (msg) {
	case WM_CREATE:
	{
		CREATESTRUCT* cs = (CREATESTRUCT*)lParam;
		p = (PivotDlgParam*)cs->lpCreateParams;
		SetWindowLongPtrW(hWnd, GWLP_USERDATA, (LONG_PTR)p);
		HINSTANCE hInst = (HINSTANCE)cs->hInstance;
		CreateWindowW(L"Static", L"Pivot X (0.0~1.0):", WS_CHILD | WS_VISIBLE, 10, 12, 90, 18, hWnd, nullptr, hInst, nullptr);
		CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", nullptr, WS_CHILD | WS_VISIBLE, 105, 10, 60, 18, hWnd, (HMENU)(UINT_PTR)IDC_PIVOT_X, hInst, nullptr);
		CreateWindowW(L"Static", L"Pivot Y (0.0~1.0):", WS_CHILD | WS_VISIBLE, 10, 38, 90, 18, hWnd, nullptr, hInst, nullptr);
		CreateWindowExW(WS_EX_CLIENTEDGE, L"Edit", nullptr, WS_CHILD | WS_VISIBLE, 105, 36, 60, 18, hWnd, (HMENU)(UINT_PTR)IDC_PIVOT_Y, hInst, nullptr);
		CreateWindowW(L"Button", L"OK", WS_CHILD | WS_VISIBLE | BS_DEFPUSHBUTTON, 45, 62, 50, 22, hWnd, (HMENU)IDOK, hInst, nullptr);
		CreateWindowW(L"Button", L"Cancel", WS_CHILD | WS_VISIBLE, 105, 62, 50, 22, hWnd, (HMENU)IDCANCEL, hInst, nullptr);
		WCHAR buf[32];
		swprintf_s(buf, L"%.3f", p->curX);
		SetDlgItemTextW(hWnd, IDC_PIVOT_X, buf);
		swprintf_s(buf, L"%.3f", p->curY);
		SetDlgItemTextW(hWnd, IDC_PIVOT_Y, buf);
		return 0;
	}
	case WM_COMMAND:
		if (LOWORD(wParam) == IDOK) {
			p = (PivotDlgParam*)GetWindowLongPtrW(hWnd, GWLP_USERDATA);
			if (p) {
				WCHAR buf[32];
				float x = 0.5f, y = 0.5f;
				if (GetDlgItemTextW(hWnd, IDC_PIVOT_X, buf, 32) > 0) x = (float)wcstod(buf, nullptr);
				if (GetDlgItemTextW(hWnd, IDC_PIVOT_Y, buf, 32) > 0) y = (float)wcstod(buf, nullptr);
				if (x < 0.0f) x = 0.0f; if (x > 1.0f) x = 1.0f;
				if (y < 0.0f) y = 0.0f; if (y > 1.0f) y = 1.0f;
				p->outX = x;
				p->outY = y;
				p->ok = true;
			}
			DestroyWindow(hWnd);
			return 0;
		}
		if (LOWORD(wParam) == IDCANCEL) {
			DestroyWindow(hWnd);
			return 0;
		}
		break;
	case WM_CLOSE:
		DestroyWindow(hWnd);
		return 0;
	}
	return DefWindowProcW(hWnd, msg, wParam, lParam);
}

void EditorPivotEditor::ShowPivotDialog(HWND parent) {
	if (!m_isPivotEditMode || !m_editingObject) return;
	PivotDlgParam param = { m_currentPivotX, m_currentPivotY, 0.0f, 0.0f, false };
	HINSTANCE hInst = (HINSTANCE)GetModuleHandle(nullptr);
	if (!hInst) hInst = (HINSTANCE)GetWindowLongPtrW(parent, GWLP_HINSTANCE);
	WNDCLASSEXW wc = {};
	wc.cbSize = sizeof(wc);
	if (!GetClassInfoExW(hInst, L"PivotDlgClass", &wc)) {
		wc.style = CS_HREDRAW | CS_VREDRAW;
		wc.lpfnWndProc = PivotDlgProc;
		wc.cbClsExtra = 0;
		wc.cbWndExtra = 0;
		wc.hInstance = hInst;
		wc.hCursor = LoadCursor(nullptr, IDC_ARROW);
		wc.hbrBackground = (HBRUSH)(COLOR_WINDOW + 1);
		wc.lpszClassName = L"PivotDlgClass";
		if (!RegisterClassExW(&wc)) return;
	}
	HWND hDlg = CreateWindowExW(WS_EX_DLGMODALFRAME | WS_EX_TOPMOST, L"PivotDlgClass", L"피벗 입력",
		WS_POPUP | WS_CAPTION | WS_SYSMENU, 0, 0, 240, 120, parent, nullptr, hInst, &param);
	if (!hDlg) return;
	SetWindowLongPtrW(hDlg, GWLP_USERDATA, (LONG_PTR)&param);
	RECT rc, rcMain;
	GetWindowRect(hDlg, &rc);
	GetWindowRect(parent, &rcMain);
	SetWindowPos(hDlg, nullptr,
		rcMain.left + (rcMain.right - rcMain.left - (rc.right - rc.left)) / 2,
		rcMain.top + (rcMain.bottom - rcMain.top - (rc.bottom - rc.top)) / 2,
		0, 0, SWP_NOSIZE | SWP_NOZORDER);
	ShowWindow(hDlg, SW_SHOW);
	EnableWindow(parent, FALSE);
	MSG dlgMsg;
	while (IsWindow(hDlg) && GetMessage(&dlgMsg, nullptr, 0, 0)) {
		if (!IsDialogMessage(hDlg, &dlgMsg)) {
			TranslateMessage(&dlgMsg);
			DispatchMessage(&dlgMsg);
		}
	}
	EnableWindow(parent, TRUE);
	SetForegroundWindow(parent);
	if (param.ok && m_editingObject) {
		m_currentPivotX = param.outX;
		m_currentPivotY = param.outY;
		m_editingObject->pivotX = param.outX;
		m_editingObject->pivotY = param.outY;
		if (m_pResources)
			m_pResources->SaveObjectResourceOverride(m_editingObject->id, *m_editingObject);
	}
}

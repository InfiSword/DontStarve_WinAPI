#pragma once

#include "IEditorScreen.h"

// 런처 화면: MapEditor / ObjectEditor 버튼 2개 표시, 클릭 시 전환 요청
class EditorLauncher : public IEditorScreen {
public:
	EditorLauncher();
	virtual ~EditorLauncher();

	void Initialize() override;
	void Update() override;
	void Render() override;
	void Release() override;
	LRESULT HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam) override;
	EditorScreenSwitch GetRequestedSwitch() override;

private:
	Gdiplus::Graphics* m_pGraphics;
	Gdiplus::Bitmap* m_pDoubleBufferBitmap;
	EditorScreenSwitch m_requestedSwitch;

	// 버튼 영역 (클라이언트 좌표)
	Gdiplus::RectF m_rectMapEditor;
	Gdiplus::RectF m_rectObjectEditor;

	void DrawButtons(Gdiplus::Graphics* pGraphics, int clientW, int clientH);
	bool HitTestMapEditor(int x, int y) const;
	bool HitTestObjectEditor(int x, int y) const;
};

#include "../pch.h"
#include "EditorLauncher.h"

EditorLauncher::EditorLauncher()
	: m_pGraphics(nullptr), m_pDoubleBufferBitmap(nullptr),
	m_requestedSwitch(EditorScreenSwitch::None)
{
}

EditorLauncher::~EditorLauncher()
{
	Release();
}

void EditorLauncher::Initialize()
{
	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	int w = clientRect.right - clientRect.left;
	int h = clientRect.bottom - clientRect.top;
	if (w <= 0) w = WINCX;
	if (h <= 0) h = WINCY;

	m_pDoubleBufferBitmap = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
	m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
	m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
	m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);

	// 버튼 영역 (중앙에 2개, 세로 배치)
	const int btnW = 280;
	const int btnH = 56;
	const int gap = 24;
	int totalH = btnH * 2 + gap;
	int left = (w - btnW) / 2;
	int top = (h - totalH) / 2;
	m_rectMapEditor = Gdiplus::RectF((Gdiplus::REAL)left, (Gdiplus::REAL)top, (Gdiplus::REAL)btnW, (Gdiplus::REAL)btnH);
	m_rectObjectEditor = Gdiplus::RectF((Gdiplus::REAL)left, (Gdiplus::REAL)(top + btnH + gap), (Gdiplus::REAL)btnW, (Gdiplus::REAL)btnH);
}

void EditorLauncher::Update()
{
}

void EditorLauncher::Render()
{
	if (!m_pGraphics) return;

	RECT clientRect;
	GetClientRect(g_hWnd, &clientRect);
	int w = clientRect.right - clientRect.left;
	int h = clientRect.bottom - clientRect.top;
	if (w <= 0 || h <= 0) return;

	if (m_pDoubleBufferBitmap && (m_pDoubleBufferBitmap->GetWidth() != (UINT)w || m_pDoubleBufferBitmap->GetHeight() != (UINT)h)) {
		Release();
		m_pDoubleBufferBitmap = new Gdiplus::Bitmap(w, h, PixelFormat32bppARGB);
		m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
		m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		const int btnW = 280;
		const int btnH = 56;
		const int gap = 24;
		int totalH = btnH * 2 + gap;
		int left = (w - btnW) / 2;
		int top = (h - totalH) / 2;
		m_rectMapEditor = Gdiplus::RectF((Gdiplus::REAL)left, (Gdiplus::REAL)top, (Gdiplus::REAL)btnW, (Gdiplus::REAL)btnH);
		m_rectObjectEditor = Gdiplus::RectF((Gdiplus::REAL)left, (Gdiplus::REAL)(top + btnH + gap), (Gdiplus::REAL)btnW, (Gdiplus::REAL)btnH);
	}

	m_pGraphics->Clear(Gdiplus::Color(255, 240, 240, 245));
	DrawButtons(m_pGraphics, w, h);

	HDC hdcScreen = GetDC(g_hWnd);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = NULL;
	Gdiplus::Color color(0, 0, 0, 0);
	if (m_pDoubleBufferBitmap->GetHBITMAP(color, &hBitmap) == Gdiplus::Ok && hBitmap) {
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
		BitBlt(hdcScreen, 0, 0, w, h, hdcMem, 0, 0, SRCCOPY);
		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
	}
	DeleteDC(hdcMem);
	ReleaseDC(g_hWnd, hdcScreen);
}

void EditorLauncher::Release()
{
	if (m_pGraphics) {
		delete m_pGraphics;
		m_pGraphics = nullptr;
	}
	if (m_pDoubleBufferBitmap) {
		delete m_pDoubleBufferBitmap;
		m_pDoubleBufferBitmap = nullptr;
	}
}

LRESULT EditorLauncher::HandleMessage(HWND hWnd, UINT message, WPARAM wParam, LPARAM lParam)
{
	switch (message) {
	case WM_LBUTTONDOWN: {
		int x = LOWORD(lParam);
		int y = HIWORD(lParam);
		if (HitTestMapEditor(x, y)) {
			m_requestedSwitch = EditorScreenSwitch::MapEditor;
			return 0;
		}
		if (HitTestObjectEditor(x, y)) {
			m_requestedSwitch = EditorScreenSwitch::ObjectEditor;
			return 0;
		}
	}
	break;
	default:
		break;
	}
	return DefWindowProc(hWnd, message, wParam, lParam);
}

EditorScreenSwitch EditorLauncher::GetRequestedSwitch()
{
	EditorScreenSwitch s = m_requestedSwitch;
	m_requestedSwitch = EditorScreenSwitch::None;
	return s;
}

void EditorLauncher::DrawButtons(Gdiplus::Graphics* pGraphics, int clientW, int clientH)
{
	using namespace Gdiplus;
	Font font(L"Malgun Gothic", 16, FontStyleBold);
	SolidBrush brushTitle(Color(255, 50, 50, 55));
	SolidBrush brushBtn(Color(255, 70, 130, 180));
	SolidBrush brushBtn2(Color(255, 100, 150, 100));
	Pen penBorder(Color(255, 50, 80, 120), 2.0f);
	SolidBrush brushText(Color(255, 255, 255, 255));
	StringFormat sf;
	sf.SetAlignment(StringAlignmentCenter);
	sf.SetLineAlignment(StringAlignmentCenter);

	// 제목
	RectF rectTitle(0, (REAL)clientH * 0.2f, (REAL)clientW, 40.0f);
	pGraphics->DrawString(L"DontStarve Editor", -1, &font, rectTitle, &sf, &brushTitle);

	// Map Editor 버튼
	pGraphics->FillRectangle(&brushBtn, m_rectMapEditor);
	pGraphics->DrawRectangle(&penBorder, m_rectMapEditor);
	RectF textRect1(m_rectMapEditor.X, m_rectMapEditor.Y, m_rectMapEditor.Width, m_rectMapEditor.Height);
	pGraphics->DrawString(L"Map Editor", -1, &font, textRect1, &sf, &brushText);

	// Object Editor 버튼
	pGraphics->FillRectangle(&brushBtn2, m_rectObjectEditor);
	pGraphics->DrawRectangle(&penBorder, m_rectObjectEditor);
	RectF textRect2(m_rectObjectEditor.X, m_rectObjectEditor.Y, m_rectObjectEditor.Width, m_rectObjectEditor.Height);
	pGraphics->DrawString(L"Object Editor", -1, &font, textRect2, &sf, &brushText);
}

bool EditorLauncher::HitTestMapEditor(int x, int y) const
{
	return m_rectMapEditor.Contains((Gdiplus::REAL)x, (Gdiplus::REAL)y) != FALSE;
}

bool EditorLauncher::HitTestObjectEditor(int x, int y) const
{
	return m_rectObjectEditor.Contains((Gdiplus::REAL)x, (Gdiplus::REAL)y) != FALSE;
}

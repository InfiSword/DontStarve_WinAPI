#include "99_Default/pch.h"
#include "GraphicsManager.h"

GraphicsManager::GraphicsManager()
    : m_pGraphics(nullptr), m_pDoubleBufferBitmap(nullptr), m_clientRect({ 0,0,0,0 })
{
}

GraphicsManager::~GraphicsManager()
{
	Release();
}

void GraphicsManager::Init() {
    if (!g_hWnd) return;
    GetClientRect(g_hWnd, &m_clientRect);

    int width = m_clientRect.right - m_clientRect.left;
    int height = m_clientRect.bottom - m_clientRect.top;


    m_pDoubleBufferBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
    m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);

    m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
    m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
    m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
}

void GraphicsManager::LateInit()
{

}

void GraphicsManager::Update(float deltaTime)
{

}

void GraphicsManager::LateUpdate()
{

}

void GraphicsManager::Render()
{
	if (!m_pGraphics || !m_pDoubleBufferBitmap || !g_hWnd) return;

	HDC hdcScreen = GetDC(g_hWnd);
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = NULL;
	Gdiplus::Color color(0, 0, 0, 0);

	if (m_pDoubleBufferBitmap->GetHBITMAP(color, &hBitmap) == Gdiplus::Ok && hBitmap) {
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
		BitBlt(hdcScreen, 0, 0, m_clientRect.right, m_clientRect.bottom, hdcMem, 0, 0, SRCCOPY);

		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
	}

	DeleteDC(hdcMem);
	ReleaseDC(g_hWnd, hdcScreen);
}

void GraphicsManager::Release()
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

Gdiplus::Graphics* GraphicsManager::GetGraphics()
{
    if (m_pGraphics) {
        m_pGraphics->Clear(Gdiplus::Color(255, 0, 0, 0));
    }
    return m_pGraphics;
}

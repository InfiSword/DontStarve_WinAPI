#include "99_Default/pch.h"
#include "GraphicsManager.h"

GraphicsManager::GraphicsManager()
    : m_pGraphics(nullptr), m_pDoubleBufferBitmap(nullptr), m_clientRect({ 0,0,0,0 })
{
}

GraphicsManager::~GraphicsManager()
{
}

void GraphicsManager::Init() {
    if (!g_hWnd) return;
    GetClientRect(g_hWnd, &m_clientRect);

    int width = m_clientRect.right - m_clientRect.left;
    int height = m_clientRect.bottom - m_clientRect.top;


    m_pDoubleBufferBitmap = new Gdiplus::Bitmap(width, height, PixelFormat32bppARGB);
    m_pGraphics = Gdiplus::Graphics::FromImage(m_pDoubleBufferBitmap);
 
    m_pGraphics->SetInterpolationMode(Gdiplus::InterpolationModeHighQualityBicubic);
    m_pGraphics->SetSmoothingMode(Gdiplus::SmoothingModeHighQuality);
    m_pGraphics->SetPixelOffsetMode(Gdiplus::PixelOffsetModeHalf);
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

    HDC hdc = GetDC(g_hWnd);
    Gdiplus::Graphics screenGraphics(hdc);
    screenGraphics.DrawImage(m_pDoubleBufferBitmap, 0, 0, m_clientRect.right, m_clientRect.bottom);
    ReleaseDC(g_hWnd, hdc);
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

#include "99_Default/pch.h"
#include "GraphicsManager.h"
#include <chrono>
#include <fstream>
#include <windows.h>

// #region agent log helper
static std::wstring GetLogPath() {
	wchar_t exePath[MAX_PATH];
	GetModuleFileNameW(NULL, exePath, MAX_PATH);
	std::wstring path(exePath);
	size_t pos = path.find_last_of(L"\\");
	if (pos != std::wstring::npos) {
		path = path.substr(0, pos); // 실행 파일 디렉토리
	}
	// 실행 파일과 같은 디렉토리에 로그 파일 생성
	return path + L"\\debug.log";
}
// #endregion

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

	// 렌더링 품질 대신 성능 우선 설정 (런타임 전용)
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
	// #region agent log
	auto startTime = std::chrono::high_resolution_clock::now();
	// #endregion
	
    if (!m_pGraphics || !m_pDoubleBufferBitmap || !g_hWnd) return;

    HDC hdcScreen = GetDC(g_hWnd);
    
	// 성능 최적화: GDI+ DrawImage 대신 BitBlt 사용 (훨씬 빠름)
	HDC hdcMem = CreateCompatibleDC(hdcScreen);
	HBITMAP hBitmap = NULL;
	Gdiplus::Color color(0, 0, 0, 0);
	
	// GDI+ Bitmap에서 HBITMAP 추출
	if (m_pDoubleBufferBitmap->GetHBITMAP(color, &hBitmap) == Gdiplus::Ok && hBitmap) {
		HBITMAP hOldBitmap = (HBITMAP)SelectObject(hdcMem, hBitmap);
		
		// BitBlt로 직접 복사 (GDI+ DrawImage보다 훨씬 빠름)
		BitBlt(hdcScreen, 0, 0, m_clientRect.right, m_clientRect.bottom, hdcMem, 0, 0, SRCCOPY);
		
		// 정리
		SelectObject(hdcMem, hOldBitmap);
		DeleteObject(hBitmap);
		DeleteDC(hdcMem);
	} else {
		// GetHBITMAP 실패 시 기존 방식 사용 (fallback)
		Gdiplus::Graphics screenGraphics(hdcScreen);
		screenGraphics.SetInterpolationMode(Gdiplus::InterpolationModeNearestNeighbor);
		screenGraphics.SetSmoothingMode(Gdiplus::SmoothingModeHighSpeed);
		screenGraphics.SetPixelOffsetMode(Gdiplus::PixelOffsetModeNone);
		screenGraphics.DrawImage(m_pDoubleBufferBitmap, 0, 0, m_clientRect.right, m_clientRect.bottom);
		if (hdcMem) DeleteDC(hdcMem);
	}
	
    ReleaseDC(g_hWnd, hdcScreen);
	
	// #region agent log
	auto endTime = std::chrono::high_resolution_clock::now();
	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(endTime - startTime).count();
	// 로그 파일 I/O는 성능 테스트를 위해 임시로 비활성화
	// std::ofstream logFile(GetLogPath(), std::ios::app);
	// if (logFile.is_open()) {
	// 	logFile << "{\"runId\":\"perf1\",\"hypothesisId\":\"I\",\"location\":\"GraphicsManager.cpp:44\",\"message\":\"GraphicsManager::Render\",\"data\":{\"duration_us\":" << duration << "},\"timestamp\":" << std::chrono::duration_cast<std::chrono::milliseconds>(std::chrono::system_clock::now().time_since_epoch()).count() << "}\n";
	// 	logFile.close();
	// }
	// #endregion
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

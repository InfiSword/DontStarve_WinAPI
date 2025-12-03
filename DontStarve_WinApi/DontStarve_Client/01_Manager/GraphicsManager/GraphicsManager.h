// GraphicsManager.h
#pragma once

class GraphicsManager : public CSingleTon<GraphicsManager>
{
	friend class CSingleTon<GraphicsManager>;
public:
	GraphicsManager();
	~GraphicsManager();

	void Init();		
	// GDI+ 초기화 및 더블 버퍼링을 위한 설정
	void LateInit();      
	void Update(float deltaTime);      
	void LateUpdate();    
	void Render();        // 더블 버퍼의 내용을 화면에 표시
	void Release();       // GDI+ 리소스 해제

	Gdiplus::Graphics* GetGraphics(); 
	// 그리기 작업을 위한 Graphics 객체를 반환

private:
	Gdiplus::Graphics* m_pGraphics;
	Gdiplus::Bitmap* m_pDoubleBufferBitmap;
	RECT m_clientRect;
};

#pragma once

#ifdef _DEBUG
#include <string>

namespace Gdiplus {
  class Font;
  class SolidBrush;
  class StringFormat;
}
#endif

class InputManager;

class DontStarve_MainGame
{
private:
    // 초기화 여부
    bool m_bIsInitialized;

#ifdef _DEBUG
  bool m_showPerfOverlay;
  bool m_prevF1Down;
  bool m_prevF2Down;
  bool m_prevF3Down;
  std::wstring m_perfOverlayText;

  Gdiplus::Font* m_pPerfFont;
  Gdiplus::SolidBrush* m_pPerfBrush;
  Gdiplus::StringFormat* m_pPerfStringFormat;

  void UpdatePerformanceOverlayText();
  void RenderPerformanceOverlay();
#endif
    
public:
    DontStarve_MainGame();
    virtual ~DontStarve_MainGame();

public:
    // 초기화
    void Init();
    void LateInit();
    
    // 업데이트
    void Update();
    void LateUpdate();
    void Render();
    
    // 해제
    void Release();

    InputManager* GetInputManager() const;
}; 

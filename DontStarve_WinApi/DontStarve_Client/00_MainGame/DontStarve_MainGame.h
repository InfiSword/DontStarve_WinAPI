#pragma once

class InputManager;

class DontStarve_MainGame
{
private:
    // 초기화 여부
    bool m_bIsInitialized;
    
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

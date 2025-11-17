#pragma once

class DontStarve_MainGame
{
private:
    // 게임 초기화 상태 관리 변수
    bool m_bIsInitialized;
    
public:
    DontStarve_MainGame();
    virtual ~DontStarve_MainGame();

public:
    // 게임 초기화 관련 메서드
    void Init();
    void LateInit();
    
    // 게임 루프 관련 메서드
    void Update();
    void LateUpdate();
    void Render();
    
    // 리소스 해제
    void Release();

private:
    // 내부 초기화 메서드들
    void InitializeManagers();   
   
}; 
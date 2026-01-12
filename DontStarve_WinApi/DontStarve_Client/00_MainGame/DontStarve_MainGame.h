#pragma once

class DontStarve_MainGame
{
private:
    // ���� �ʱ�ȭ ���� ���� ����
    bool m_bIsInitialized;
    
public:
    DontStarve_MainGame();
    virtual ~DontStarve_MainGame();

public:
    // ���� �ʱ�ȭ ���� �޼���
    void Init();
    void LateInit();
    
    // ���� ���� ���� �޼���
    void Update();
    void LateUpdate();
    void Render();
    
    // ���ҽ� ����
    void Release();

private:
    // ���� �ʱ�ȭ �޼����
    void InitializeManagers();   
   
}; 

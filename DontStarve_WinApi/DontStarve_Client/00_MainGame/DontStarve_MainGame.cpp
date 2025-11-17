#include "../99_Default/pch.h"
#include "DontStarve_MainGame.h"

// Manager includes
#include "../01_Manager/TimeManager/TimeManager.h"
#include "../01_Manager/CameraManager/CameraManager.h"
#include "../01_Manager/InputManager/InputManager.h"
#include "../01_Manager/GraphicsManager/GraphicsManager.h"
#include "../01_Manager/RenderManager/RenderManager.h"
#include "../01_Manager/ObjectManager/ObjectManager.h"
#include "../01_Manager/ColliderManager/ColliderManager.h"
#include "../01_Manager/InventoryManager/InventoryManager.h"
#include "../01_Manager/UIManager/UIManager.h"
#include "../01_Manager/SceneManager/SceneManager.h"
#include "../01_Manager/ResourceManager/ResourceManager.h"

DontStarve_MainGame::DontStarve_MainGame()
    : m_bIsInitialized(false)
{
}

DontStarve_MainGame::~DontStarve_MainGame()
{
    Release();
}

void DontStarve_MainGame::Init()
{
    if (m_bIsInitialized)
        return;

    // 기본 시스템 매니저들만 초기화 (씬과 무관한 시스템)
    TimeManager::GetInstance()->Init();
    TimeManager::GetInstance()->SetFPS(20);
    
    GraphicsManager::GetInstance()->Init();
    RenderManager::GetInstance()->Init();
    ResourceManager::GetInstance()->Init(); // 리소스는 씬과 무관하게 유지
    
    // SceneManager 초기화 (첫 번째 씬 로드)
    SceneManager::GetInstance()->Init();
     
    m_bIsInitialized = true;
}

void DontStarve_MainGame::LateInit()
{
    if (!m_bIsInitialized)
        return;
        
}

void DontStarve_MainGame::Update()
{
    if (!m_bIsInitialized)
        return;

    // TimeManager 업데이트 (기본 시간 관리)
    TimeManager::GetInstance()->Update();
    float deltaTime = TimeManager::GetInstance()->GetDeltaTime(); // 이미 제한된 deltaTime
    
    // SceneManager를 통한 씬 업데이트
    SceneManager::GetInstance()->Update(deltaTime);
    
    // 프레임 제한 적용 (FPS 계산 포함)
    TimeManager::GetInstance()->UpdateFrameLimit();
}

void DontStarve_MainGame::LateUpdate()
{
    if (!m_bIsInitialized)
        return;

    // SceneManager를 통한 씬 LateUpdate
    SceneManager::GetInstance()->LateUpdate();
}

void DontStarve_MainGame::Render()
{
    if (!m_bIsInitialized)
        return;

    // Graphics 객체 얻기
    Gdiplus::Graphics* pGraphics = GraphicsManager::GetInstance()->GetGraphics();
    if (!pGraphics) return;

    // SceneManager를 통한 씬 렌더링
    SceneManager::GetInstance()->Render();

    // RenderManager에서 누적 렌더링 실행
    RenderManager::GetInstance()->Flush(pGraphics);

    // 백버퍼에 그려진 내용을 화면에 표시
    GraphicsManager::GetInstance()->Render();
}

void DontStarve_MainGame::Release()
{
    if (!m_bIsInitialized)
        return;

    // SceneManager 해제 (씬별 매니저들은 SceneManager에서 해제됨)
    SceneManager::GetInstance()->Release();
    
    // 기본 시스템 매니저들 해제
    ResourceManager::DestroyInstance();
    RenderManager::DestroyInstance();
    GraphicsManager::DestroyInstance();
    TimeManager::DestroyInstance();
    
    m_bIsInitialized = false;
}

void DontStarve_MainGame::InitializeManagers()
{
    // 이 함수는 더 이상 사용하지 않음
    // 매니저 초기화는 각 씬에서 담당
}


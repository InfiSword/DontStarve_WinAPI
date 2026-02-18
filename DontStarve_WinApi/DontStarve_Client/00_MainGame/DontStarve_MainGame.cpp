#include "99_Default/pch.h"
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
#include "../01_Manager/GameProgressManager/GameProgressManager.h"

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

    // 기본 시스템 매니저만 초기화 (렌더/윈도우 매니저 등)
    TimeManager::GetInstance()->Init();
    TimeManager::GetInstance()->SetFPS(30);
    
    GraphicsManager::GetInstance()->Init();
    RenderManager::GetInstance()->Init();
    ResourceManager::GetInstance()->Init(); // 리소스 매니저 초기화 (오브젝트 리소스 등록 포함)

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

    // TimeManager 업데이트 (기본 프레임 제한)
    TimeManager::GetInstance()->Update();
    float deltaTime = TimeManager::GetInstance()->GetDeltaTime(); // 실제 deltaTime
    
    // 입력 처리: 가장 먼저 업데이트 (버튼 반응 속도 개선)
    InputManager::GetInstance()->Update(deltaTime);
    
    // SceneManager 업데이트 (FPS 제한 적용)
    SceneManager::GetInstance()->Update(deltaTime);

    // FPS 제한 업데이트
    TimeManager::GetInstance()->UpdateFrameLimit();
}

void DontStarve_MainGame::LateUpdate()
{
    if (!m_bIsInitialized)
        return;

    // InputManager LateUpdate (입력 처리 후 정리 작업)
    InputManager::GetInstance()->LateUpdate();

    // SceneManager LateUpdate
    SceneManager::GetInstance()->LateUpdate();
}

void DontStarve_MainGame::Render()
{
    if (!m_bIsInitialized)
        return;

    // Graphics 그래픽스 컨텍스트 가져오기 (내부에서 검은색으로 Clear됨)
    Gdiplus::Graphics* pGraphics = GraphicsManager::GetInstance()->GetGraphics();
    if (!pGraphics) return;

    // SceneManager 렌더링 (씬이 RenderManager에 렌더링 명령 추가)
    SceneManager::GetInstance()->Render();

    // RenderManager 렌더링 커밋
    RenderManager::GetInstance()->Flush(pGraphics);

    // 그래픽스 컨텍스트 렌더링
    GraphicsManager::GetInstance()->Render();
}

void DontStarve_MainGame::Release()
{
    if (!m_bIsInitialized)
        return;

    // SceneManager 해제 (SceneManager 해제 시 모든 씬 해제)
    SceneManager::GetInstance()->Release();
    
    // 기본 시스템 매니저 해제
    ResourceManager::DestroyInstance();
    RenderManager::DestroyInstance();
    GraphicsManager::DestroyInstance();
    TimeManager::DestroyInstance();
    
    // 씬/게임 매니저 해제 (누수 검사 전 인스턴스 파괴)
    GameProgressManager::DestroyInstance();
    SceneManager::DestroyInstance();
    ObjectManager::DestroyInstance();
    CameraManager::DestroyInstance();
    InventoryManager::DestroyInstance();
    ColliderManager::DestroyInstance();
    InputManager::DestroyInstance();
    UIManager::DestroyInstance();
    
    m_bIsInitialized = false;
}

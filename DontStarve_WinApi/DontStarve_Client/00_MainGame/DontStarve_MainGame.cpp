#include "99_Default/pch.h"
#include "DontStarve_MainGame.h"
#include "../01_Manager/TimeManager/TimeManager.h"
#include "../01_Manager/CameraManager/CameraManager.h"
#include "../01_Manager/InputManager/InputManager.h"
#include "../01_Manager/GraphicsManager/GraphicsManager.h"
#include "../01_Manager/RenderManager/RenderManager.h"
#include "../01_Manager/ObjectManager/ObjectManager.h"
#include "../01_Manager/ColliderManager/ColliderManager.h"
#include "../01_Manager/InventoryManager/InventoryManager.h"
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
    InputManager::GetInstance()->Init();

    // 씬과 무관하게 한 번만 초기화하면 되는 매니저들
    ObjectManager::GetInstance()->Init();
    InventoryManager::GetInstance()->Init();
    ColliderManager::GetInstance()->Init();
    CameraManager::GetInstance()->Init();

    // SceneManager 초기화 (첫 번째 씬 로드)
    SceneManager::GetInstance()->Init();
    
    // 게임 진행도 매니저 초기화 (저장된 데이터 로드)
    GameProgressManager::GetInstance()->Init();
     
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

    // 게임 진행도 업데이트 (전체 시간 측정 등)
    GameProgressManager::GetInstance()->Update(deltaTime);

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

    // 1단계: 게임 로직 매니저 해제 (씬 안의 오브젝트 정리 포함)
    //        GameProgressManager는 씬 해제 전에 저장/파괴
    GameProgressManager::DestroyInstance();

    // 2단계: 씬 전체 파괴 (ObjectManager, CameraManager 등 씬 종속 매니저 포함)
    SceneManager::DestroyInstance();

    // 3단계: 씬과 무관한 독립 매니저 파괴
    InventoryManager::DestroyInstance();
    ColliderManager::DestroyInstance();
    ObjectManager::DestroyInstance();
    CameraManager::DestroyInstance();
    InputManager::DestroyInstance();

    // 4단계: 렌더/그래픽 매니저 파괴 (모든 Bitmap/Sprite 참조 해제 후)
    RenderManager::DestroyInstance();
    GraphicsManager::DestroyInstance();

    // 5단계: 리소스 매니저 파괴 (모든 오브젝트가 해제된 후 마지막)
    ResourceManager::DestroyInstance();

    // 6단계: 시간 매니저 파괴
    TimeManager::DestroyInstance();

    m_bIsInitialized = false;
}

InputManager* DontStarve_MainGame::GetInputManager() const
{
    return InputManager::GetInstance();
}

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
#include "../01_Manager/DataManager/DataManager.h"
#include "../01_Manager/GameProgressManager/GameProgressManager.h"

#ifdef _DEBUG
#include <iomanip>
#include <sstream>
#endif

DontStarve_MainGame::DontStarve_MainGame()
    : m_bIsInitialized(false)
#ifdef _DEBUG
    , m_showPerfOverlay(false)
    , m_prevF1Down(false)
    , m_prevF2Down(false)
    , m_pPerfFont(nullptr)
    , m_pPerfBrush(nullptr)
    , m_pPerfStringFormat(nullptr)
#endif
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
    DataManager::GetInstance()->Init();
    InputManager::GetInstance()->Init();

#ifdef _DEBUG
    m_pPerfFont = new Gdiplus::Font(L"Consolas", 14.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPoint);
    m_pPerfBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
    m_pPerfStringFormat = new Gdiplus::StringFormat();
    m_pPerfStringFormat->SetAlignment(Gdiplus::StringAlignmentNear);
    m_pPerfStringFormat->SetLineAlignment(Gdiplus::StringAlignmentNear);
#endif

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

#ifdef _DEBUG
    const bool isF1Down = InputManager::GetInstance()->IsKeyDown(VK_F1);
    if (isF1Down && !m_prevF1Down) {
        m_showPerfOverlay = !m_showPerfOverlay;
    }
    m_prevF1Down = isF1Down;

    const bool isF2Down = InputManager::GetInstance()->IsKeyDown(VK_F2);
    if (isF2Down && !m_prevF2Down) {
        RenderManager* pRM = RenderManager::GetInstance();
        pRM->SetOptimizationEnabled(!pRM->IsOptimizationEnabled());
    }
    m_prevF2Down = isF2Down;

    if (m_showPerfOverlay) {
        UpdatePerformanceOverlayText();
    }
#endif
    
    // SceneManager 업데이트 (FPS 제한 적용)
    SceneManager::GetInstance()->Update(deltaTime);

    // RenderManager 업데이트 (카메라 좌표 캐싱)
    RenderManager::GetInstance()->Update(deltaTime);

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
    if (!pGraphics) {
        return;
    }

    // SceneManager 렌더링 (씬이 RenderManager에 렌더링 명령 추가)
    // BeginFrame: 비최적화(일반 렌더) 모드에서 즉시 그리기에 사용할 Graphics 컨텍스트를 등록한다.
    RenderManager::GetInstance()->BeginFrame(pGraphics);
    SceneManager::GetInstance()->Render();

#ifdef _DEBUG
    if (m_showPerfOverlay) {
        RenderPerformanceOverlay();
    }
#endif

    // RenderManager 렌더링 커밋
    RenderManager::GetInstance()->Flush(pGraphics);

    // 그래픽스 컨텍스트 렌더링
    GraphicsManager::GetInstance()->Render();
}

void DontStarve_MainGame::Release()
{
    if (!m_bIsInitialized) {
        return;
    }

	SceneManager::DestroyInstance();
	ObjectManager::DestroyInstance();
	InputManager::DestroyInstance();
	ResourceManager::DestroyInstance();
	RenderManager::DestroyInstance();
	GraphicsManager::DestroyInstance();
	CameraManager::DestroyInstance();
	DataManager::DestroyInstance();
	GameProgressManager::DestroyInstance();
	InventoryManager::DestroyInstance();
	TimeManager::DestroyInstance();
	ColliderManager::DestroyInstance();

#ifdef _DEBUG
    Utils::SafeDelete(m_pPerfStringFormat);
    Utils::SafeDelete(m_pPerfBrush);
    Utils::SafeDelete(m_pPerfFont);
#endif

    m_bIsInitialized = false;
}

InputManager* DontStarve_MainGame::GetInputManager() const
{
    return InputManager::GetInstance();
}

#ifdef _DEBUG
void DontStarve_MainGame::UpdatePerformanceOverlayText()
{
    TimeManager* pTimeManager = TimeManager::GetInstance();
    if (!pTimeManager) return;

    const int targetFps = pTimeManager->GetFPS();
    const float currentFps = pTimeManager->GetCurrentFPS();
    const float frameMs = pTimeManager->GetDeltaTimeMs();
    const float targetFrameMs = pTimeManager->GetTargetFrameTimeMs();

    std::wostringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << L"[Performance Debug (F1)]\n";
    stream << L"FPS(Current) : " << currentFps << L"\n";
    stream << L"FPS(Target)  : ";
    if (targetFps > 0) {
        stream << targetFps;
    }
    else {
        stream << L"Unlimited";
    }
    stream << L"\n";
    stream << L"Frame Time   : " << frameMs << L" ms\n";

    if (targetFps > 0) {
        stream << L"Target Frame : " << targetFrameMs << L" ms\n";
    }

    stream << L"Delta Time   : " << pTimeManager->GetDeltaTime() << L" s\n";

    const bool isOptimized = RenderManager::GetInstance()->IsOptimizationEnabled();
    stream << L"Render Mode  : " << (isOptimized ? L"RenderQueue (F2)" : L"Direct (F2)");

    m_perfOverlayText = stream.str();
}

void DontStarve_MainGame::RenderPerformanceOverlay()
{
    if (m_perfOverlayText.empty() || !m_pPerfFont || !m_pPerfBrush || !m_pPerfStringFormat) return;

    const float panelX = 10.0f;
    const float panelY = 10.0f;
    const float panelW = 380.0f;
    const float panelH = 150.0f;
    const float padding = 10.0f;

    const Gdiplus::RectF backgroundRect(panelX, panelY, panelW, panelH);
    const Gdiplus::RectF textRect(panelX + padding, panelY + padding, panelW - (padding * 2.0f), panelH - (padding * 2.0f));

    // UI 최상위 레이어에 렌더링해 씬 UI에 가려지지 않도록 고정한다.
    RenderManager::GetInstance()->AddFillRectangleCommand(backgroundRect, Gdiplus::Color(220, 255, 255, 255), LAYER_UI_FOREGROUND, 9999.0f);
    RenderManager::GetInstance()->AddDrawRectCommand(backgroundRect, Gdiplus::Color(255, 30, 30, 30), 1.5f, LAYER_UI_FOREGROUND, 10000.0f);
    RenderManager::GetInstance()->AddTextCommand(&m_perfOverlayText, m_pPerfFont, m_pPerfBrush, m_pPerfStringFormat, textRect, LAYER_UI_FOREGROUND, 10001.0f);
}
#endif


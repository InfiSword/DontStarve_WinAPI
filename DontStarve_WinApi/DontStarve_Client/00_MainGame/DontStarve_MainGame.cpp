#include "99_Default/pch.h"
#include "DontStarve_MainGame.h"
#include "../99_Default/ClientOptimatzationOption.h"
#include "../01_Manager/SoundManager/SoundManager.h"
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
    , m_prevF3Down(false)
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
    TimeManager::GetInstance()->SetFPS(30); // 목표 프레임을 30으로 고정
    
    GraphicsManager::GetInstance()->Init();
    RenderManager::GetInstance()->Init();
    ResourceManager::GetInstance()->Init(); // 리소스 매니저 초기화 (오브젝트 리소스 등록 포함)
    DataManager::GetInstance()->Init();
    InputManager::GetInstance()->Init();
    SoundManager::GetInstance()->Init();

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
    // F1: 성능 오버레이 토글
    const bool isF1Down = InputManager::GetInstance()->IsKeyDown(VK_F1);
    if (isF1Down && !m_prevF1Down) {
        m_showPerfOverlay = !m_showPerfOverlay;
    }
    m_prevF1Down = isF1Down;

    // F2: 최적화 모드 토글
    const bool isF2Down = InputManager::GetInstance()->IsKeyDown(VK_F2);
    if (isF2Down && !m_prevF2Down) {
        ToggleOptimizationMode();
    }
    m_prevF2Down = isF2Down;

    // F3: 공간 분할 토글
    const bool isF3Down = InputManager::GetInstance()->IsKeyDown(VK_F3);
    if (isF3Down && !m_prevF3Down) {
        g_bEnableSpatialPartitioning = !g_bEnableSpatialPartitioning;
    }
    m_prevF3Down = isF3Down;

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

    // 모드와 무관하게 프레임 시작 시 백버퍼를 1회 초기화한다.
    GraphicsManager::GetInstance()->BeginFrame();

    // Graphics 그래픽스 컨텍스트 가져오기
    Gdiplus::Graphics* pGraphics = GraphicsManager::GetInstance()->GetGraphics();
    if (!pGraphics) {
        return;
    }

    // SceneManager 렌더링
#ifdef _DEBUG
    if (g_bEnableOptimizationMode) {
#endif
        // 최적화 모드: 커맨드 큐 기반 렌더
        RenderManager::GetInstance()->BeginFrame(CameraManager::GetInstance()->GetCameraPos());
        SceneManager::GetInstance()->Render();
#ifdef _DEBUG
    }
    else {
        // 비최적화 모드: 즉시 렌더 경로 사용
        RenderManager::GetInstance()->Clear();
        SceneManager::GetInstance()->Render();
    }
#endif

#ifdef _DEBUG
    if (m_showPerfOverlay) {
        RenderPerformanceOverlay();
    }
#endif

    RenderManager::GetInstance()->Flush(pGraphics);
    GraphicsManager::GetInstance()->Render();
}

void DontStarve_MainGame::Release()
{
    if (!m_bIsInitialized) {
        return;
    }

	SceneManager::DestroyInstance();
	SoundManager::DestroyInstance();
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
    const float avgCullVisibleGameObjectsMs = CameraManager::GetInstance()->GetAvgCullVisibleGameObjectsMs();
    const float avgRenderVisibleGameObjectsMs = CameraManager::GetInstance()->GetAvgRenderVisibleGameObjectsMs();
    const float avgRenderVisibleTilesMs = CameraManager::GetInstance()->GetAvgRenderVisibleTilesMs();

    const int renderedObjectCount = RenderManager::GetInstance()->GetRenderedObjectCount();
    const int renderedEntityCount = RenderManager::GetInstance()->GetRenderedEntityCount();

    auto formatPerfValue = [](float ms) -> std::wstring {
        std::wostringstream value;
        value << std::fixed;
        if (ms > 0.0f && ms < 0.01f) {
            value << std::setprecision(1) << (ms * 1000.0f) << L"us";
        }
        else {
            value << std::setprecision(3) << ms << L"ms";
        }
        return value.str();
    };

    std::wostringstream stream;
    stream << std::fixed << std::setprecision(2);
    stream << L"[성능 디버그 - F1]\n";
    stream << L"===================================\n";
    stream << L"모드: " << (g_bEnableOptimizationMode ? L"최적화 ON" : L"비최적화") << L" [F2]\n";
    stream << L"공간 분할: " << (g_bEnableSpatialPartitioning ? L"ON" : L"OFF") << L" [F3]\n";
    stream << L"===================================\n";
    stream << L"FPS(현재) : " << currentFps << L"\n";
    stream << L"FPS(목표) : ";
    if (targetFps > 0) stream << targetFps;
    else stream << L"무제한";
    stream << L"\n[EMA 시간(ms)]\n";
    stream << L"CullVisibleGameObjects          : " << formatPerfValue(avgCullVisibleGameObjectsMs) << L"\n";
    stream << L"RenderVisibleGameObjects        : " << formatPerfValue(avgRenderVisibleGameObjectsMs) << L"\n";
    stream << L"RenderVisibleTiles              : " << formatPerfValue(avgRenderVisibleTilesMs) << L"\n";
    stream << L"\n[통계]\n";
    stream << L"렌더링 오브젝트 수              : " << renderedObjectCount << L"\n";
    stream << L"렌더링 엔티티 수                : " << renderedEntityCount << L"\n";

    m_perfOverlayText = stream.str();
}

void DontStarve_MainGame::RenderPerformanceOverlay()
{
    if (m_perfOverlayText.empty() || !m_pPerfFont || !m_pPerfBrush || !m_pPerfStringFormat) return;

    const float panelX = 10.0f;
    const float panelY = 10.0f;
    const float panelW = 700.0f;
    const float padding = 12.0f;

    int lineCount = 1;
    for (wchar_t ch : m_perfOverlayText) {
        if (ch == L'\n') ++lineCount;
    }

    const float lineHeight = 22.0f;
    float panelH = (padding * 2.0f) + (static_cast<float>(lineCount) * lineHeight) + 12.0f;
    if (panelH < 280.0f) panelH = 280.0f;
    if (panelH > 680.0f) panelH = 680.0f;

    const Gdiplus::RectF backgroundRect(panelX, panelY, panelW, panelH);
    const Gdiplus::RectF textRect(panelX + padding, panelY + padding, panelW - (padding * 2.0f), panelH - (padding * 2.0f));

    RenderManager::GetInstance()->AddFillRectangleCommand(backgroundRect, Gdiplus::Color(220, 255, 255, 255), LAYER_UI_FOREGROUND, 9999.0f);
    RenderManager::GetInstance()->AddDrawRectCommand(backgroundRect, Gdiplus::Color(255, 30, 30, 30), 1.5f, LAYER_UI_FOREGROUND, 10000.0f);
    RenderManager::GetInstance()->AddTextCommand(&m_perfOverlayText, m_pPerfFont, m_pPerfBrush, m_pPerfStringFormat, textRect, LAYER_UI_FOREGROUND, 10001.0f);
}
#endif


#include "99_Default/pch.h"
#include "TitleScene.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"
#include "SceneManager.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"

TitleScene::TitleScene()
	: m_backgroundImage(nullptr),
	m_logoImage(nullptr),
	m_startButton(nullptr),
	m_exitButton(nullptr)
{
}

TitleScene::~TitleScene()
{
	Release();
}

void TitleScene::Init()
{
	// TitleScene에 필요한 매니저들 초기화
	InitializeManagers();
	
	// UI 생성
	CreateUI();
}

void TitleScene::CreateUI()
{
	// 화면 크기 계산
	float screenWidth = WINCX;
	float screenHeight = WINCY;

	// 배경 이미지 생성
	m_backgroundImage = new UIImage(
		static_cast<GameObjectID>(GOID_MAIN_BG),
		screenWidth / 2.0f,
		screenHeight / 2.0f,
		screenWidth,
		screenHeight,
		LAYER_UI_BACKGROUND,
		L"../Resource/UI/motd_fallbacks_box6.png",
		0.f
	);
	UIManager::GetInstance()->AddUIImage(m_backgroundImage);

	// 로고 이미지 생성 (화면 상단에 위치)
	m_logoImage = new UIImage(
		static_cast<GameObjectID>(GOID_GAME_LOGO),
		screenWidth / 2.0f,
		200.0f,
		400.0f,
		200.0f,
		LAYER_UI_FOREGROUND,
		L"../Resource/UI/logo.png",
		0.f
	);
	UIManager::GetInstance()->AddUIImage(m_logoImage);

	// 게임시작 버튼 생성
	m_startButton = new UIButton(
		static_cast<GameObjectID>(GOID_BUTTON1),
		screenWidth / 2.0f,
		screenHeight / 2.0f + 100.0f,
		200.0f,
		60.0f,
		L"../Resource/UI/frontscreen.png",
		L"../Resource/UI/HighLight_frontscreen.png",
		L"게임시작"
	);
	
	// 게임시작 버튼 콜백 설정
	m_startButton->SetOnClickCallback([this]() {
		OnStartButtonClicked();
	});
	UIManager::GetInstance()->AddUIButton(m_startButton);

	// 종료 버튼 생성
	m_exitButton = new UIButton(
		static_cast<GameObjectID>(GOID_ENDBUTTON1),
		screenWidth / 2.0f,
		screenHeight / 2.0f + 200.0f,
		200.0f,
		60.0f,
		L"../Resource/UI/frontscreen.png",
		L"../Resource/UI/HighLight_frontscreen.png",
		L"종료"
	);
	
	// 종료 버튼 콜백 설정
	m_exitButton->SetOnClickCallback([this]() {
		OnExitButtonClicked();
	});
	UIManager::GetInstance()->AddUIButton(m_exitButton);
}

void TitleScene::Update(float deltaTime)
{
	// 매니저들 업데이트
	UpdateManagers(deltaTime);
}

void TitleScene::LateUpdate()
{
	// 매니저들 LateUpdate
	LateUpdateManagers();
}

void TitleScene::Render()
{
	// 매니저들 렌더링
	RenderManagers();
}

void TitleScene::Release()
{
	// UI 객체들은 UIManager에서 해제되므로, 포인터만 nullptr로 설정
	// UIManager::Release()가 호출되면 자동으로 delete됨
	m_backgroundImage = nullptr;
	m_logoImage = nullptr;
	m_startButton = nullptr;
	m_exitButton = nullptr;
	
	// TitleScene에서 사용한 매니저들 해제
	ReleaseAllManagers();
}

void TitleScene::OnStartButtonClicked()
{
	OutputDebugStringW(L"TitleScene: Start button clicked!\n");
	// m_startButton->SetDisabled(false);
	// SceneManager를 통해 캐릭터 선택 씬으로 전환
	SceneManager::GetInstance()->LoadCharacterSelectScene();
}

void TitleScene::OnExitButtonClicked()
{
	OutputDebugStringW(L"TitleScene: Exit button clicked!\n");
	
	// 프로그램 종료
	PostQuitMessage(0);
}

void TitleScene::UpdateManagers(float deltaTime)
{
	// TitleScene에서 UIManager와 InputManager 업데이트
	UIManager::GetInstance()->Update(deltaTime);
	InputManager::GetInstance()->Update(deltaTime);
}

void TitleScene::LateUpdateManagers()
{
	// TitleScene에서 UIManager와 InputManager LateUpdate
	UIManager::GetInstance()->LateUpdate();
	InputManager::GetInstance()->LateUpdate();
}

void TitleScene::RenderManagers()
{
	// TitleScene에서 UIManager와 InputManager 렌더링
	UIManager::GetInstance()->Render();
	InputManager::GetInstance()->Render();
}

void TitleScene::ReleaseManagers()
{
	// TitleScene에서 UIManager 해제
	UIManager::GetInstance()->Release();
}

void TitleScene::InitializeManagers()
{
	OutputDebugStringW(L"TitleScene: 매니저 초기화 시작\n");
	
	// TitleScene에서 UIManager와 InputManager를 초기화
	UIManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	
	OutputDebugStringW(L"TitleScene: 매니저 초기화 완료\n");
}

void TitleScene::ReleaseAllManagers()
{
	OutputDebugStringW(L"TitleScene: 매니저 해제 시작\n");
	
	// TitleScene에서 사용한 매니저들 해제
	InputManager::GetInstance()->Release();
	UIManager::GetInstance()->Release();
	
	OutputDebugStringW(L"TitleScene: 매니저 해제 완료\n");
}

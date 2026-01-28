#include "99_Default/pch.h"
#include "TitleScene.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"
#include "../../02_GameObject/UI/UIText.h"
#include "SceneManager.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"

TitleScene::TitleScene()
{
}

TitleScene::~TitleScene()
{
	Release();
}

void TitleScene::Init()
{
	// TitleScene에 필요한 매니저들 초기화
	UIManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	
	// UI 생성
	UIManager* uiManager = UIManager::GetInstance();

	// 배경 이미지 생성 (전체 화면)
	UIImage* backgroundImage = new UIImage(
		static_cast<GameObjectID>(GOID_MAIN_BG),
		static_cast<float>(WINCX),
		static_cast<float>(WINCY),
		LAYER_UI_BACKGROUND,
		L"../Resource/UI/motd_fallbacks_box6.png",
		0.f,
		0.0f, 0.0f,  // anchorMin
		1.0f, 1.0f,  // anchorMax
		0.0f, 0.0f    // anchoredPosition
	);
	uiManager->AddUIImage(backgroundImage);

	// 로고 이미지 생성 (화면 상단 중앙)
	UIImage* logoImage = new UIImage(
		static_cast<GameObjectID>(GOID_GAME_LOGO),
		400.0f,
		200.0f,
		LAYER_UI_FOREGROUND,
		L"../Resource/UI/logo.png",
		0.f,
		0.5f, 1.0f,  // anchorMin (상단 중앙)
		0.5f, 1.0f,  // anchorMax (상단 중앙)
		0.0f, -200.0f // anchoredPosition (상단에서 아래로 200px)
	);
	uiManager->AddUIImage(logoImage);

	// 게임시작 버튼 생성 (화면 중앙 기준 아래로 100px)
	UIButton* startButton = new UIButton(
		static_cast<GameObjectID>(GOID_BUTTON1),
		200.0f,
		60.0f,
		L"../Resource/UI/frontscreen.png",
		L"../Resource/UI/HighLight_frontscreen.png",
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 100.0f // anchoredPosition (중앙에서 아래로 100px)
	);

	// 게임시작 버튼 콜백 설정
	startButton->SetOnClickCallback([this]() {
		OnStartButtonClicked();
		});
	uiManager->AddUIButton(startButton);

	// 게임시작 버튼 텍스트 생성 (버튼과 동일한 anchor)
	UIText* startButtonText = new UIText(
		static_cast<GameObjectID>(GOID_BUTTON1_TEXT),
		200.0f,
		60.0f,
		L"게임시작",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.1f,
		L"맑은 고딕",
		16.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 100.0f // anchoredPosition (중앙에서 아래로 100px)
	);
	uiManager->AddUIText(startButtonText);

	// 종료 버튼 생성 (화면 중앙 기준 아래로 200px)
	UIButton* exitButton = new UIButton(
		static_cast<GameObjectID>(GOID_ENDBUTTON1),
		200.0f,
		60.0f,
		L"../Resource/UI/frontscreen.png",
		L"../Resource/UI/HighLight_frontscreen.png",
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 200.0f // anchoredPosition (중앙에서 아래로 200px)
	);

	// 종료 버튼 콜백 설정
	exitButton->SetOnClickCallback([this]() {
		OnExitButtonClicked();
		});
	uiManager->AddUIButton(exitButton);

	// 종료 버튼 텍스트 생성 (버튼과 동일한 anchor)
	UIText* exitButtonText = new UIText(
		static_cast<GameObjectID>(GOID_ENDBUTTON1_TEXT),
		200.0f,
		60.0f,
		L"종료",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.1f,
		L"맑은 고딕",
		16.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 200.0f // anchoredPosition (중앙에서 아래로 200px)
	);
	uiManager->AddUIText(exitButtonText);
}

void TitleScene::Update(float deltaTime)
{
	// TitleScene에서 UIManager와 InputManager 업데이트
	UIManager::GetInstance()->Update(deltaTime);
	InputManager::GetInstance()->Update(deltaTime);
}

void TitleScene::LateUpdate()
{
	UIManager::GetInstance()->LateUpdate();
	InputManager::GetInstance()->LateUpdate();
}

void TitleScene::Render()
{
	// 매니저들 렌더링
	UIManager::GetInstance()->Render();
	InputManager::GetInstance()->Render();
}

void TitleScene::Release()
{
	// UI 객체들은 UIManager에서 해제되므로 별도 처리 불필요
	// TitleScene에서 사용한 매니저들 해제
	UIManager::GetInstance()->Release();
	InputManager::GetInstance()->Release();
}

void TitleScene::OnStartButtonClicked()
{
	OutputDebugStringW(L"TitleScene: Start button clicked!\n");
	// SceneManager를 통해 캐릭터 선택 씬으로 전환 요청
	SceneManager::GetInstance()->RequestLoadCharacterSelectScene();
}

void TitleScene::OnExitButtonClicked()
{
	OutputDebugStringW(L"TitleScene: Exit button clicked!\n");
	
	// 프로그램 종료
	PostQuitMessage(0);
}

#include "99_Default/pch.h"
#include "TitleScene.h"
#include "SceneManager.h"
#include "../InputManager/InputManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../GameProgressManager/GameProgressManager.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"
#include "../../02_GameObject/UI/UIText.h"

TitleScene::TitleScene()
	: m_resetMessageText(nullptr)
{
}

TitleScene::~TitleScene()
{
	Release();
}

void TitleScene::Init(const MapData* mapData)
{
	// TitleScene에 필요한 매니저들 초기화
	ObjectManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	
	// UI 생성
	ObjectManager* objectManager = ObjectManager::GetInstance();
	ResourceManager* resourceManager = ResourceManager::GetInstance();

	// 배경 이미지 생성 (전체 화면)
	UIImage* backgroundImage = new UIImage(
		static_cast<GameObjectID>(GOID_UI_IMAGE),
		static_cast<float>(WINCX),
		static_cast<float>(WINCY),
		LAYER_UI_BACKGROUND,
		L"Resource/UI/motd_fallbacks_box6.png",
		0.f,
		0.0f, 0.0f,  // anchorMin
		1.0f, 1.0f,  // anchorMax
		0.0f, 0.0f    // anchoredPosition
	);
	objectManager->AddGameObject(backgroundImage);

	// 로고 이미지 생성 (화면 상단 중앙)
	UIImage* logoImage = new UIImage(
		static_cast<GameObjectID>(GOID_UI_IMAGE),
		400.0f,
		200.0f,
		LAYER_UI_FOREGROUND,
		L"Resource/UI/logo.png",
		0.f,
		0.5f, 0.0f,  // anchorMin (상단 중앙)
		0.5f, 0.0f,  // anchorMax (상단 중앙)
		0.0f, 200.0f // anchoredPosition (상단에서 아래로 200px)
	);
	objectManager->AddGameObject(logoImage);

	// 게임시작 버튼 생성 (화면 중앙 기준 아래로 100px)
	std::shared_ptr<Sprite> startNormalSprite = resourceManager->LoadSprite(L"Resource/UI/frontscreen.png");
	std::shared_ptr<Sprite> startHoverSprite = resourceManager->LoadSprite(L"Resource/UI/HighLight_frontscreen.png");
	UIButton* startButton = new UIButton(
		static_cast<GameObjectID>(GOID_UI_BUTTON),
		200.0f,
		60.0f,
		startNormalSprite,
		startHoverSprite,
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 100.0f // anchoredPosition (중앙에서 아래로 100px)
	);

	// Hover 시 밝게 빛나는 효과 설정
	startButton->SetNormalColor(Gdiplus::Color(255, 255, 255, 255));  // 완전 흰색 (밝게)
	startButton->SetHoverColor(Gdiplus::Color(255, 220, 220, 220)); // 기본 (약간 어둡게)

	// 게임시작 버튼 콜백 설정
	startButton->SetOnClickCallback([this]() {
		OnStartButtonClicked();
		});
	objectManager->AddGameObject(startButton);

	// 게임시작 버튼 텍스트 생성 (버튼과 동일한 anchor)
	UIText* startButtonText = new UIText(
		static_cast<GameObjectID>(GOID_UI_TEXT),
		200.0f,
		60.0f,
		L"게임시작",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.1f,
		L"맑은 고딕",
		16.0f, Gdiplus::FontStyleRegular,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 100.0f // anchoredPosition (중앙에서 아래로 100px)
	);
	objectManager->AddGameObject(startButtonText);

	// 종료 버튼 생성 (화면 중앙 기준 아래로 200px)
	std::shared_ptr<Sprite> exitNormalSprite = resourceManager->LoadSprite(L"Resource/UI/frontscreen.png");
	std::shared_ptr<Sprite> exitHoverSprite = resourceManager->LoadSprite(L"Resource/UI/HighLight_frontscreen.png");
	UIButton* exitButton = new UIButton(
		static_cast<GameObjectID>(GOID_UI_BUTTON),
		200.0f,
		60.0f,
		exitNormalSprite,
		exitHoverSprite,
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 200.0f // anchoredPosition (중앙에서 아래로 200px)
	);

	// Hover 시 밝게 빛나는 효과 설정
	exitButton->SetNormalColor(Gdiplus::Color(255, 255, 255, 255));  // 완전 흰색 (밝게)
	exitButton->SetHoverColor(Gdiplus::Color(255, 220, 220, 220)); // 기본 (약간 어둡게)

	// 종료 버튼 콜백 설정
	exitButton->SetOnClickCallback([this]() {
		OnExitButtonClicked();
		});
	objectManager->AddGameObject(exitButton);

	// 종료 버튼 텍스트 생성 (버튼과 동일한 anchor)
	UIText* exitButtonText = new UIText(
		static_cast<GameObjectID>(GOID_UI_TEXT),
		200.0f,
		60.0f,
		L"종료",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.1f,
		L"맑은 고딕",
		16.0f, Gdiplus::FontStyleRegular,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 200.0f // anchoredPosition (중앙에서 아래로 200px)
	);
	objectManager->AddGameObject(exitButtonText);

	// --- 진행상황 초기화 버튼 추가 (오른쪽 위) ---
	UIButton* resetProgressButton = new UIButton(
		static_cast<GameObjectID>(GOID_UI_BUTTON),
		180.0f,
		40.0f,
		startNormalSprite, // 재사용
		startHoverSprite,  // 재사용
		1.0f, 0.0f,  // anchorMin (우측 상단)
		1.0f, 0.0f,  // anchorMax (우측 상단)
		-100.0f, 50.0f // anchoredPosition (우측에서 100px 좌측, 상단에서 50px 아래)
	);
	resetProgressButton->SetOnClickCallback([this]() {
		OnResetButtonClicked();
	});
	objectManager->AddGameObject(resetProgressButton);

	UIText* resetBtnText = new UIText(
		static_cast<GameObjectID>(GOID_UI_TEXT),
		180.0f,
		40.0f,
		L"진행상황 초기화",
		Gdiplus::Color::DarkRed,
		LAYER_UI_FOREGROUND,
		0.1f,
		L"맑은 고딕",
		12.0f, Gdiplus::FontStyleRegular,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		1.0f, 0.0f,
		1.0f, 0.0f,
		-100.0f, 50.0f
	);
	objectManager->AddGameObject(resetBtnText);

	// 리셋 완료 메시지 텍스트 (초기에는 비활성)
	m_resetMessageText = new UIText(
		static_cast<GameObjectID>(GOID_UI_TEXT),
		400.0f,
		60.0f,
		L"초기화 되었습니다!",
		Gdiplus::Color::LimeGreen,
		LAYER_UI_FOREGROUND,
		0.0f, // 가장 앞에 렌더링 (sortKey가 작을수록 앞)
		L"맑은 고딕",
		24.0f, Gdiplus::FontStyleBold,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.5f,  // anchorMin (중앙)
		0.5f, 0.5f,  // anchorMax (중앙)
		0.0f, 0.0f   // anchoredPosition (중앙)
	);
	m_resetMessageText->SetActive(false);
	objectManager->AddGameObject(m_resetMessageText);
}

void TitleScene::Update(float deltaTime)
{
	// TitleScene에서 ObjectManager 업데이트
	// InputManager는 메인 루프에서 가장 먼저 업데이트됨 (반응 속도 개선)
	ObjectManager::GetInstance()->Update(deltaTime);
}

void TitleScene::LateUpdate()
{
	ObjectManager::GetInstance()->LateUpdate();
	// InputManager::LateUpdate는 메인 루프에서 처리됨
}

void TitleScene::Render()
{
	// 매니저들 렌더링
	ObjectManager::GetInstance()->Render();
	InputManager::GetInstance()->Render();
}

void TitleScene::Release()
{
	// TitleScene에서 사용한 매니저/포인터 정리 (소멸자에서 호출)
	ObjectManager::GetInstance()->Release();
	InputManager::GetInstance()->Release();
	m_resetMessageText = nullptr;
}

void TitleScene::OnStartButtonClicked()
{
	SceneManager::GetInstance()->LoadCharacterSelectScene();
}

void TitleScene::OnExitButtonClicked()
{
	PostQuitMessage(0);
}

void TitleScene::OnResetButtonClicked()
{
	GameProgressManager::GetInstance()->ResetAllProgress();
	if (m_resetMessageText) {
		m_resetMessageText->SetActive(true);
		
		// 2초 뒤에 메시지 숨기기 (코루틴 활용)
		m_resetMessageText->StopAllCoroutines();
		float timer = 0.0f;
		m_resetMessageText->StartCoroutine([this, timer](float dt) mutable -> bool {
			timer += dt;
			if (timer >= 2.0f) {
				m_resetMessageText->SetActive(false);
				return false;
			}
			return true;
		});
	}
}

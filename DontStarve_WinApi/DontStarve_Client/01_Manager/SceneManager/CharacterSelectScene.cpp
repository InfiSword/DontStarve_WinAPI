#include "../../99_Default/pch.h"
#include "CharacterSelectScene.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"
#include "SceneManager.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"
#include "../RenderManager/RenderManager.h"

CharacterSelectScene::CharacterSelectScene()
	: m_descriptionFont(nullptr), m_descriptionBrush(nullptr),
	  m_currentState(CharacterSelectionState::BROWSING), m_selectedCharacterIndex(-1),
	  m_isLockedCharacterSelected(false), m_isSelectButtonDisabled(false)
{
}

CharacterSelectScene::~CharacterSelectScene()
{
	Release();
}

void CharacterSelectScene::Init()
{
	// CharacterSelectScene에 필요한 매니저들 초기화
	InitializeManagers();
	
	// 캐릭터 목록 초기화
	InitializeCharacters();
	OutputDebugStringW((L"CharacterSelectScene: 캐릭터 목록 초기화 완료 - " + std::to_wstring(m_characterList.size()) + L"개 캐릭터\n").c_str());
	
	// UI 생성
	CreateUI();
	OutputDebugStringW(L"CharacterSelectScene: UI 생성 완료\n");
	
	// 텍스트 렌더링 초기화
	InitializeTextRendering();
	
	// 초기 상태 설정
	m_currentState = CharacterSelectionState::BROWSING;
	m_selectedCharacterIndex = -1;
	m_isLockedCharacterSelected = false;
	m_isSelectButtonDisabled = true;
	
	OutputDebugStringW(L"CharacterSelectScene: 초기화 완료\n");
}


void CharacterSelectScene::InitializeCharacters()
{
	// 캐릭터 목록 초기화
	m_characterList.clear();
	
	float screenWidth = static_cast<float>(WINCX);
	float screenHeight = static_cast<float>(WINCY);
	
	// 캐릭터들의 배치 위치
	float startX = 150.0f;
	float spacing = 200.0f;
	float characterY = 300.0f;
	
	// Wilson 캐릭터 추가 (기본 해금)
	m_characterList.emplace_back(
		L"Wilson",
		L"../Resource/UI/wilson.png",
		L"../Resource/UI/Willson_Character.png",
		L"기본 캐릭터입니다.\n모든 상황에서 안정적으로 생존할 수 있는 균형잡힌 캐릭터입니다.",
		startX,
		characterY,
		GOID_PLAYER_WILSON,
		true  // Wilson은 항상 해금됨
	);
	
	// Willow 캐릭터 추가 (불씨와 나무 클리어 시 해금)
	m_characterList.emplace_back(
		L"Willow",
		L"../Resource/Objects/Player/Willson/willow_portrait.png",
		L"../Resource/Objects/Player/Willson/willow_character.png",
		L"불의 마법사입니다.\n불을 다루는 능력이 뛰어납니다.\n\n해금 조건: 불씨와 나무 클리어",
		startX + spacing,
		characterY,
		GOID_PLAYER_WILLOW,
		m_gameProgress.IsCharacterUnlocked(GOID_PLAYER_WILLOW)
	);
	
	// Wolfgang 캐릭터 추가 (거미여왕과 나무 클리어 시 해금)
	m_characterList.emplace_back(
		L"Wolfgang",
		L"../Resource/Objects/Player/Willson/wolfgang_portrait.png",
		L"../Resource/Objects/Player/Willson/wolfgang_character.png",
		L"강한 캐릭터입니다.\n배고픔이 낮을수록 더 강해집니다.\n\n해금 조건: 거미여왕과 나무 클리어",
		startX + spacing * 2,
		characterY,
		GOID_PLAYER_WOLFGANG,
		m_gameProgress.IsCharacterUnlocked(GOID_PLAYER_WOLFGANG)
	);
}

void CharacterSelectScene::CreateUI()
{
	// 화면 크기
	float screenWidth = static_cast<float>(WINCX);
	float screenHeight = static_cast<float>(WINCY);

	// 배경 이미지 생성
	UIImage* backgroundImage = new UIImage(
		static_cast<GameObjectID>(GOID_MAIN_BG),
		screenWidth / 2.0f,
		screenHeight / 2.0f,
		screenWidth,
		screenHeight,
		LAYER_UI_BACKGROUND,
		L"../Resource/UI/BG.png",
		0.f
	);
	UIManager::GetInstance()->AddUIImage(backgroundImage);

	// 뒤로가기 버튼 생성 (화면 하단)
	UIButton* backButton = new UIButton(
		static_cast<GameObjectID>(GOID_BACK_BUTTON),
		100.0f,  
		screenHeight / 2.0f + 300.f,
		80.0f,
		100.0f,
		L"../Resource/UI/Button.png",
		L"../Resource/UI/Button_Click.png",
		L"타이틀 화면"
	);
	
	backButton->SetOnClickCallback([this]() {
		OnBackButtonClicked();
	});
	UIManager::GetInstance()->AddUIButton(backButton);

	// 선택된 캐릭터 포트레이트 (오른쪽에 배치) - 초기에는 숨김
	UIImage* selectedPortrait = new UIImage(
		static_cast<GameObjectID>(GOID_PLAYER_PORTRAIT),
		screenWidth / 2.0f + 300.0f,
		screenHeight / 2.0f - 150.0f,
		350.0f,
		500.0f,
		LAYER_UI_FOREGROUND,
		L"../Resource/UI/wilson.png",
		1.0f
	);
	selectedPortrait->SetActive(false);  // 초기에는 비활성화
	UIManager::GetInstance()->AddUIImage(selectedPortrait);

	// 캐릭터 정보창 (UI4.png) - 초기에는 숨김
	UIImage* characterInfoPanel = new UIImage(
		static_cast<GameObjectID>(GOID_PLAYER_INFO),
		screenWidth / 2.0f + 300.0f,
		screenHeight / 2.0f + 200.f,
		500.0f,
		200.0f,
		LAYER_UI_FOREGROUND,
		L"../Resource/UI/UI4.png",
		1.0f
	);
	characterInfoPanel->SetActive(false);  // 초기에는 비활성화
	UIManager::GetInstance()->AddUIImage(characterInfoPanel);

	// 선택 버튼 - 초기에는 숨김
	UIButton* selectButton = new UIButton(
		static_cast<GameObjectID>(GOID_SELECT_BUTTON),
		screenWidth / 2.0f + 200.f,
		screenHeight / 2.0f + 350.0f,
		120.0f,
		50.0f,
		L"../Resource/UI/Select_Bar.png",
		L"../Resource/UI/Select_Bar.png",
		L"선택"
	);
	
	selectButton->SetOnClickCallback([this]() {
		this->OnSelectButtonClicked();
	});
	selectButton->SetActive(false);  // 초기에는 비활성화
	UIManager::GetInstance()->AddUIButton(selectButton);

	// 취소 버튼 - 초기에는 숨김
	UIButton* cancelButton = new UIButton(
		static_cast<GameObjectID>(GOID_CANCEL_SELECTION),
		screenWidth / 2.0f + 450.f,
		screenHeight / 2.0f + 350.0f,
		120.0f,
		50.0f,
		L"../Resource/UI/Select_Bar.png",
		L"../Resource/UI/Select_Bar.png",
		L"취소"
	);
	
	cancelButton->SetOnClickCallback([this]() {
		this->OnCancelButtonClicked();
	});
	cancelButton->SetActive(false);  // 초기에는 비활성화
	UIManager::GetInstance()->AddUIButton(cancelButton);
	
	// 캐릭터 버튼들 생성
	CreateCharacterButtons();
}

void CharacterSelectScene::CreateCharacterButtons()
{
	OutputDebugStringW((L"CharacterSelectScene: 캐릭터 버튼 생성 시작 - " + std::to_wstring(m_characterList.size()) + L"개 캐릭터\n").c_str());
	
	// 모든 캐릭터에 대한 UI 요소들 생성
	for (size_t i = 0; i < m_characterList.size(); ++i) {
		const CharacterInfo& charInfo = m_characterList[i];
		
		OutputDebugStringW((L"CharacterSelectScene: 캐릭터 " + std::to_wstring(i) + L" 버튼 생성 - " + charInfo.name + L"\n").c_str());
		
		float buttonWidth = 150.0f;
		float buttonHeight = 150.0f;
		
		// HUD 배경 생성
		UIImage* hudBackground = new UIImage(
			static_cast<GameObjectID>(3000 + i * 10),
			charInfo.buttonPosX,
			charInfo.buttonPosY,
			buttonWidth,
			buttonHeight,
			LAYER_UI_BACKGROUND,
			L"../Resource/UI/quagmire_hud.png",
			1.0f
		);
		UIManager::GetInstance()->AddUIImage(hudBackground);
		
		// 캐릭터 이미지 생성
		UIImage* characterImage = new UIImage(
			static_cast<GameObjectID>(3001 + i * 10),
			charInfo.buttonPosX,
			charInfo.buttonPosY + 15.0f,
			buttonWidth * 0.8f,
			buttonHeight * 0.8f,
			LAYER_UI_FOREGROUND,
			charInfo.characterImagePath,
			1.0f
		);
		UIManager::GetInstance()->AddUIImage(characterImage);
		
		// 잠금 오버레이 생성 (해금되지 않은 캐릭터에만)
		if (!charInfo.isUnlocked) {
			UIImage* lockOverlay = new UIImage(
				static_cast<GameObjectID>(3003 + i * 10),
				charInfo.buttonPosX,
				charInfo.buttonPosY,
				buttonWidth,
				buttonHeight,
				LAYER_UI_FOREGROUND,
				L"../Resource/UI/locked_Character.png",
				2.0f
			);
			UIManager::GetInstance()->AddUIImage(lockOverlay);
		}
		
		// 캐릭터 버튼 생성 (클릭 이벤트 처리) - 모든 캐릭터에 대해 생성
		UIButton* characterButton = new UIButton(
			static_cast<GameObjectID>(3002 + i * 10),
			charInfo.buttonPosX,
			charInfo.buttonPosY,
			buttonWidth,
			buttonHeight,
			L"", // 빈 버튼
			L"", // 빈 버튼
			L""  // 텍스트 없음
		);
		
		// 람다로 캐릭터 인덱스를 캡처
		int characterIndex = static_cast<int>(i);
		characterButton->SetOnClickCallback([this, characterIndex]() {
			OnCharacterButtonClicked(characterIndex);
		});
		
		UIManager::GetInstance()->AddUIButton(characterButton);
	}
	
	OutputDebugStringW(L"CharacterSelectScene: 캐릭터 버튼 생성 완료\n");
}

void CharacterSelectScene::Update(float deltaTime)
{
	// 매니저들 업데이트
	UpdateManagers(deltaTime);
}

void CharacterSelectScene::LateUpdate()
{
	// 매니저들 LateUpdate
	LateUpdateManagers();
}

void CharacterSelectScene::Render()
{
	// 매니저들 렌더링
	RenderManagers();
	
	// 캐릭터 설명 텍스트 렌더링
	if (m_selectedCharacterIndex >= 0) {
		RenderCharacterDescription();
	}
}

void CharacterSelectScene::Release()
{
	// 텍스트 렌더링 관련 해제
	if (m_descriptionFont) {
		delete m_descriptionFont;
		m_descriptionFont = nullptr;
	}
	if (m_descriptionBrush) {
		delete m_descriptionBrush;
		m_descriptionBrush = nullptr;
	}
	
	// CharacterSelectScene에서 사용한 매니저들 해제
	ReleaseAllManagers();
}

void CharacterSelectScene::InitializeTextRendering()
{
	// 폰트 생성 (기본 폰트, 크기 16)
	m_descriptionFont = new Gdiplus::Font(L"맑은 고딕", 16.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	
	// 브러시 생성 (검은색)
	m_descriptionBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
}

void CharacterSelectScene::RenderCharacterDescription()
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& selectedChar = m_characterList[m_selectedCharacterIndex];
		
		// 캐릭터 정보 패널의 위치와 크기
		float panelX = WINCX / 2.0f + 300.0f - 250.0f; // 패널 중심에서 왼쪽으로
		float panelY = WINCY / 2.0f + 200.0f - 100.0f; // 패널 중심에서 위로
		float panelWidth = 500.0f;
		float panelHeight = 200.0f;
		
		// 텍스트 렌더링 영역 계산 (패널 내부에 여백)
		float textX = panelX + 20.0f;
		float textY = panelY + 20.0f;
		float textWidth = panelWidth - 40.0f;
		float textHeight = panelHeight - 40.0f;
		
		// 텍스트 렌더링
		RenderManager::GetInstance()->RenderUIText(
			selectedChar.description,
			m_descriptionFont,
			m_descriptionBrush,
			textX,
			textY,
			textWidth,
			textHeight,
			LAYER_UI_FOREGROUND,
			6.0f  // 텍스트는 다른 UI보다 위에 렌더링
		);
	}
}

void CharacterSelectScene::UpdateCharacterSelection()
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& selectedChar = m_characterList[m_selectedCharacterIndex];
		
		// 선택된 캐릭터의 포트레이트 이미지 변경
		UIImage* selectedPortrait = UIManager::GetInstance()->FindUIImage(GOID_PLAYER_PORTRAIT);
		if (selectedPortrait)
		{
			// 잠긴 캐릭터는 lock.png, 해금된 캐릭터는 해당 포트레이트 사용
			std::wstring portraitPath;
			if (!selectedChar.isUnlocked) 
			{
				portraitPath = L"../Resource/UI/locked.png";
			} else {
				portraitPath = selectedChar.portraitPath;
			}
			
			// UIImage의 LoadBitmap 함수를 사용해서 포트레이트 이미지 변경
			selectedPortrait->LoadBitmap(portraitPath);
		}
	}
}

std::wstring CharacterSelectScene::GetSelectedCharacterName() const
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		return m_characterList[m_selectedCharacterIndex].name;
	}
	return L"UnKnown"; // 기본값
}

GameObjectID CharacterSelectScene::GetSelectedCharacterID() const
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		return m_characterList[m_selectedCharacterIndex].characterID;
	}
	return GOID_NONE; // 기본값
}

void CharacterSelectScene::OnCharacterButtonClicked(int characterIndex)
{
	if (characterIndex >= 0 && characterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& charInfo = m_characterList[characterIndex];
		
		OutputDebugStringW((L"Character Selected: " + charInfo.name + L"\n").c_str());
		
		m_selectedCharacterIndex = characterIndex;
		m_isLockedCharacterSelected = !charInfo.isUnlocked;
		m_currentState = CharacterSelectionState::CHARACTER_INFO;
		
		// UI 보이기
		UIImage* selectedPortrait = UIManager::GetInstance()->FindUIImage(GOID_PLAYER_PORTRAIT);
		if (selectedPortrait) {
			selectedPortrait->SetActive(true);
		}
		
		UIImage* characterInfoPanel = UIManager::GetInstance()->FindUIImage(GOID_PLAYER_INFO);
		if (characterInfoPanel) {
			characterInfoPanel->SetActive(true);
		}
		
		UIButton* selectButton = UIManager::GetInstance()->FindUIButton(GOID_SELECT_BUTTON);
		if (selectButton) {
			selectButton->SetActive(true);
		}
		
		UIButton* cancelButton = UIManager::GetInstance()->FindUIButton(GOID_CANCEL_SELECTION);
		if (cancelButton) {
			cancelButton->SetActive(true);
		}
		
		UpdateCharacterSelection();
		
		// 잠시 후 CONFIRM_SELECT 상태로 전환
		m_currentState = CharacterSelectionState::CONFIRM_SELECT;
		
		// 선택 버튼 상태 업데이트
		UpdateSelectButtonState();
	}
}

void CharacterSelectScene::OnSelectButtonClicked()
{
	if (m_selectedCharacterIndex == -1) {
		OutputDebugStringW(L"No character selected!\n");
		return;
	}
	
	// 잠긴 캐릭터는 선택할 수 없음
	if (m_isLockedCharacterSelected) {
		OutputDebugStringW(L"Cannot select locked character!\n");
		return;
	}
	
	std::wstring selectedCharacterName = GetSelectedCharacterName();
	GameObjectID selectedCharacterID = GetSelectedCharacterID();
	OutputDebugStringW((L"Character Confirmed! Loading Game Scene with: " + selectedCharacterName + L" (ID: " + std::to_wstring(selectedCharacterID) + L")\n").c_str());
	
	m_currentState = CharacterSelectionState::CLICK_GAME;
	// 선택된 캐릭터 정보를 SceneManager에 전달하여 게임 씬으로 전환
	SceneManager::GetInstance()->LoadGameScene(L"../MapData/00_map.dsm", selectedCharacterID);
}

void CharacterSelectScene::OnCancelButtonClicked()
{
	OutputDebugStringW(L"Character Selection Cancelled!\n");
	
	// 캐릭터 선택 초기화, 브라우징 상태로 돌아가기
	m_selectedCharacterIndex = -1;
	m_isLockedCharacterSelected = false;
	m_isSelectButtonDisabled = false;
	m_currentState = CharacterSelectionState::BROWSING;
	
	// UI 숨기기
	UIImage* selectedPortrait = UIManager::GetInstance()->FindUIImage(GOID_PLAYER_PORTRAIT);
	if (selectedPortrait) {
		selectedPortrait->SetActive(false);
	}
	
	UIImage* characterInfoPanel = UIManager::GetInstance()->FindUIImage(GOID_PLAYER_INFO);
	if (characterInfoPanel) {
		characterInfoPanel->SetActive(false);
	}
	
	UIButton* selectButton = UIManager::GetInstance()->FindUIButton(GOID_SELECT_BUTTON);
	if (selectButton) {
		selectButton->SetActive(false);
	}
	
	UIButton* cancelButton = UIManager::GetInstance()->FindUIButton(GOID_CANCEL_SELECTION);
	if (cancelButton) {
		cancelButton->SetActive(false);
	}
	
	// 선택 버튼 상태 초기화
	UpdateSelectButtonState();
}

void CharacterSelectScene::OnBackButtonClicked()
{
	OutputDebugStringW(L"Back button clicked! Returning to Title Scene\n");
	
	// UI 숨기기
	UIImage* selectedPortrait = UIManager::GetInstance()->FindUIImage(GOID_PLAYER_PORTRAIT);
	if (selectedPortrait) {
		selectedPortrait->SetActive(false);
	}
	
	UIImage* characterInfoPanel = UIManager::GetInstance()->FindUIImage(GOID_PLAYER_INFO);
	if (characterInfoPanel) {
		characterInfoPanel->SetActive(false);
	}
	
	UIButton* selectButton = UIManager::GetInstance()->FindUIButton(GOID_SELECT_BUTTON);
	if (selectButton) {
		selectButton->SetActive(false);
	}
	
	UIButton* cancelButton = UIManager::GetInstance()->FindUIButton(GOID_CANCEL_SELECTION);
	if (cancelButton) {
		cancelButton->SetActive(false);
	}
	
	// 타이틀 씬으로 돌아가기
	SceneManager::GetInstance()->ReturnToTitle();
}

void CharacterSelectScene::UpdateSelectButtonState()
{
	// 잠긴 캐릭터가 선택되었으면 선택 버튼 비활성화
	m_isSelectButtonDisabled = m_isLockedCharacterSelected;
	
	UIButton* selectButton = UIManager::GetInstance()->FindUIButton(GOID_SELECT_BUTTON);
	if (selectButton) {
		if (m_isSelectButtonDisabled) {
			// 버튼을 비활성화 상태로 변경
			selectButton->SetDisabled(true);
		} else {
			// 버튼을 활성화 상태로 변경
			selectButton->SetDisabled(false);
		}
	}
}

void CharacterSelectScene::UpdateCharacterUnlockStatus()
{
	// 게임 진행 상황에 따라 캐릭터 해금 상태 업데이트
	for (auto& charInfo : m_characterList) {
		charInfo.isUnlocked = m_gameProgress.IsCharacterUnlocked(charInfo.characterID);
	}
	
	// UI 요소들 재생성
	CreateUI();
}

void CharacterSelectScene::UpdateManagers(float deltaTime)
{
	// CharacterSelectScene에서는 UIManager와 InputManager 업데이트
	UIManager::GetInstance()->Update(deltaTime);
	InputManager::GetInstance()->Update(deltaTime);
}

void CharacterSelectScene::LateUpdateManagers()
{
	// CharacterSelectScene에서는 UIManager와 InputManager LateUpdate
	UIManager::GetInstance()->LateUpdate();
	InputManager::GetInstance()->LateUpdate();
}

void CharacterSelectScene::RenderManagers()
{
	// CharacterSelectScene에서는 UIManager와 InputManager 렌더링
	UIManager::GetInstance()->Render();
	InputManager::GetInstance()->Render();
}

void CharacterSelectScene::ReleaseManagers()
{
	// CharacterSelectScene에서는 UIManager만 해제
	UIManager::GetInstance()->Release();
}

void CharacterSelectScene::InitializeManagers()
{
	OutputDebugStringW(L"CharacterSelectScene: 매니저 초기화 시작\n");
	
	// CharacterSelectScene에서는 UIManager와 InputManager만 초기화
	UIManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	
	OutputDebugStringW(L"CharacterSelectScene: 매니저 초기화 완료\n");
}

void CharacterSelectScene::ReleaseAllManagers()
{
	OutputDebugStringW(L"CharacterSelectScene: 매니저 해제 시작\n");
	
	// CharacterSelectScene에서 사용한 매니저들 해제
	InputManager::GetInstance()->Release();
	UIManager::GetInstance()->Release();
	
	OutputDebugStringW(L"CharacterSelectScene: 매니저 해제 완료\n");
}
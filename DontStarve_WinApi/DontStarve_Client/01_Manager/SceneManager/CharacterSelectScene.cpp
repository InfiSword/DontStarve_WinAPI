#include "99_Default/pch.h"
#include "CharacterSelectScene.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"
#include "../../02_GameObject/UI/UIText.h"
#include "SceneManager.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"
#include "../RenderManager/RenderManager.h"

CharacterSelectScene::CharacterSelectScene()
	: m_currentState(CharacterSelectionState::BROWSING), m_selectedCharacterIndex(-1),
	  m_isLockedCharacterSelected(false), m_isSelectButtonDisabled(false)
{
}

CharacterSelectScene::~CharacterSelectScene()
{
	Release();
}

void CharacterSelectScene::Init()
{
	OutputDebugStringW(L"=== CharacterSelectScene::Init() 시작 ===\n");
	
	// CharacterSelectScene에 필요한 매니저들 초기화
	UIManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	
	OutputDebugStringW(L"CharacterSelectScene: 매니저 초기화 완료\n");
	
	// 캐릭터 목록 초기화
	InitializeCharacters();
	OutputDebugStringW((L"CharacterSelectScene: 캐릭터 목록 초기화 완료 - " + std::to_wstring(m_characterList.size()) + L"개 캐릭터\n").c_str());
	
	// UI 생성
	UIManager* uiManager = UIManager::GetInstance();

	// 배경 이미지 생성 (전체 화면)
	OutputDebugStringW(L"CharacterSelectScene: 배경 이미지 생성 시작\n");
	UIImage* backgroundImage = new UIImage(
		static_cast<GameObjectID>(GOID_MAIN_BG),
		static_cast<float>(WINCX),
		static_cast<float>(WINCY),
		LAYER_UI_BACKGROUND,
		L"../Resource/UI/BG.png",
		0.f,
		0.0f, 0.0f,  // anchorMin
		1.0f, 1.0f,  // anchorMax
		0.0f, 0.0f    // anchoredPosition
	);
	uiManager->AddUIImage(backgroundImage);
	OutputDebugStringW(L"CharacterSelectScene: 배경 이미지 생성 완료\n");

	// 뒤로가기 버튼 생성 (좌측 중앙)
	UIButton* backButton = new UIButton(
		static_cast<GameObjectID>(GOID_BACK_BUTTON),
		80.0f,
		100.0f,
		L"../Resource/UI/Button.png",
		L"../Resource/UI/Button.png",
		0.0f, 0.5f,  // anchorMin (좌측 중앙)
		0.0f, 0.5f,  // anchorMax (좌측 중앙)
		100.0f, 300.0f // anchoredPosition (좌측에서 오른쪽으로 100px, 중앙에서 아래로 300px)
	);
	backButton->SetOnClickCallback([this]() {
		OnBackButtonClicked();
		});
	uiManager->AddUIButton(backButton);

	// 선택된 캐릭터 포트레이트 (우측 중앙) - 초기에는 숨김
	UIImage* selectedPortrait = new UIImage(
		static_cast<GameObjectID>(GOID_PLAYER_PORTRAIT),
		350.0f,
		500.0f,
		LAYER_UI_FOREGROUND,
		L"../Resource/UI/wilson.png",
		1.0f,
		1.0f, 0.5f,  // anchorMin (우측 중앙)
		1.0f, 0.5f,  // anchorMax (우측 중앙)
		-300.0f, -150.0f // anchoredPosition (우측에서 왼쪽으로 300px, 중앙에서 위로 150px)
	);
	selectedPortrait->SetActive(false);  // 초기에는 비활성화
	uiManager->AddUIImage(selectedPortrait);

	// 캐릭터 정보창 (우측 중앙) - 초기에는 숨김
	UIImage* characterInfoPanel = new UIImage(
		static_cast<GameObjectID>(GOID_PLAYER_INFO),
		500.0f,
		200.0f,
		LAYER_UI_FOREGROUND,
		L"../Resource/UI/UI4.png",
		1.0f,
		1.0f, 0.5f,  // anchorMin (우측 중앙)
		1.0f, 0.5f,  // anchorMax (우측 중앙)
		-300.0f, 200.0f // anchoredPosition (우측에서 왼쪽으로 300px, 중앙에서 아래로 200px)
	);
	characterInfoPanel->SetActive(false);  // 초기에는 비활성화
	uiManager->AddUIImage(characterInfoPanel);

	// 선택 버튼 (하단 중앙) - 초기에는 숨김
	UIButton* selectButton = new UIButton(
		static_cast<GameObjectID>(GOID_SELECT_BUTTON),
		120.0f,
		50.0f,
		L"../Resource/UI/Select_Bar.png",
		L"../Resource/UI/Select_Bar.png",
		0.5f, 0.0f,  // anchorMin (하단 중앙)
		0.5f, 0.0f,  // anchorMax (하단 중앙)
		-150.0f, -350.0f // anchoredPosition (중앙에서 왼쪽으로 150px, 하단에서 위로 350px)
	);
	selectButton->SetOnClickCallback([this]() {
		this->OnSelectButtonClicked();
		});
	selectButton->SetActive(false);  // 초기에는 비활성화
	uiManager->AddUIButton(selectButton);

	// 선택 버튼 텍스트 생성 (버튼과 동일한 anchor)
	UIText* selectButtonText = new UIText(
		static_cast<GameObjectID>(GOID_SELECT_BUTTON_TEXT),
		120.0f,
		50.0f,
		L"선택",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.1f,
		L"맑은 고딕",
		16.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.0f,  // anchorMin (하단 중앙)
		0.5f, 0.0f,  // anchorMax (하단 중앙)
		-150.0f, -350.0f // anchoredPosition (중앙에서 왼쪽으로 150px, 하단에서 위로 350px)
	);
	selectButtonText->SetActive(false);  // 초기에는 비활성화
	uiManager->AddUIText(selectButtonText);

	// 취소 버튼 (하단 중앙) - 초기에는 숨김
	UIButton* cancelButton = new UIButton(
		static_cast<GameObjectID>(GOID_CANCEL_SELECTION),
		120.0f,
		50.0f,
		L"../Resource/UI/Select_Bar.png",
		L"../Resource/UI/Select_Bar.png",
		0.5f, 0.0f,  // anchorMin (하단 중앙)
		0.5f, 0.0f,  // anchorMax (하단 중앙)
		150.0f, -350.0f // anchoredPosition (중앙에서 오른쪽으로 150px, 하단에서 위로 350px)
	);
	cancelButton->SetOnClickCallback([this]() {
		this->OnCancelButtonClicked();
		});
	cancelButton->SetActive(false);  // 초기에는 비활성화
	uiManager->AddUIButton(cancelButton);

	// 취소 버튼 텍스트 생성 (버튼과 동일한 anchor)
	UIText* cancelButtonText = new UIText(
		static_cast<GameObjectID>(GOID_CANCEL_SELECTION_TEXT),
		120.0f,
		50.0f,
		L"취소",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		0.1f,
		L"맑은 고딕",
		16.0f,
		Gdiplus::StringAlignmentCenter,
		Gdiplus::StringAlignmentCenter,
		0.5f, 0.0f,  // anchorMin (하단 중앙)
		0.5f, 0.0f,  // anchorMax (하단 중앙)
		150.0f, -350.0f // anchoredPosition (중앙에서 오른쪽으로 150px, 하단에서 위로 350px)
	);
	cancelButtonText->SetActive(false);  // 초기에는 비활성화
	uiManager->AddUIText(cancelButtonText);

	// 캐릭터 설명 텍스트 생성 (캐릭터 정보창과 같은 anchor, 상대적 위치)
	UIText* descriptionText = new UIText(
		static_cast<GameObjectID>(GOID_CHARACTER_DESCRIPTION),
		500.0f - 40.0f,  // textWidth
		200.0f - 40.0f,  // textHeight
		L"",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		6.0f,
		L"맑은 고딕",
		16.0f,
		Gdiplus::StringAlignmentNear,
		Gdiplus::StringAlignmentNear,
		1.0f, 0.5f,  // anchorMin (우측 중앙, 정보창과 동일)
		1.0f, 0.5f,  // anchorMax (우측 중앙, 정보창과 동일)
		-280.0f, 180.0f // anchoredPosition (정보창보다 약간 왼쪽 위로)
	);
	descriptionText->SetActive(false);  // 초기에는 비활성화
	uiManager->AddUIText(descriptionText);

	// 캐릭터 버튼들 생성
	CreateCharacterButtons();

	OutputDebugStringW(L"CharacterSelectScene: UI 생성 완료\n");
	
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
	
	// 캐릭터들의 위치 설정
	float startX = 150.0f;
	float spacing = 200.0f;
	float characterY = 300.0f;
	
	// Wilson 캐릭터 추가 (기본 해금)
	m_characterList.emplace_back(
		L"Wilson",
		L"../Resource/UI/wilson.png",
		L"../Resource/UI/Willson_Character.png",
		L"기본 캐릭터입니다.\n모든 상황에서 안정적으로 플레이할 수 있는\n 균형잡힌 캐릭터입니다.",
		startX,
		characterY,
		GOID_PLAYER_WILSON,
		true  
	);
	
	// Willow 캐릭터 추가 (불타는 나무 클릭 시 해금)
	m_characterList.emplace_back(
		L"Willow",
		L"../Resource/Objects/Player/Willson/willow_portrait.png",
		L"../Resource/Objects/Player/Willson/willow_character.png",
		L"불의 마법사입니다.\n불을 두려워하지 않고 활용할 수 있습니다.\n\n해금 조건: 불타는 나무 클릭",
		startX + spacing,
		characterY,
		GOID_PLAYER_WILLOW,
		m_gameProgress.IsCharacterUnlocked(GOID_PLAYER_WILLOW)
	);
	
	// Wolfgang 캐릭터 추가 (돌멩이 던지기 클릭 시 해금)
	m_characterList.emplace_back(
		L"Wolfgang",
		L"../Resource/Objects/Player/Willson/wolfgang_portrait.png",
		L"../Resource/Objects/Player/Willson/wolfgang_character.png",
		L"강한 캐릭터입니다.\n체력이 높을수록 더 강해지는 캐릭터입니다.\n\n해금 조건: 돌멩이 던지기 클릭",
		startX + spacing * 2,
		characterY,
		GOID_PLAYER_WOLFGANG,
		m_gameProgress.IsCharacterUnlocked(GOID_PLAYER_WOLFGANG)
	);
}

void CharacterSelectScene::Update(float deltaTime)
{
	// 매니저들 업데이트
	UIManager::GetInstance()->Update(deltaTime);
	InputManager::GetInstance()->Update(deltaTime);
}
 
void CharacterSelectScene::CreateCharacterButtons()
{
	OutputDebugStringW((L"CharacterSelectScene: 캐릭터 버튼 생성 시작 - " + std::to_wstring(m_characterList.size()) + L"개 캐릭터\n").c_str());
	
	float screenHeight = static_cast<float>(WINCY);
	
	// 모든 캐릭터에 대한 UI 요소들 생성
	for (size_t i = 0; i < m_characterList.size(); ++i) {
		const CharacterInfo& charInfo = m_characterList[i];
		
		OutputDebugStringW((L"CharacterSelectScene: 캐릭터 " + std::to_wstring(i) + L" 버튼 생성 - " + charInfo.name + L"\n").c_str());
		
		float buttonWidth = 150.0f;
		float buttonHeight = 150.0f;
		
		// anchor 기반 위치 계산 (좌측 상단 기준)
		float anchorPosX = charInfo.buttonPosX;
		float anchorPosY = charInfo.buttonPosY - screenHeight / 2.0f;
		
		// HUD 배경 생성
		UIImage* hudBackground = new UIImage(
			static_cast<GameObjectID>(3000 + i * 10),
			buttonWidth,
			buttonHeight,
			LAYER_UI_BACKGROUND,
			L"../Resource/UI/quagmire_hud.png",
			1.0f,
			0.0f, 0.5f,  // anchorMin (좌측 중앙)
			0.0f, 0.5f,  // anchorMax (좌측 중앙)
			anchorPosX, anchorPosY // anchoredPosition
		);
		UIManager::GetInstance()->AddUIImage(hudBackground);
		
		// 캐릭터 이미지 생성 (HUD 배경보다 약간 아래)
		UIImage* characterImage = new UIImage(
			static_cast<GameObjectID>(3001 + i * 10),
			buttonWidth * 0.8f,
			buttonHeight * 0.8f,
			LAYER_UI_FOREGROUND,
			charInfo.characterImagePath,
			1.0f,
			0.0f, 0.5f,  // anchorMin (좌측 중앙)
			0.0f, 0.5f,  // anchorMax (좌측 중앙)
			anchorPosX, anchorPosY + 15.0f // anchoredPosition (15px 아래)
		);
		UIManager::GetInstance()->AddUIImage(characterImage);
		
		// 잠금 오버레이 생성 (해금되지 않은 캐릭터만)
		if (!charInfo.isUnlocked) {
			UIImage* lockOverlay = new UIImage(
				static_cast<GameObjectID>(3003 + i * 10),
				buttonWidth,
				buttonHeight,
				LAYER_UI_FOREGROUND,
				L"../Resource/UI/locked_Character.png",
				2.0f,
				0.0f, 0.5f,  // anchorMin (좌측 중앙)
				0.0f, 0.5f,  // anchorMax (좌측 중앙)
				anchorPosX, anchorPosY // anchoredPosition
			);
			UIManager::GetInstance()->AddUIImage(lockOverlay);
		}
		
		// 캐릭터 버튼 생성 (클릭 이벤트 처리) - 모든 캐릭터에 대해 생성
		// 투명한 버튼을 위해 빈 이미지 경로 사용 (클릭 영역만 필요)
		UIButton* characterButton = new UIButton(
			static_cast<GameObjectID>(3002 + i * 10),
			buttonWidth,
			buttonHeight,
			L"",  // 빈 경로 (투명 버튼)
			L"",  // 빈 경로
			0.0f, 0.5f,  // anchorMin (좌측 중앙)
			0.0f, 0.5f,  // anchorMax (좌측 중앙)
			anchorPosX, anchorPosY // anchoredPosition
		);
		
		// 람다로 캐릭터 인덱스 캡처
		int characterIndex = static_cast<int>(i);
		characterButton->SetOnClickCallback([this, characterIndex]() {
			OnCharacterButtonClicked(characterIndex);
		});
		
		UIManager::GetInstance()->AddUIButton(characterButton);
	}
	
	OutputDebugStringW(L"CharacterSelectScene: 캐릭터 버튼 생성 완료\n");
}
 
void CharacterSelectScene::LateUpdate()
{
	// 매니저들 LateUpdate
	UIManager::GetInstance()->LateUpdate();
	InputManager::GetInstance()->LateUpdate();
}

void CharacterSelectScene::Render()
{
	// 매니저들 렌더링
	UIManager::GetInstance()->Render();
	InputManager::GetInstance()->Render();
}

void CharacterSelectScene::Release()
{
	// UI 객체들은 UIManager에서 해제되므로 별도 처리 불필요
	// CharacterSelectScene에서 사용한 매니저들 해제
	InputManager::GetInstance()->Release();
	UIManager::GetInstance()->Release();
}

void CharacterSelectScene::UpdateCharacterDescription()
{
	UIManager* uiManager = UIManager::GetInstance();
	UIText* descriptionText = uiManager->FindUIText(GOID_CHARACTER_DESCRIPTION);
	
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& selectedChar = m_characterList[m_selectedCharacterIndex];
		
		if (descriptionText) {
			descriptionText->SetText(selectedChar.description);
			descriptionText->SetActive(true);
		}
	}
	else {
		if (descriptionText) {
			descriptionText->SetActive(false);
		}
	}
}

void CharacterSelectScene::UpdateCharacterSelection()
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& selectedChar = m_characterList[m_selectedCharacterIndex];
		
		// 선택된 캐릭터의 포트레이트 이미지 업데이트
		UIManager* uiManager = UIManager::GetInstance();
		UIImage* selectedPortrait = uiManager->FindUIImage(GOID_PLAYER_PORTRAIT);
		
		if (selectedPortrait)
		{
			// 잠금 캐릭터는 lock.png, 해금된 캐릭터는 해당 포트레이트 경로
			std::wstring portraitPath;
			if (!selectedChar.isUnlocked) 
			{
				portraitPath = L"../Resource/UI/locked.png";
			} else {
				portraitPath = selectedChar.portraitPath;
			}
			
			// UIImage의 LoadSprite 메서드를 사용하여 포트레이트 이미지 업데이트
			selectedPortrait->LoadSprite(portraitPath);
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
		
		// UI 표시
		UIManager* uiManager = UIManager::GetInstance();
		UIImage* selectedPortrait = uiManager->FindUIImage(GOID_PLAYER_PORTRAIT);
		UIImage* characterInfoPanel = uiManager->FindUIImage(GOID_PLAYER_INFO);
		UIButton* selectButton = uiManager->FindUIButton(GOID_SELECT_BUTTON);
		UIText* selectButtonText = uiManager->FindUIText(GOID_SELECT_BUTTON_TEXT);
		UIButton* cancelButton = uiManager->FindUIButton(GOID_CANCEL_SELECTION);
		UIText* cancelButtonText = uiManager->FindUIText(GOID_CANCEL_SELECTION_TEXT);
		
		if (selectedPortrait) {
			selectedPortrait->SetActive(true);
		}
		
		if (characterInfoPanel) {
			characterInfoPanel->SetActive(true);
		}
		
		if (selectButton) {
			selectButton->SetActive(true);
		}
		
		if (selectButtonText) {
			selectButtonText->SetActive(true);
		}
		
		if (cancelButton) {
			cancelButton->SetActive(true);
		}
		
		if (cancelButtonText) {
			cancelButtonText->SetActive(true);
		}
		
		UpdateCharacterSelection();
		
		// 캐릭터 설명 텍스트 업데이트
		UpdateCharacterDescription();
		
		// 상태를 CONFIRM_SELECT 상태로 전환
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
	
	// 잠금 캐릭터는 선택할 수 없음
	if (m_isLockedCharacterSelected) {
		OutputDebugStringW(L"Cannot select locked character!\n");
		return;
	}
	
	std::wstring selectedCharacterName = GetSelectedCharacterName();
	GameObjectID selectedCharacterID = GetSelectedCharacterID();
	OutputDebugStringW((L"Character Confirmed! Loading Game Scene with: " + selectedCharacterName + L" (ID: " + std::to_wstring(selectedCharacterID) + L")\n").c_str());
	
	m_currentState = CharacterSelectionState::CLICK_GAME;
	
	// 선택된 캐릭터 정보를 SceneManager에 전달하여 게임 씬으로 전환 요청
	SceneManager::GetInstance()->RequestLoadGameScene(L"../MapData/00_map.dsm", selectedCharacterID);
}

void CharacterSelectScene::OnCancelButtonClicked()
{
	OutputDebugStringW(L"Character Selection Cancelled!\n");
	
	// 캐릭터 선택 초기화, 브라우징 상태로 되돌리기
	m_selectedCharacterIndex = -1;
	m_isLockedCharacterSelected = false;
	m_isSelectButtonDisabled = false;
	m_currentState = CharacterSelectionState::BROWSING;
	
	// UI 숨김
	UIManager* uiManager = UIManager::GetInstance();
	UIImage* selectedPortrait = uiManager->FindUIImage(GOID_PLAYER_PORTRAIT);
	UIImage* characterInfoPanel = uiManager->FindUIImage(GOID_PLAYER_INFO);
	UIButton* selectButton = uiManager->FindUIButton(GOID_SELECT_BUTTON);
	UIText* selectButtonText = uiManager->FindUIText(GOID_SELECT_BUTTON_TEXT);
	UIButton* cancelButton = uiManager->FindUIButton(GOID_CANCEL_SELECTION);
	UIText* cancelButtonText = uiManager->FindUIText(GOID_CANCEL_SELECTION_TEXT);
	UIText* descriptionText = uiManager->FindUIText(GOID_CHARACTER_DESCRIPTION);
	
	if (selectedPortrait) {
		selectedPortrait->SetActive(false);
	}
	
	if (characterInfoPanel) {
		characterInfoPanel->SetActive(false);
	}
	
	if (selectButton) {
		selectButton->SetActive(false);
	}
	
	if (selectButtonText) {
		selectButtonText->SetActive(false);
	}
	
	if (cancelButton) {
		cancelButton->SetActive(false);
	}
	
	if (cancelButtonText) {
		cancelButtonText->SetActive(false);
	}
	
	if (descriptionText) {
		descriptionText->SetActive(false);
	}
	
	// 선택 버튼 상태 초기화
	UpdateSelectButtonState();
}

void CharacterSelectScene::OnBackButtonClicked()
{
	OutputDebugStringW(L"Back button clicked! Returning to Title Scene\n");
	
	// UI 숨김
	UIManager* uiManager = UIManager::GetInstance();
	UIImage* selectedPortrait = uiManager->FindUIImage(GOID_PLAYER_PORTRAIT);
	UIImage* characterInfoPanel = uiManager->FindUIImage(GOID_PLAYER_INFO);
	UIButton* selectButton = uiManager->FindUIButton(GOID_SELECT_BUTTON);
	UIText* selectButtonText = uiManager->FindUIText(GOID_SELECT_BUTTON_TEXT);
	UIButton* cancelButton = uiManager->FindUIButton(GOID_CANCEL_SELECTION);
	UIText* cancelButtonText = uiManager->FindUIText(GOID_CANCEL_SELECTION_TEXT);
	
	if (selectedPortrait) {
		selectedPortrait->SetActive(false);
	}
	
	if (characterInfoPanel) {
		characterInfoPanel->SetActive(false);
	}
	
	if (selectButton) {
		selectButton->SetActive(false);
	}
	
	if (selectButtonText) {
		selectButtonText->SetActive(false);
	}
	
	if (cancelButton) {
		cancelButton->SetActive(false);
	}
	
	if (cancelButtonText) {
		cancelButtonText->SetActive(false);
	}
	
	// 타이틀 씬으로 되돌리기 요청
	SceneManager::GetInstance()->RequestLoadTitleScene();
}

void CharacterSelectScene::UpdateSelectButtonState()
{
	// 잠금 캐릭터가 선택되었다면 선택 버튼 비활성화
	m_isSelectButtonDisabled = m_isLockedCharacterSelected;
	
	UIManager* uiManager = UIManager::GetInstance();
	UIButton* selectButton = uiManager->FindUIButton(GOID_SELECT_BUTTON);
	
	if (selectButton) {
		if (m_isSelectButtonDisabled) {
			// 버튼을 비활성화 상태로 설정
			selectButton->SetDisabled(true);
		} else {
			// 버튼을 활성화 상태로 설정
			selectButton->SetDisabled(false);
		}
	}
}

void CharacterSelectScene::UpdateCharacterUnlockStatus()
{
	// 현재 진행 상황에 따라 캐릭터 해금 상태 업데이트
	for (auto& charInfo : m_characterList) {
		charInfo.isUnlocked = m_gameProgress.IsCharacterUnlocked(charInfo.characterID);
	}
	
	// UI 요소들 재생성
	// CreateUI();
}
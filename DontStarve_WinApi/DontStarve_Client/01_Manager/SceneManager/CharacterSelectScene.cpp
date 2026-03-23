#include "99_Default/pch.h"
#include "CharacterSelectScene.h"
#include "SceneManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../InputManager/InputManager.h"
#include "../RenderManager/RenderManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../GameProgressManager/GameProgressManager.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"
#include "../../02_GameObject/UI/UIText.h"

CharacterSelectScene::CharacterSelectScene()
	: m_currentState(CharacterSelectionState::BROWSING), m_selectedCharacterIndex(-1),
	  m_isLockedCharacterSelected(false), m_isSelectButtonDisabled(false)
{
}

CharacterSelectScene::~CharacterSelectScene()
{
	Release();
}

void CharacterSelectScene::Init(const MapData* mapData)
{
	OutputDebugStringW(L"=== CharacterSelectScene::Init() 시작 ===\n");
	
	// CharacterSelectScene에 필요한 매니저들 초기화
	ObjectManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	
	OutputDebugStringW(L"CharacterSelectScene: 매니저 초기화 완료\n");
	
	// 캐릭터 목록 초기화
	InitializeCharacters();
	OutputDebugStringW((L"CharacterSelectScene: 캐릭터 목록 초기화 완료 - " + std::to_wstring(m_characterList.size()) + L"개 캐릭터\n").c_str());
	
	// UI 생성
	ObjectManager* objectManager = ObjectManager::GetInstance();
	ResourceManager * resourceManager = ResourceManager::GetInstance();

	// 배경 이미지 생성 (전체 화면)
	OutputDebugStringW(L"CharacterSelectScene: 배경 이미지 생성 시작\n");
	UIImage* backgroundImage = new UIImage(
		static_cast<GameObjectID>(GOID_UI_IMAGE),
		static_cast<float>(WINCX),
		static_cast<float>(WINCY),
		LAYER_UI_BACKGROUND,
		L"Resource/UI/BG.png",
		0.f,
		0.0f, 0.0f,  
		1.0f, 1.0f,  
		0.0f, 0.0f    
	);
	objectManager->AddGameObject(backgroundImage);
	OutputDebugStringW(L"CharacterSelectScene: 배경 이미지 생성 완료\n");

	// 뒤로가기 버튼 생성 (좌측 중앙)
	std::shared_ptr<Sprite> backNormalSprite = resourceManager->LoadSprite(L"Resource/UI/Button.png");
	std::shared_ptr<Sprite> backHoverSprite = resourceManager->LoadSprite(L"Resource/UI/Button.png");
	UIButton* backButton = new UIButton(
		static_cast<GameObjectID>(GOID_UI_BUTTON),
		80.0f,
		100.0f,
		backNormalSprite,
		backHoverSprite,
		0.0f, 0.5f,  
		0.0f, 0.5f,  
		100.0f, 300.0f 
	);
	backButton->SetOnClickCallback([this]() {
		OnBackButtonClicked();
		});
	objectManager->AddGameObject(backButton);

	// 선택된 캐릭터 포트레이트 (우측 중앙) - 초기에는 숨김
	m_pPlayerPortrait = new UIImage(
		static_cast<GameObjectID>(GOID_UI_IMAGE),
		350.0f,
		500.0f,
		LAYER_UI_FOREGROUND,
		L"Resource/UI/wilson.png",
		1.0f,
		1.0f, 0.5f,  
		1.0f, 0.5f,  
		-300.0f, -150.0f 
	);
	m_pPlayerPortrait->SetActive(false);  // 초기에는 비활성화
	objectManager->AddGameObject(m_pPlayerPortrait);

	// 캐릭터 정보창 (우측 중앙, 설명 텍스트와 위치 맞춤) - 초기에는 숨김
	m_pPlayerInfo = new UIImage(
		static_cast<GameObjectID>(GOID_UI_IMAGE),
		550.0f,
		200.0f,
		LAYER_UI_FOREGROUND,
		L"Resource/UI/UI4.png",
		1.0f,
		1.0f, 0.5f,  
		1.0f, 0.5f,  
		-320.0f, 180.0f 
	);
	m_pPlayerInfo->SetActive(false);  
	objectManager->AddGameObject(m_pPlayerInfo);

	// 캐릭터 설명 텍스트 생성 (캐릭터 정보창과 같은 anchor, 상대적 위치)
	m_pCharacterDescription = new UIText(
		static_cast<GameObjectID>(GOID_UI_TEXT),
		500.0f - 40.0f,  
		200.0f - 40.0f,  
		L"",
		Gdiplus::Color::Black,
		LAYER_UI_FOREGROUND,
		6.0f,
		L"맑은 고딕",
		16.0f,
		Gdiplus::StringAlignmentNear,
		Gdiplus::StringAlignmentNear,
		1.0f, 0.5f, 
		1.0f, 0.5f,  
		-300.0f, 200.0f 
	);
	m_pCharacterDescription->SetActive(false);  // 초기에는 비활성화
	objectManager->AddGameObject(m_pCharacterDescription);

	// 선택 버튼 (캐릭터 정보창 아래, 왼쪽) - 초기에는 숨김
	std::shared_ptr<Sprite> selectNormalSprite = resourceManager->LoadSprite(L"Resource/UI/Select_Bar.png");
	std::shared_ptr<Sprite> selectHoverSprite = resourceManager->LoadSprite(L"Resource/UI/Select_Bar.png");
	m_pSelectButton = new UIButton(
		static_cast<GameObjectID>(GOID_UI_BUTTON),
		120.0f,
		50.0f,
		selectNormalSprite,
		selectHoverSprite,
		1.0f, 0.5f, 
		1.0f, 0.5f,  
		-390.0f, 320.0f 
	);
	m_pSelectButton->SetOnClickCallback([this]() {
		this->OnSelectButtonClicked();
		});
	m_pSelectButton->SetActive(false);  // 초기에는 비활성화
	objectManager->AddGameObject(m_pSelectButton);

	// 선택 버튼 텍스트 생성 (버튼과 동일한 anchor)
	m_pSelectText = new UIText(
		static_cast<GameObjectID>(GOID_UI_TEXT),
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
		1.0f, 0.5f,  
		1.0f, 0.5f,  
		-400.0f, 320.0f 
	);
	m_pSelectText->SetActive(false);  // 초기에는 비활성화
	objectManager->AddGameObject(m_pSelectText);

	// 취소 버튼 (캐릭터 정보창 아래, 오른쪽) - 초기에는 숨김
	std::shared_ptr<Sprite> cancelNormalSprite = resourceManager->LoadSprite(L"Resource/UI/Select_Bar.png");
	std::shared_ptr<Sprite> cancelHoverSprite = resourceManager->LoadSprite(L"Resource/UI/Select_Bar.png");
	m_pCancelButton = new UIButton(
		static_cast<GameObjectID>(GOID_UI_BUTTON),
		120.0f,
		50.0f,
		cancelNormalSprite,
		cancelHoverSprite,
		1.0f, 0.5f,  
		1.0f, 0.5f,  
		-210.0f, 320.0f 
	);
	m_pCancelButton->SetOnClickCallback([this]() {
		this->OnCancelButtonClicked();
		});
	m_pCancelButton->SetActive(false);  // 초기에는 비활성화
	objectManager->AddGameObject(m_pCancelButton);

	// 취소 버튼 텍스트 생성 (버튼과 동일한 anchor)
	m_pCancelText = new UIText(
		static_cast<GameObjectID>(GOID_UI_TEXT),
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
		1.0f, 0.5f,  
		1.0f, 0.5f, 
		-220.0f, 320.0f 
	);
	m_pCancelText->SetActive(false);  // 초기에는 비활성화
	objectManager->AddGameObject(m_pCancelText);	

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
		L"Resource/UI/wilson.png",
		L"Resource/UI/Willson_Character.png",
		L"기본 캐릭터입니다.\n모든 상황에서 안정적으로 플레이할 수 있는\n 균형잡힌 캐릭터입니다.",
		startX,
		characterY,
		GOID_PLAYER_WILSON,
		true  
	);
	
	// Willow 캐릭터 추가 (불타는 나무 클릭 시 해금)
	ResourceManager* pRM = ResourceManager::GetInstance();
	const ResourcePathUtils::ObjectResourceDef* willowData = pRM->GetObjectResourceInfo(GOID_PLAYER_WILLOW);
	std::wstring willowPortraitPath;
	std::wstring willowCharacterPath;
	if (willowData) {
		willowPortraitPath = willowData->baseDir;
		if (!willowPortraitPath.empty() && willowPortraitPath.back() != L'\\' && willowPortraitPath.back() != L'/') {
			willowPortraitPath += L"\\";
		}
		willowPortraitPath += L"willow_portrait.png";
		
		willowCharacterPath = willowData->baseDir;
		if (!willowCharacterPath.empty() && willowCharacterPath.back() != L'\\' && willowCharacterPath.back() != L'/') {
			willowCharacterPath += L"\\";
		}
		willowCharacterPath += L"willow_character.png";
	} else {
		willowPortraitPath = L"Resource\\Objects\\Player\\Willow\\willow_portrait.png";
		willowCharacterPath = L"Resource\\Objects\\Player\\Willow\\willow_character.png";
	}
	m_characterList.emplace_back(
		L"Willow",
		willowPortraitPath,
		willowCharacterPath,
		L"불의 마법사입니다.\n불을 두려워하지 않고 활용할 수 있습니다.\n\n해금 조건: 늑대 던전 클리어",
		startX + spacing,
		characterY,
		GOID_PLAYER_WILLOW,
		GameProgressManager::GetInstance()->IsCharacterUnlocked(GOID_PLAYER_WILLOW)
	);
	
	// Wolfgang 캐릭터 추가 (돌멩이 던지기 클릭 시 해금)
	const ResourcePathUtils::ObjectResourceDef* wolfgangData = pRM->GetObjectResourceInfo(GOID_PLAYER_WOLFGANG);
	std::wstring wolfgangPortraitPath;
	std::wstring wolfgangCharacterPath;
	if (wolfgangData) {
		wolfgangPortraitPath = wolfgangData->baseDir;
		if (!wolfgangPortraitPath.empty() && wolfgangPortraitPath.back() != L'\\' && wolfgangPortraitPath.back() != L'/') {
			wolfgangPortraitPath += L"\\";
		}
		wolfgangPortraitPath += L"wolfgang_portrait.png";
		
		wolfgangCharacterPath = wolfgangData->baseDir;
		if (!wolfgangCharacterPath.empty() && wolfgangCharacterPath.back() != L'\\' && wolfgangCharacterPath.back() != L'/') {
			wolfgangCharacterPath += L"\\";
		}
		wolfgangCharacterPath += L"wolfgang_character.png";
	} else {
		wolfgangPortraitPath = L"Resource\\Objects\\Player\\Wolfgang\\wolfgang_portrait.png";
		wolfgangCharacterPath = L"Resource\\Objects\\Player\\Wolfgang\\wolfgang_character.png";
	}
	m_characterList.emplace_back(
		L"Wolfgang",
		wolfgangPortraitPath,
		wolfgangCharacterPath,
		L"강한 캐릭터입니다.\n체력이 높을수록 더 강해지는 캐릭터입니다.\n\n해금 조건: 거미 던전 클리어",
		startX + spacing * 2,
		characterY,
		GOID_PLAYER_WOLFGANG,
		GameProgressManager::GetInstance()->IsCharacterUnlocked(GOID_PLAYER_WOLFGANG)
	);
}

void CharacterSelectScene::Update(float deltaTime)
{
	ObjectManager::GetInstance()->Update(deltaTime);
}
 
void CharacterSelectScene::CreateCharacterButtons()
{
	OutputDebugStringW((L"CharacterSelectScene: 캐릭터 버튼 생성 시작 - " + std::to_wstring(m_characterList.size()) + L"개 캐릭터\n").c_str());
	
	float screenHeight = static_cast<float>(WINCY);
	ObjectManager* objectManager = ObjectManager::GetInstance();
	
	// 모든 캐릭터에 대한 UI 요소들 생성
	for (size_t i = 0; i < m_characterList.size(); ++i) {
		const CharacterInfo& charInfo = m_characterList[i];
		
		OutputDebugStringW((L"CharacterSelectScene: 캐릭터 " + std::to_wstring(i) + L" 버튼 생성 - " + charInfo.name + L"\n").c_str());
		
		float buttonWidth = 150.0f;
		float buttonHeight = 150.0f;
		
		// anchor 기반 위치 계산 (좌측 상단 기준)
		float anchorPosX = charInfo.buttonPosX;
		float anchorPosY = charInfo.buttonPosY - screenHeight / 2.0f;
		
		ResourceManager* resourceManager =  ResourceManager::GetInstance();

		// HUD 배경을 버튼으로 생성 (hover 시 밝게 표시)
		std::shared_ptr<Sprite> hudNormalSprite = resourceManager->LoadSprite(L"Resource/UI/quagmire_hud.png");
		std::shared_ptr<Sprite> hudHoverSprite = resourceManager->LoadSprite(L"Resource/UI/quagmire_hud.png");
		UIButton* hudButton = new UIButton(
			static_cast<GameObjectID>(GOID_UI_BUTTON),
			buttonWidth,
			buttonHeight,
			hudNormalSprite,  // normal 이미지
			hudHoverSprite,   // hover 이미지 (같은 이미지 사용)
			0.0f, 0.5f,		  // anchorMin (좌측 중앙)
			0.0f, 0.5f,		  // anchorMax (좌측 중앙)
			anchorPosX, anchorPosY // anchoredPosition
		);
		
		// Normal 상태는 원본 밝기, Hover 상태는 밝게 표시
		hudButton->SetNormalColor(Gdiplus::Color(255, 255, 255, 255));  
		hudButton->SetHoverColor(Gdiplus::Color(255, 250, 250, 200));  
		
		// 람다로 캐릭터 인덱스 캡처
		int characterIndex = static_cast<int>(i);
		hudButton->SetOnClickCallback([this, characterIndex]() {
			OnCharacterButtonClicked(characterIndex);
		});
		
		objectManager->AddGameObject(hudButton);
		
		// 캐릭터 이미지 또는 잠금 오버레이 생성
		std::wstring displayImagePath;
		if (!charInfo.isUnlocked) {
			// 잠금된 캐릭터는 잠금 이미지 표시
			displayImagePath = L"Resource/UI/locked_Character.png";
		} else {
			// 해금된 캐릭터는 캐릭터 이미지 표시
			displayImagePath = charInfo.characterImagePath;
		}
		
		UIImage* characterOverlay = new UIImage(
			static_cast<GameObjectID>(GOID_UI_IMAGE),
			buttonWidth * 0.8f,
			buttonHeight * 0.8f,
			LAYER_UI_FOREGROUND,  // 버튼 위에 표시
			displayImagePath,
			4.0f,  // sortKey
			0.0f, 0.5f,  // anchorMin (좌측 중앙)
			0.0f, 0.5f,  // anchorMax (좌측 중앙)
			anchorPosX, anchorPosY + 15.0f // anchoredPosition (15px 아래)
		);
		objectManager->AddGameObject(characterOverlay);
		
		OutputDebugStringW((L"캐릭터 버튼 생성 완료: ID=" + std::to_wstring(3004 + i * 10) + L"\n").c_str());
	}
	
	OutputDebugStringW(L"CharacterSelectScene: 캐릭터 버튼 생성 완료\n");
}
 
void CharacterSelectScene::LateUpdate()
{
	ObjectManager::GetInstance()->LateUpdate();
}

void CharacterSelectScene::Render()
{
	ObjectManager::GetInstance()->Render();
	// 매니저들 렌더링
	InputManager::GetInstance()->Render();
}

void CharacterSelectScene::Release()
{
	ObjectManager::GetInstance()->Release();
	InputManager::GetInstance()->Release();
}

void CharacterSelectScene::UpdateCharacterDescription()
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& selectedChar = m_characterList[m_selectedCharacterIndex];
		
		if (m_pCharacterDescription) {
			m_pCharacterDescription->SetText(selectedChar.description);
			m_pCharacterDescription->SetActive(true);
		}
	}
	else {
		if (m_pCharacterDescription) {
			m_pCharacterDescription->SetActive(false);
		}
	}
}

void CharacterSelectScene::UpdateCharacterSelection()
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& selectedChar = m_characterList[m_selectedCharacterIndex];
		
		// 선택된 캐릭터의 포트레이트 이미지 업데이트
		if (m_pPlayerPortrait)
		{
			// 잠금 캐릭터는 lock.png, 해금된 캐릭터는 해당 포트레이트 경로
			std::wstring portraitPath;
			if (!selectedChar.isUnlocked) 
			{
				portraitPath = L"Resource/UI/locked.png";
			} else {
				portraitPath = selectedChar.portraitPath;
			}
			
			// UIImage의 LoadSprite 메서드를 사용하여 포트레이트 이미지 업데이트
			m_pPlayerPortrait->LoadSprite(portraitPath);
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
	OutputDebugStringW(L"=== OnCharacterButtonClicked 호출됨 ===\n");
	OutputDebugStringW((L"캐릭터 인덱스: " + std::to_wstring(characterIndex) + L"\n").c_str());
	
	if (characterIndex >= 0 && characterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& charInfo = m_characterList[characterIndex];
		
		OutputDebugStringW((L"Character Selected: " + charInfo.name + L"\n").c_str());
		OutputDebugStringW((L"해금 상태: " + std::wstring(charInfo.isUnlocked ? L"해금됨" : L"잠김") + L"\n").c_str());
		
		m_selectedCharacterIndex = characterIndex;
		m_isLockedCharacterSelected = !charInfo.isUnlocked;
		m_currentState = CharacterSelectionState::CHARACTER_INFO;
		
		// UI 표시
		OutputDebugStringW(L"UI 요소 상태 확인:\n");
		OutputDebugStringW((L"  - m_pPlayerPortrait: " + std::wstring(m_pPlayerPortrait ? L"Valid" : L"NULL") + L"\n").c_str());
		OutputDebugStringW((L"  - m_pPlayerInfo: " + std::wstring(m_pPlayerInfo ? L"Valid" : L"NULL") + L"\n").c_str());
		OutputDebugStringW((L"  - m_pSelectButton: " + std::wstring(m_pSelectButton ? L"Valid" : L"NULL") + L"\n").c_str());
		OutputDebugStringW((L"  - m_pSelectText: " + std::wstring(m_pSelectText ? L"Valid" : L"NULL") + L"\n").c_str());
		OutputDebugStringW((L"  - m_pCancelButton: " + std::wstring(m_pCancelButton ? L"Valid" : L"NULL") + L"\n").c_str());
		OutputDebugStringW((L"  - m_pCancelText: " + std::wstring(m_pCancelText ? L"Valid" : L"NULL") + L"\n").c_str());
		
		if (m_pPlayerPortrait) {
			m_pPlayerPortrait->SetActive(true);
			OutputDebugStringW(L"  - m_pPlayerPortrait 활성화됨\n");
		}
		
		if (m_pPlayerInfo) {
			m_pPlayerInfo->SetActive(true);
			OutputDebugStringW(L"  - m_pPlayerInfo 활성화됨\n");
		}
		
		if (m_pSelectButton) {
			m_pSelectButton->SetActive(true);
			OutputDebugStringW(L"  - m_pSelectButton 활성화됨\n");
		}
		
		if (m_pSelectText) {
			m_pSelectText->SetActive(true);
			OutputDebugStringW(L"  - m_pSelectText 활성화됨\n");
		}
		
		if (m_pCancelButton) {
			m_pCancelButton->SetActive(true);
			OutputDebugStringW(L"  - m_pCancelButton 활성화됨\n");
		}
		
		if (m_pCancelText) {
			m_pCancelText->SetActive(true);
			OutputDebugStringW(L"  - m_pCancelText 활성화됨\n");
		}
		
		UpdateCharacterSelection();
		
		// 캐릭터 설명 텍스트 업데이트
		UpdateCharacterDescription();
		
		// 상태를 CONFIRM_SELECT 상태로 전환
		m_currentState = CharacterSelectionState::CONFIRM_SELECT;
		
		// 선택 버튼 상태 업데이트
		UpdateSelectButtonState();
		
		OutputDebugStringW(L"=== OnCharacterButtonClicked 완료 ===\n");
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
	SceneManager::GetInstance()->LoadGameScene(SCENE_GAME_FARMING_AREA, selectedCharacterID);
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
	if (m_pPlayerPortrait) {
		m_pPlayerPortrait->SetActive(false);
	}
	
	if (m_pPlayerInfo) {
		m_pPlayerInfo->SetActive(false);
	}
	
	if (m_pSelectButton) {
		m_pSelectButton->SetActive(false);
	}
	
	if (m_pSelectText) {
		m_pSelectText->SetActive(false);
	}
	
	if (m_pCancelButton) {
		m_pCancelButton->SetActive(false);
	}
	
	if (m_pCancelText) {
		m_pCancelText->SetActive(false);
	}
	
	if (m_pCharacterDescription) {
		m_pCharacterDescription->SetActive(false);
	}
	
	// 선택 버튼 상태 초기화
	UpdateSelectButtonState();
}

void CharacterSelectScene::OnBackButtonClicked()
{
	OutputDebugStringW(L"Back button clicked! Returning to Title Scene\n");
	
	// UI 숨김
	if (m_pPlayerPortrait) {
		m_pPlayerPortrait->SetActive(false);
	}
	
	if (m_pPlayerInfo) {
		m_pPlayerInfo->SetActive(false);
	}
	
	if (m_pSelectButton) {
		m_pSelectButton->SetActive(false);
	}
	
	if (m_pSelectText) {
		m_pSelectText->SetActive(false);
	}
	
	if (m_pCancelButton) {
		m_pCancelButton->SetActive(false);
	}
	
	if (m_pCancelText) {
		m_pCancelText->SetActive(false);
	}
	
	// 타이틀 씬으로 되돌리기 요청
	SceneManager::GetInstance()->LoadTitleScene();
}

void CharacterSelectScene::UpdateSelectButtonState()
{
	// 잠금 캐릭터가 선택되었다면 선택 버튼 비활성화
	m_isSelectButtonDisabled = m_isLockedCharacterSelected;
	
	if (m_pSelectButton) {
		if (m_isSelectButtonDisabled) {
			// 버튼을 비활성화 상태로 설정
			m_pSelectButton->SetDisabled(true);
		} else {
			// 버튼을 활성화 상태로 설정
			m_pSelectButton->SetDisabled(false);
		}
	}
}

void CharacterSelectScene::UpdateCharacterUnlockStatus()
{
	// GameProgressManager를 통해 캐릭터 해금 상태 업데이트
	GameProgressManager* progressManager = GameProgressManager::GetInstance();
	
	for (auto& charInfo : m_characterList) {
		charInfo.isUnlocked = progressManager->IsCharacterUnlocked(charInfo.characterID);
	}
	
	// UI 요소들 재생성
	// CreateUI();
}
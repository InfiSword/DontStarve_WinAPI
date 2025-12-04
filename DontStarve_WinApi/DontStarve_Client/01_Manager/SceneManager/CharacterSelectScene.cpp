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
	// CharacterSelectScene�� �ʿ��� �Ŵ����� �ʱ�ȭ
	InitializeManagers();
	
	// ĳ���� ��� �ʱ�ȭ
	InitializeCharacters();
	OutputDebugStringW((L"CharacterSelectScene: ĳ���� ��� �ʱ�ȭ �Ϸ� - " + std::to_wstring(m_characterList.size()) + L"�� ĳ����\n").c_str());
	
	// UI ����
	CreateUI();
	OutputDebugStringW(L"CharacterSelectScene: UI ���� �Ϸ�\n");
	
	// �ؽ�Ʈ ������ �ʱ�ȭ
	InitializeTextRendering();
	
	// �ʱ� ���� ����
	m_currentState = CharacterSelectionState::BROWSING;
	m_selectedCharacterIndex = -1;
	m_isLockedCharacterSelected = false;
	m_isSelectButtonDisabled = true;
	
	OutputDebugStringW(L"CharacterSelectScene: �ʱ�ȭ �Ϸ�\n");
}


void CharacterSelectScene::InitializeCharacters()
{
	// ĳ���� ��� �ʱ�ȭ
	m_characterList.clear();
	
	float screenWidth = static_cast<float>(WINCX);
	float screenHeight = static_cast<float>(WINCY);
	
	// ĳ���͵��� ��ġ ��ġ
	float startX = 150.0f;
	float spacing = 200.0f;
	float characterY = 300.0f;
	
	// Wilson ĳ���� �߰� (�⺻ �ر�)
	m_characterList.emplace_back(
		L"Wilson",
		L"../Resource/UI/wilson.png",
		L"../Resource/UI/Willson_Character.png",
		L"�⺻ ĳ�����Դϴ�.\n��� ��Ȳ���� ���������� ������ �� �ִ�\n �������� ĳ�����Դϴ�.",
		startX,
		characterY,
		GOID_PLAYER_WILSON,
		true  
	);
	
	// Willow ĳ���� �߰� (�Ҿ��� ���� Ŭ���� �� �ر�)
	m_characterList.emplace_back(
		L"Willow",
		L"../Resource/Objects/Player/Willson/willow_portrait.png",
		L"../Resource/Objects/Player/Willson/willow_character.png",
		L"���� �������Դϴ�.\n���� �ٷ�� �ɷ��� �پ�ϴ�.\n\n�ر� ����: �Ҿ��� ���� Ŭ����",
		startX + spacing,
		characterY,
		GOID_PLAYER_WILLOW,
		m_gameProgress.IsCharacterUnlocked(GOID_PLAYER_WILLOW)
	);
	
	// Wolfgang ĳ���� �߰� (�Ź̿��հ� ���� Ŭ���� �� �ر�)
	m_characterList.emplace_back(
		L"Wolfgang",
		L"../Resource/Objects/Player/Willson/wolfgang_portrait.png",
		L"../Resource/Objects/Player/Willson/wolfgang_character.png",
		L"���� ĳ�����Դϴ�.\n������� �������� �� �������ϴ�.\n\n�ر� ����: �Ź̿��հ� ���� Ŭ����",
		startX + spacing * 2,
		characterY,
		GOID_PLAYER_WOLFGANG,
		m_gameProgress.IsCharacterUnlocked(GOID_PLAYER_WOLFGANG)
	);
}

void CharacterSelectScene::CreateUI()
{
	// ȭ�� ũ��
	float screenWidth = static_cast<float>(WINCX);
	float screenHeight = static_cast<float>(WINCY);

	// ��� �̹��� ����
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

	// �ڷΰ��� ��ư ���� (ȭ�� �ϴ�)
	UIButton* backButton = new UIButton(
		static_cast<GameObjectID>(GOID_BACK_BUTTON),
		100.0f,  
		screenHeight / 2.0f + 300.f,
		80.0f,
		100.0f,
		L"../Resource/UI/Button.png",
		L"../Resource/UI/Button_Click.png",
		L"Ÿ��Ʋ ȭ��"
	);
	
	backButton->SetOnClickCallback([this]() {
		OnBackButtonClicked();
	});
	UIManager::GetInstance()->AddUIButton(backButton);

	// ���õ� ĳ���� ��Ʈ����Ʈ (�����ʿ� ��ġ) - �ʱ⿡�� ����
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
	selectedPortrait->SetActive(false);  // �ʱ⿡�� ��Ȱ��ȭ
	UIManager::GetInstance()->AddUIImage(selectedPortrait);

	// ĳ���� ����â (UI4.png) - �ʱ⿡�� ����
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
	characterInfoPanel->SetActive(false);  // �ʱ⿡�� ��Ȱ��ȭ
	UIManager::GetInstance()->AddUIImage(characterInfoPanel);

	// ���� ��ư - �ʱ⿡�� ����
	UIButton* selectButton = new UIButton(
		static_cast<GameObjectID>(GOID_SELECT_BUTTON),
		screenWidth / 2.0f + 200.f,
		screenHeight / 2.0f + 350.0f,
		120.0f,
		50.0f,
		L"../Resource/UI/Select_Bar.png",
		L"../Resource/UI/Select_Bar.png",
		L"����"
	);
	
	selectButton->SetOnClickCallback([this, selectButton]()
	{
		selectButton->SetDisabled(true);
		this->OnSelectButtonClicked();
	});

	selectButton->SetActive(false);  // �ʱ⿡�� ��Ȱ��ȭ
	UIManager::GetInstance()->AddUIButton(selectButton);

	// ��� ��ư - �ʱ⿡�� ����
	UIButton* cancelButton = new UIButton(
		static_cast<GameObjectID>(GOID_CANCEL_SELECTION),
		screenWidth / 2.0f + 450.f,
		screenHeight / 2.0f + 350.0f,
		120.0f,
		50.0f,
		L"../Resource/UI/Select_Bar.png",
		L"../Resource/UI/Select_Bar.png",
		L"���"
	);
	
	cancelButton->SetOnClickCallback([this]() {
		this->OnCancelButtonClicked();
	});
	cancelButton->SetActive(false);  // �ʱ⿡�� ��Ȱ��ȭ
	UIManager::GetInstance()->AddUIButton(cancelButton);
	
	// ĳ���� ��ư�� ����
	CreateCharacterButtons();
}

void CharacterSelectScene::CreateCharacterButtons()
{
	OutputDebugStringW((L"CharacterSelectScene: ĳ���� ��ư ���� ���� - " + std::to_wstring(m_characterList.size()) + L"�� ĳ����\n").c_str());
	
	// ��� ĳ���Ϳ� ���� UI ��ҵ� ����
	for (size_t i = 0; i < m_characterList.size(); ++i) {
		const CharacterInfo& charInfo = m_characterList[i];
		
		OutputDebugStringW((L"CharacterSelectScene: ĳ���� " + std::to_wstring(i) + L" ��ư ���� - " + charInfo.name + L"\n").c_str());
		
		float buttonWidth = 150.0f;
		float buttonHeight = 150.0f;
		
		// HUD ��� ����
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
		
		// ĳ���� �̹��� ����
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
		
		// ��� �������� ���� (�رݵ��� ���� ĳ���Ϳ���)
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
		
		// ĳ���� ��ư ���� (Ŭ�� �̺�Ʈ ó��) - ��� ĳ���Ϳ� ���� ����
		UIButton* characterButton = new UIButton(
			static_cast<GameObjectID>(3002 + i * 10),
			charInfo.buttonPosX,
			charInfo.buttonPosY,
			buttonWidth,
			buttonHeight,
			L"", // �� ��ư
			L"", // �� ��ư
			L""  // �ؽ�Ʈ ����
		);
		
		// ���ٷ� ĳ���� �ε����� ĸó
		int characterIndex = static_cast<int>(i);
		characterButton->SetOnClickCallback([this, characterIndex]() {
			OnCharacterButtonClicked(characterIndex);
		});
		
		UIManager::GetInstance()->AddUIButton(characterButton);
	}
	
	OutputDebugStringW(L"CharacterSelectScene: ĳ���� ��ư ���� �Ϸ�\n");
}

void CharacterSelectScene::Update(float deltaTime)
{
	// �Ŵ����� ������Ʈ
	UpdateManagers(deltaTime);
}

void CharacterSelectScene::LateUpdate()
{
	// �Ŵ����� LateUpdate
	LateUpdateManagers();
}

void CharacterSelectScene::Render()
{
	// �Ŵ����� ������
	RenderManagers();
	
	// ĳ���� ���� �ؽ�Ʈ ������
	if (m_selectedCharacterIndex >= 0) {
		RenderCharacterDescription();
	}
}

void CharacterSelectScene::Release()
{
	// �ؽ�Ʈ ������ ���� ����
	if (m_descriptionFont) {
		delete m_descriptionFont;
		m_descriptionFont = nullptr;
	}
	if (m_descriptionBrush) {
		delete m_descriptionBrush;
		m_descriptionBrush = nullptr;
	}
	
	// CharacterSelectScene���� ����� �Ŵ����� ����
	ReleaseAllManagers();
}

void CharacterSelectScene::InitializeTextRendering()
{
	// ��Ʈ ���� (�⺻ ��Ʈ, ũ�� 16)
	m_descriptionFont = new Gdiplus::Font(L"���� ���", 16.0f, Gdiplus::FontStyleRegular, Gdiplus::UnitPixel);
	
	// �귯�� ���� (������)
	m_descriptionBrush = new Gdiplus::SolidBrush(Gdiplus::Color(255, 0, 0, 0));
}

void CharacterSelectScene::RenderCharacterDescription()
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& selectedChar = m_characterList[m_selectedCharacterIndex];
		
		// ĳ���� ���� �г��� ��ġ�� ũ��
		float panelX = WINCX / 2.0f + 300.0f - 250.0f; // �г� �߽ɿ��� ��������
		float panelY = WINCY / 2.0f + 200.0f - 100.0f; // �г� �߽ɿ��� ����
		float panelWidth = 500.0f;
		float panelHeight = 200.0f;
		
		// �ؽ�Ʈ ������ ���� ��� (�г� ���ο� ����)
		float textX = panelX + 20.0f;
		float textY = panelY + 20.0f;
		float textWidth = panelWidth - 40.0f;
		float textHeight = panelHeight - 40.0f;
		
		// �ؽ�Ʈ ������
		RenderManager::GetInstance()->RenderUIText(
			selectedChar.description,
			m_descriptionFont,
			m_descriptionBrush,
			textX,
			textY,
			textWidth,
			textHeight,
			LAYER_UI_FOREGROUND,
			6.0f  // �ؽ�Ʈ�� �ٸ� UI���� ���� ������
		);
	}
}

void CharacterSelectScene::UpdateCharacterSelection()
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& selectedChar = m_characterList[m_selectedCharacterIndex];
		
		// ���õ� ĳ������ ��Ʈ����Ʈ �̹��� ����
		UIImage* selectedPortrait = UIManager::GetInstance()->FindUIImage(GOID_PLAYER_PORTRAIT);
		if (selectedPortrait)
		{
			// ��� ĳ���ʹ� lock.png, �رݵ� ĳ���ʹ� �ش� ��Ʈ����Ʈ ���
			std::wstring portraitPath;
			if (!selectedChar.isUnlocked) 
			{
				portraitPath = L"../Resource/UI/locked.png";
			} else {
				portraitPath = selectedChar.portraitPath;
			}
			
			// UIImage�� LoadBitmap �Լ��� ����ؼ� ��Ʈ����Ʈ �̹��� ����
			selectedPortrait->LoadBitmap(portraitPath);
		}
	}
}

std::wstring CharacterSelectScene::GetSelectedCharacterName() const
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		return m_characterList[m_selectedCharacterIndex].name;
	}
	return L"UnKnown"; // �⺻��
}

GameObjectID CharacterSelectScene::GetSelectedCharacterID() const
{
	if (m_selectedCharacterIndex >= 0 && m_selectedCharacterIndex < static_cast<int>(m_characterList.size())) {
		return m_characterList[m_selectedCharacterIndex].characterID;
	}
	return GOID_NONE; // �⺻��
}

void CharacterSelectScene::OnCharacterButtonClicked(int characterIndex)
{
	if (characterIndex >= 0 && characterIndex < static_cast<int>(m_characterList.size())) {
		const CharacterInfo& charInfo = m_characterList[characterIndex];
		
		OutputDebugStringW((L"Character Selected: " + charInfo.name + L"\n").c_str());
		
		m_selectedCharacterIndex = characterIndex;
		m_isLockedCharacterSelected = !charInfo.isUnlocked;
		m_currentState = CharacterSelectionState::CHARACTER_INFO;
		
		// UI ���̱�
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
		
		// ��� �� CONFIRM_SELECT ���·� ��ȯ
		m_currentState = CharacterSelectionState::CONFIRM_SELECT;
		
		// ���� ��ư ���� ������Ʈ
		UpdateSelectButtonState();
	}
}

void CharacterSelectScene::OnSelectButtonClicked()
{
	if (m_selectedCharacterIndex == -1) {
		OutputDebugStringW(L"No character selected!\n");
		return;
	}
	
	// ��� ĳ���ʹ� ������ �� ����
	if (m_isLockedCharacterSelected) {
		OutputDebugStringW(L"Cannot select locked character!\n");
		return;
	}
	
	std::wstring selectedCharacterName = GetSelectedCharacterName();
	GameObjectID selectedCharacterID = GetSelectedCharacterID();
	OutputDebugStringW((L"Character Confirmed! Loading Game Scene with: " + selectedCharacterName + L" (ID: " + std::to_wstring(selectedCharacterID) + L")\n").c_str());
	
	m_currentState = CharacterSelectionState::CLICK_GAME;
	// ���õ� ĳ���� ������ SceneManager�� �����Ͽ� ���� ������ ��ȯ
	SceneManager::GetInstance()->LoadGameScene(L"../MapData/00_map.dsm", selectedCharacterID);
}

void CharacterSelectScene::OnCancelButtonClicked()
{
	OutputDebugStringW(L"Character Selection Cancelled!\n");
	
	// ĳ���� ���� �ʱ�ȭ, ����¡ ���·� ���ư���
	m_selectedCharacterIndex = -1;
	m_isLockedCharacterSelected = false;
	m_isSelectButtonDisabled = false;
	m_currentState = CharacterSelectionState::BROWSING;
	
	// UI �����
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
	
	// ���� ��ư ���� �ʱ�ȭ
	UpdateSelectButtonState();
}

void CharacterSelectScene::OnBackButtonClicked()
{
	OutputDebugStringW(L"Back button clicked! Returning to Title Scene\n");
	
	// UI �����
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
	
	// Ÿ��Ʋ ������ ���ư���
	SceneManager::GetInstance()->ReturnToTitle();
}

void CharacterSelectScene::UpdateSelectButtonState()
{
	// ��� ĳ���Ͱ� ���õǾ����� ���� ��ư ��Ȱ��ȭ
	m_isSelectButtonDisabled = m_isLockedCharacterSelected;
	
	UIButton* selectButton = UIManager::GetInstance()->FindUIButton(GOID_SELECT_BUTTON);
	if (selectButton) {
		if (m_isSelectButtonDisabled) {
			// ��ư�� ��Ȱ��ȭ ���·� ����
			selectButton->SetDisabled(true);
		} else {
			// ��ư�� Ȱ��ȭ ���·� ����
			selectButton->SetDisabled(false);
		}
	}
}

void CharacterSelectScene::UpdateCharacterUnlockStatus()
{
	// ���� ���� ��Ȳ�� ���� ĳ���� �ر� ���� ������Ʈ
	for (auto& charInfo : m_characterList) {
		charInfo.isUnlocked = m_gameProgress.IsCharacterUnlocked(charInfo.characterID);
	}
	
	// UI ��ҵ� �����
	CreateUI();
}

void CharacterSelectScene::UpdateManagers(float deltaTime)
{
	// CharacterSelectScene������ UIManager�� InputManager ������Ʈ
	UIManager::GetInstance()->Update(deltaTime);
	InputManager::GetInstance()->Update(deltaTime);
}

void CharacterSelectScene::LateUpdateManagers()
{
	// CharacterSelectScene������ UIManager�� InputManager LateUpdate
	UIManager::GetInstance()->LateUpdate();
	InputManager::GetInstance()->LateUpdate();
}

void CharacterSelectScene::RenderManagers()
{
	// CharacterSelectScene������ UIManager�� InputManager ������
	UIManager::GetInstance()->Render();
	InputManager::GetInstance()->Render();
}

void CharacterSelectScene::ReleaseManagers()
{
	// CharacterSelectScene������ UIManager�� ����
	UIManager::GetInstance()->Release();
}

void CharacterSelectScene::InitializeManagers()
{
	OutputDebugStringW(L"CharacterSelectScene: �Ŵ��� �ʱ�ȭ ����\n");
	
	// CharacterSelectScene������ UIManager�� InputManager�� �ʱ�ȭ
	UIManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	
	OutputDebugStringW(L"CharacterSelectScene: �Ŵ��� �ʱ�ȭ �Ϸ�\n");
}

void CharacterSelectScene::ReleaseAllManagers()
{
	OutputDebugStringW(L"CharacterSelectScene: �Ŵ��� ���� ����\n");
	
	// CharacterSelectScene���� ����� �Ŵ����� ����
	InputManager::GetInstance()->Release();
	UIManager::GetInstance()->Release();
	
	OutputDebugStringW(L"CharacterSelectScene: �Ŵ��� ���� �Ϸ�\n");
}
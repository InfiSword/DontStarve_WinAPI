#include "../../99_Default/pch.h"
#include "GameScene.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../CameraManager/CameraManager.h"
#include "../RenderManager/RenderManager.h"
#include "../InventoryManager/InventoryManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/UI/UIImage.h"
#include "../../02_GameObject/UI/UIButton.h"

GameScene::GameScene() : m_selectedCharacterID(GOID_NONE)
{
}

GameScene::~GameScene()
{
	Release();
}

void GameScene::Init()
{
	// GameScene에 필요한 매니저들 초기화
	InitializeManagers();

	// UI 생성
	CreateUI();

	// 게임 진행 정보 로드
	LoadGameProgress();
}

void GameScene::Init(const MapData& mapData)
{
	// 맵 데이터 저장
	m_mapData = mapData;

	// 기본 초기화
	Init();

	// 맵 데이터의 게임오브젝트들을 생성
	CreateGameObjectsFromMapData();

	// 플레이어 생성
	SpawnPlayer();

	ObjectManager::GetInstance()->InitializeObjects();
}


void GameScene::CreateUI()
{
	// 게임 씬에서 필요한 UI 요소들 생성
	// 예: 인벤토리 UI, 상태 표시 UI 등

	// 화면 크기
	float screenWidth = static_cast<float>(WINCX);
	float screenHeight = static_cast<float>(WINCY);

	//OutputDebugStringW(L"GameScene: UI 생성 시작\n");

	// 게임 배경 이미지 추가
	//UIImage* backgroundImage = new UIImage(
	//	static_cast<GameObjectID>(GOID_GAME_BACKGROUND),
	//	screenWidth / 2.0f,
	//	screenHeight / 2.0f,
	//	screenWidth,
	//	screenHeight,
	//	LAYER_UI_BACKGROUND,
	//	L"../Resource/UI/game_background.png",
	//	0.f
	//);
	//UIManager::GetInstance()->AddUIImage(backgroundImage);
	//OutputDebugStringW(L"GameScene: 게임 배경 UI 추가 완료\n");

	//// 게임 상태 표시 UI (예: 체력, 배고픔 등)
	//UIImage* statusUI = new UIImage(
	//	static_cast<GameObjectID>(GOID_STATUS_UI),
	//	screenWidth - 100.0f,
	//	100.0f,
	//	200.0f,
	//	100.0f,
	//	LAYER_UI_FOREGROUND,
	//	L"../Resource/UI/status_panel.png",
	//	0.f
	//);
	//UIManager::GetInstance()->AddUIImage(statusUI);
	//OutputDebugStringW(L"GameScene: 상태 UI 추가 완료\n");
	//
	//// 일시정지 버튼
	//UIButton* pauseButton = new UIButton(
	//	static_cast<GameObjectID>(GOID_PAUSE_BUTTON),
	//	screenWidth - 50.0f,
	//	50.0f,
	//	40.0f,
	//	40.0f,
	//	L"../Resource/UI/pause_button.png",
	//	L"../Resource/UI/pause_button_hover.png",
	//	L"일시정지"
	//);
	//
	//pauseButton->SetOnClickCallback([this]() {
	//	// 일시정지 로직
	//	OutputDebugStringW(L"GameScene: 일시정지 버튼 클릭\n");
	//});
	//UIManager::GetInstance()->AddUIButton(pauseButton);
	//OutputDebugStringW(L"GameScene: 일시정지 버튼 추가 완료\n");

	//OutputDebugStringW(L"GameScene: UI 생성 완료\n");
}

void GameScene::Update(float deltaTime)
{
	// 매니저들 업데이트
	UpdateManagers(deltaTime);

	// 플레이어 이동 처리 제거 - Player::Update()에서 처리됨
	
	// 테두리 표시 토글 (B 키)
	InputManager* inputManager = InputManager::GetInstance();
	if (inputManager && inputManager->IsKeyPressed('B')) {
		ObjectManager* objectManager = ObjectManager::GetInstance();
		if (objectManager) {
			objectManager->ToggleBoundsDisplay();
		}
	}
}

void GameScene::LateUpdate()
{
	// 매니저들 LateUpdate
	LateUpdateManagers();
	
	// GameScene의 특별한 LateUpdate 로직
}

void GameScene::Render()
{
	// 매니저들 렌더링
	RenderManagers();
}

void GameScene::Release()
{
	// GameScene에서 사용한 매니저들 해제
	ReleaseAllManagers();
}

void GameScene::UpdateManagers(float deltaTime)
{
	// GameScene에서는 게임에 필요한 모든 매니저들을 업데이트
	UIManager::GetInstance()->Update(deltaTime);
	InputManager::GetInstance()->Update(deltaTime);
	ObjectManager::GetInstance()->Update(deltaTime);
	CameraManager::GetInstance()->Update(deltaTime);
	RenderManager::GetInstance()->Update(deltaTime);
	InventoryManager::GetInstance()->Update(deltaTime);
}

void GameScene::LateUpdateManagers()
{
	// GameScene에서는 게임에 필요한 모든 매니저들을 LateUpdate
	UIManager::GetInstance()->LateUpdate();
	InputManager::GetInstance()->LateUpdate();
	ObjectManager::GetInstance()->LateUpdate();
	CameraManager::GetInstance()->LateUpdate();
	InventoryManager::GetInstance()->LateUpdate();
}

void GameScene::RenderManagers()
{
	// GameScene에서는 게임에 필요한 모든 매니저들을 렌더링
	// 렌더링 순서: 타일 -> 게임 오브젝트 -> UI -> 인벤토리
	
	// 1. CameraManager (타일 렌더링)
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (cameraManager && renderManager) {
		// 맵 데이터가 있는 경우에만 타일 렌더링
		if (m_mapData.mapWidth > 0 && m_mapData.mapHeight > 0) {
			cameraManager->RenderVisibleTiles(renderManager, &m_mapData);
		}
	}
	
	// 2. RenderManager (게임 오브젝트들 렌더링)
	if (renderManager) {
		renderManager->Render();
	}
	
	// 2. InputManager (입력 관련 렌더링)
	InputManager* inputManager = InputManager::GetInstance();
	if (inputManager) {
		inputManager->Render();
	} else {
		OutputDebugStringW(L"GameScene: InputManager가 null입니다.\n");
	}
	
	// 3. UIManager (UI 요소들 렌더링)
	UIManager* uiManager = UIManager::GetInstance();
	if (uiManager) {
		uiManager->Render();
	}
	
	// 4. InventoryManager (인벤토리 UI 렌더링)
	InventoryManager* inventoryManager = InventoryManager::GetInstance();
	if (inventoryManager) {
		inventoryManager->Render();
	}
}

void GameScene::ReleaseManagers()
{
	// GameScene에서는 게임에 필요한 모든 매니저들을 해제
	UIManager::GetInstance()->Release();
	InventoryManager::GetInstance()->Release();
}

void GameScene::CreateGameObjectsFromMapData()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) {
		return;
	}

	int createdCount = 0;
	for (const auto& objData : m_mapData.gameObjects) {
		// 맵 파일의 콜라이더 정보를 포함한 resourceData 생성
		GameObjectData resourceData;
		resourceData.id = objData.id;
		resourceData.type = objData.type;
		resourceData.x = objData.x;
		resourceData.y = objData.y;
		resourceData.objectAssetBaseDirectory = objData.objectAssetBaseDirectory;
		resourceData.assetImageName = objData.assetImageName;
		resourceData.pivotX = objData.pivotX;
		resourceData.pivotY = objData.pivotY;
		resourceData.hasCollider = objData.hasCollider;
		resourceData.colliderOffsetX = objData.colliderOffsetX;
		resourceData.colliderOffsetY = objData.colliderOffsetY;
		resourceData.colliderWidth = objData.colliderWidth;
		resourceData.colliderHeight = objData.colliderHeight;
		
		// 리소스 데이터와 함께 게임 오브젝트 생성
		GameObject* obj = objectManager->CreateGameObject(objData.id, objData.x, objData.y, &resourceData);
		if (obj) {
			createdCount++;
		}
	}


}

void GameScene::SpawnPlayer()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) {
		return;
	}

	// 플레이어 생성 (ID 기반)
	GameObject* player = objectManager->CreateGameObject(m_selectedCharacterID, m_mapData.playerSpawn.x, m_mapData.playerSpawn.y);

	if (player) {
		// ObjectManager에서 캐싱된 플레이어 가져오기
		Player* cachedPlayer = objectManager->GetPlayer();
		if (cachedPlayer) {
			// 플레이어를 카메라의 타겟으로 설정
			CameraManager* cameraManager = CameraManager::GetInstance();
			if (cameraManager) {
				cameraManager->SetTarget(cachedPlayer);
				cameraManager->SetFollowMode(true);
			}
		}
	}
}


void GameScene::ClearScene(SceneType sceneType)
{
	// 씬 클리어 로직
	//m_gameProgress.SetSceneCleared(sceneType, true);
	SaveGameProgress();
}

bool GameScene::IsSceneCleared(SceneType sceneType) const
{
	return m_gameProgress.IsSceneCleared(sceneType);
}

bool GameScene::CheckSceneClearCondition(SceneType sceneType) const
{
	// 씬 클리어 조건 확인 로직
	switch (sceneType) {
	case SCENE_GAME_FARMING_AREA:
		// 농장 지역 클리어 조건: 특정 아이템 수집 등
		return true; // 임시로 항상 클리어된 것으로 처리
	default:
		return false;
	}
}

void GameScene::SaveGameProgress()
{
	// 게임 진행 정보 저장
	//m_gameProgress.SaveToFile(L"game_progress.txt");
}

void GameScene::LoadGameProgress()
{
	// 게임 진행 정보 로드
	//m_gameProgress.LoadFromFile(L"game_progress.txt");
}

void GameScene::InitializeManagers()
{
	// GameScene에서는 게임에 필요한 모든 매니저들을 초기화
	UIManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	ObjectManager::GetInstance()->Init();
	CameraManager::GetInstance()->Init();
	RenderManager::GetInstance()->Init();
	InventoryManager::GetInstance()->Init();
	ResourceManager::GetInstance()->Init();
}

void GameScene::ReleaseAllManagers()
{
	// GameScene에서 사용한 매니저들 해제
	InventoryManager::GetInstance()->Release();
	RenderManager::GetInstance()->Release();
	CameraManager::GetInstance()->Release();
	ObjectManager::GetInstance()->Release();
	InputManager::GetInstance()->Release();
	UIManager::GetInstance()->Release();
}
#include "99_Default/pch.h"
#include "GameScene.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../CameraManager/CameraManager.h"
#include "../RenderManager/RenderManager.h"
#include "../InventoryManager/InventoryManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../ColliderManager/ColliderManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"

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
	UIManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();
	ObjectManager::GetInstance()->Init();
	CameraManager::GetInstance()->Init();
	RenderManager::GetInstance()->Init();
	InventoryManager::GetInstance()->Init();
	ResourceManager::GetInstance()->Init();

	// UI 생성
	// CreateUI();

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

void GameScene::Update(float deltaTime)
{
	// 매니저들 업데이트
	UIManager::GetInstance()->Update(deltaTime);
	InputManager::GetInstance()->Update(deltaTime);
	ObjectManager::GetInstance()->Update(deltaTime);
	CameraManager::GetInstance()->Update(deltaTime);
	RenderManager::GetInstance()->Update(deltaTime);
	InventoryManager::GetInstance()->Update(deltaTime);

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
	UIManager::GetInstance()->LateUpdate();
	InputManager::GetInstance()->LateUpdate();
	ObjectManager::GetInstance()->LateUpdate();
	InventoryManager::GetInstance()->LateUpdate();	
}

void GameScene::Render()
{
	// 매니저들 렌더링
	// 렌더링 순서: 타일 -> (월드) 게임 오브젝트 -> 디버그 기즈모 -> UI -> 인벤토리
	
	// 1. CameraManager (타일 렌더링)
	CameraManager* cameraManager = CameraManager::GetInstance();
	if (cameraManager) {
		// 맵 데이터가 있는 경우에만 타일 렌더링
		if (m_mapData.mapWidth > 0 && m_mapData.mapHeight > 0) {
			cameraManager->RenderVisibleTiles(&m_mapData);
		}
	}
	
	// 2. ObjectManager (월드 오브젝트 렌더링 + 디버그 바운드)
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (objectManager) {
		objectManager->Render();
	}
	
	// 3. 디버그용 콜라이더 Gizmo 렌더링
	ColliderManager* colliderManager = ColliderManager::GetInstance();
	if (colliderManager) {
		colliderManager->RenderGizmos();
	}
	
	// 4. InputManager (입력 관련 렌더링)
	InputManager* inputManager = InputManager::GetInstance();
	if (inputManager) {
		inputManager->Render();
	} else {
		OutputDebugStringW(L"GameScene: InputManager가 null입니다.\n");
	}
	
	// 5. UIManager (UI 요소들 렌더링)
	UIManager* uiManager = UIManager::GetInstance();
	if (uiManager) {
		uiManager->Render();
	}
	
	// 6. InventoryManager (인벤토리 UI 렌더링)
	InventoryManager* inventoryManager = InventoryManager::GetInstance();
	if (inventoryManager) {
		inventoryManager->Render();
	}
}

void GameScene::Release()
{
	// GameScene에서 사용한 매니저/포인터 정리 (소멸자에서 호출)
	InventoryManager::GetInstance()->Release();
	RenderManager::GetInstance()->Release();
	CameraManager::GetInstance()->Release();
	ObjectManager::GetInstance()->Release();
	InputManager::GetInstance()->Release();
	UIManager::GetInstance()->Release();
}

void GameScene::CreateGameObjectsFromMapData()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) {
		return;
	}

	int createdCount = 0;
	for (const GameObjectData& objData : m_mapData.gameObjects) 
	{
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
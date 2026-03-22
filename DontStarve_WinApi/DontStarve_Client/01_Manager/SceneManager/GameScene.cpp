#include "99_Default/pch.h"
#include "GameScene.h"
#include "../InputManager/InputManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../CameraManager/CameraManager.h"
#include "../RenderManager/RenderManager.h"
#include "../InventoryManager/InventoryManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../ColliderManager/ColliderManager.h"
#include "../GameProgressManager/GameProgressManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/UI/CraftingUI.h"
#include "../../02_GameObject/UI/HPUI.h"
#include "../../02_GameObject/UI/GameOverUI.h"

GameScene::GameScene()
	: m_mapData(nullptr)
	, m_hasWalkableBounds(false)
	, m_walkableMinX(0.0f)
	, m_walkableMinY(0.0f)
	, m_walkableMaxX(0.0f)
	, m_walkableMaxY(0.0f)
	, m_selectedCharacterID(GOID_NONE)
	, m_craftingUI(nullptr)
	, m_playerHPUI(nullptr)
	, m_gameOverUI(nullptr)
{
}

GameScene::~GameScene()
{
	Release();
}

void GameScene::Init(const MapData* mapData)
{
	// 1. 매니저들 기초 초기화
	ObjectManager::GetInstance()->Init();
	CameraManager::GetInstance()->Init();
	InventoryManager::GetInstance()->Init();
	ColliderManager::GetInstance()->Init();
	RenderManager::GetInstance()->Init();
	InputManager::GetInstance()->Init();

	// 3. 맵 데이터 처리
	m_mapData = mapData;
	m_hasWalkableBounds = false;

	if (m_mapData && m_mapData->mapWidth > 0 && m_mapData->mapHeight > 0)
	{
		float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
		int count = 0;
		for (int y = 0; y < m_mapData->mapHeight; ++y)
		{
			for (int x = 0; x < m_mapData->mapWidth; ++x)
			{
				if (!m_mapData->walkableAreas[x][y]) continue;

				float wx = static_cast<float>(x) * TILE_SIZE + TILE_SIZE * 0.5f;
				float wy = static_cast<float>(y) * TILE_SIZE + TILE_SIZE * 0.5f;

				if (wx < minX) minX = wx;
				if (wy < minY) minY = wy;
				if (wx > maxX) maxX = wx;
				if (wy > maxY) maxY = wy;
				++count;
			}
		}

		if (count > 0)
		{
			m_hasWalkableBounds = true;
			m_walkableMinX = minX;
			m_walkableMinY = minY;
			m_walkableMaxX = maxX;
			m_walkableMaxY = maxY;

			CameraManager* cameraManager = CameraManager::GetInstance();
			if (cameraManager)
				cameraManager->SetWalkableBounds(m_walkableMinX, m_walkableMinY, m_walkableMaxX, m_walkableMaxY);
		}
	}

	// 4. 오브젝트 및 플레이어 생성 (UI 초기화보다 먼저 호출하여 HPUI에 플레이어 정보를 넘김)
	CreateGameObjectsFromMapData();
	SpawnPlayer();

	// 5. UI 생성 및 초기화 (보스 씬 제외)
	// 보스 씬은 CraftingUI를 표시하지 않음
	SceneType currentSceneType = GetSceneType();
	bool isBossScene = (currentSceneType == SCENE_GAME_HOUND_FOREST || 
	                     currentSceneType == SCENE_GAME_SPIDER_QUEEN_HOUSE);

	if (!isBossScene) {
		if (!m_craftingUI) m_craftingUI = new MenuUI();
		if (m_craftingUI) {
			m_craftingUI->Init();
			ObjectManager::GetInstance()->AddGameObject(m_craftingUI);
		}
	}

	if (!m_playerHPUI) {
		Player* player = ObjectManager::GetInstance()->GetPlayer();
		m_playerHPUI = new HPUI(player, L"", 200.0f, 28.0f,
            Gdiplus::Color(255, 60, 0, 0), Gdiplus::Color(255, 255, 0, 0), Gdiplus::Color(255, 255, 255, 255),
            1.0f, 0.0f, 1.0f, 0.0f, -278.0f, 34.0f,
            10.1f, 10.2f, true, true);
	}
	if (m_playerHPUI) {
		m_playerHPUI->Init();
		ObjectManager::GetInstance()->AddGameObject(m_playerHPUI);
	}

	if (!m_gameOverUI) m_gameOverUI = new GameOverUI();
	if (m_gameOverUI) {
		m_gameOverUI->Init();
		ObjectManager::GetInstance()->AddGameObject(m_gameOverUI);
	}
}

void GameScene::Update(float deltaTime)
{
	// 매니저들 업데이트 (InputManager는 메인 루프에서 가장 먼저 업데이트됨)
	ObjectManager::GetInstance()->Update(deltaTime);
	CameraManager::GetInstance()->Update(deltaTime);
	RenderManager::GetInstance()->Update(deltaTime);
	InventoryManager::GetInstance()->Update(deltaTime);
}

void GameScene::LateUpdate()
{
	// 매니저들 LateUpdate
	// InputManager::LateUpdate는 메인 루프에서 처리됨
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
		if (m_mapData && m_mapData->mapWidth > 0 && m_mapData->mapHeight > 0) {
			cameraManager->RenderVisibleTiles(m_mapData);
		}
	}
	
	// 2. ObjectManager (월드 오브젝트 렌더링 + 디버그 바운드 + UI 렌더링)
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
	}
	
	// 6. InventoryManager (인벤토리 UI 렌더링)
	InventoryManager* inventoryManager = InventoryManager::GetInstance();
	if (inventoryManager) {
		inventoryManager->Render();
	}
}

void GameScene::Release()
{
	// UI 해제
	if (m_playerHPUI) {
		m_playerHPUI->Release();
		delete m_playerHPUI;
		m_playerHPUI = nullptr;
	}

	if (m_gameOverUI) {
		m_gameOverUI->Release();
		delete m_gameOverUI;
		m_gameOverUI = nullptr;
	}

	if (m_craftingUI) {
		m_craftingUI->Release();
		delete m_craftingUI;
		m_craftingUI = nullptr;
	}

	// GameScene에서 사용한 매니저/포인터 정리 (소멸자에서 호출)
	ColliderManager::GetInstance()->Release();
	InventoryManager::GetInstance()->Release();
	RenderManager::GetInstance()->Release();
	CameraManager::GetInstance()->Release();
	ObjectManager::GetInstance()->Release();
	InputManager::GetInstance()->Release();
	
	m_mapData = nullptr;
}

void GameScene::CreateGameObjectsFromMapData()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager || !m_mapData) {
		return;
	}

	for (const ResourcePathUtils::ObjectResourceDef& objData : m_mapData->gameObjects)
	{
		objectManager->CreateGameObject(objData.id, objData.x, objData.y, nullptr);
	}
}

void GameScene::SpawnPlayer()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) {
		return;
	}

	GameObject* player = objectManager->CreateGameObject(m_selectedCharacterID, m_mapData->playerSpawn.x, m_mapData->playerSpawn.y);

	if (player) {
		Player* cachedPlayer = objectManager->GetPlayer();
		if (cachedPlayer) {
			CameraManager* cameraManager = CameraManager::GetInstance();
			if (cameraManager) {
				cameraManager->SetTarget(cachedPlayer);
				cameraManager->SetFollowMode(true);
			}
		}
	}
}

void GameScene::SaveCurrentObjectsToMapData(MapData& outMapData)
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;

	const auto& gameObjects = objectManager->GetGameObjects();

	// 기존 초기 맵 오브젝트 리스트를 클리어하고 현재 활성화된 상태로 갱신
	outMapData.gameObjects.clear();

	for (GameObject* obj : gameObjects)
	{
		if (!obj || !obj->IsEnabled()) continue;

		GameObjectID id = obj->GetID();

		// 플레이어나 UI 등 영구 상태 저장이 필요 없는 특수 오브젝트는 제외 (ID 범위 Wilson ~ Wolfgang, UI 등)
		if (id >= 1000 && id < 2000) continue; // Player
		if (id >= 3000) continue;              // UI

		Transform* transform = obj->GetComponent<Transform>();
		if (!transform) continue;

		ResourcePathUtils::ObjectResourceDef objDef;
		objDef.id = id;
		objDef.x = transform->GetX();
		objDef.y = transform->GetY();

		outMapData.gameObjects.push_back(objDef);
	}
}


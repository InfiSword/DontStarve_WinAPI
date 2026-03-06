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
#include "../GameProgressManager/GameProgressManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"
#include "../../02_GameObject/UI/CraftingUI.h"
#include "../../02_GameObject/UI/PlayerHPUI.h"

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
{
}

GameScene::~GameScene()
{
	Release();
}

void GameScene::Init()
{
	// GameScene에 필요한 매니저들 초기화
	// 주의: 일부 매니저는 이미 MainGame::Init()에서 초기화되었을 수 있음
	// 중복 초기화를 피하기 위해 필요한 경우에만 초기화
	
	// UI 매니저는 씬마다 초기화 필요 (UI 리스트 클리어)
	UIManager::GetInstance()->Init();
	
	// ObjectManager 초기화 (게임 오브젝트 관리)
	ObjectManager::GetInstance()->Init();
	
	// CameraManager 초기화 (카메라 위치 등)
	CameraManager::GetInstance()->Init();
	
	// InventoryManager 초기화
	InventoryManager::GetInstance()->Init();

	// ColliderManager 초기화 (콜라이더 목록 등, 씬별로 정리 후 사용)
	ColliderManager::GetInstance()->Init();

	// 크래프팅 UI 생성
	m_craftingUI = new CraftingUI();
	if (m_craftingUI) {
		m_craftingUI->Init();
		// CraftingUI는 내부적으로 필요한 UI 요소들을 UIManager에 추가함
	}

	// 플레이어 HP UI 생성 (우측 상단 게이지 + Game Over 패널)
	m_playerHPUI = new PlayerHPUI();
	if (m_playerHPUI) {
		m_playerHPUI->Init();
	}

	// GameProgressManager 초기화 (게임 진행도 로드)
	GameProgressManager::GetInstance()->Init();
}

void GameScene::Init(const MapData* mapData)
{
	// 맵 데이터 참조만 저장 (복사 없음, SceneManager가 소유)
	m_mapData = mapData;
	m_hasWalkableBounds = false;

	// 기본 초기화
	Init();

	// Walkable 경계 계산 (맵 데이터 기준, 월드 좌표)
	if (m_mapData && m_mapData->mapWidth > 0 && m_mapData->mapHeight > 0)
	{
		float minX = 1e9f, minY = 1e9f, maxX = -1e9f, maxY = -1e9f;
		int count = 0;
		for (int y = 0; y < m_mapData->mapHeight; ++y)
		{
			for (int x = 0; x < m_mapData->mapWidth; ++x)
			{
				if (!m_mapData->walkableAreas[x][y])
					continue;

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

			// Walkable 영역으로 카메라 이동 제한 설정 (에디터에서 설정한 이동 가능 구역만큼만 카메라 이동)
			CameraManager* cameraManager = CameraManager::GetInstance();
			if (cameraManager)
			{
				cameraManager->SetWalkableBounds(m_walkableMinX, m_walkableMinY, m_walkableMaxX, m_walkableMaxY);
			}
		}
	}

	// 맵 데이터의 게임오브젝트들을 생성
	CreateGameObjectsFromMapData();

	// 플레이어 생성
	SpawnPlayer();
}

void GameScene::Update(float deltaTime)
{
	// 매니저들 업데이트 (InputManager는 메인 루프에서 가장 먼저 업데이트됨)
	UIManager::GetInstance()->Update(deltaTime);
	ObjectManager::GetInstance()->Update(deltaTime);
	CameraManager::GetInstance()->Update(deltaTime);
	RenderManager::GetInstance()->Update(deltaTime);
	InventoryManager::GetInstance()->Update(deltaTime);
	if (m_playerHPUI) {
		m_playerHPUI->Update(deltaTime);
	}
}

void GameScene::LateUpdate()
{
	// 매니저들 LateUpdate
	// InputManager::LateUpdate는 메인 루프에서 처리됨
	UIManager::GetInstance()->LateUpdate();
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
	
	// 5-2. 플레이어 HP UI (우측 상단 게이지 + Game Over 시 블록)
	if (m_playerHPUI) {
		m_playerHPUI->Render();
	}
	
	// 6. InventoryManager (인벤토리 UI 렌더링)
	InventoryManager* inventoryManager = InventoryManager::GetInstance();
	if (inventoryManager) {
		inventoryManager->Render();
	}
}

void GameScene::Release()
{
	// 플레이어 HP UI 해제
	if (m_playerHPUI) {
		m_playerHPUI->Release();
		delete m_playerHPUI;
		m_playerHPUI = nullptr;
	}

	// 크래프팅 UI 해제
	if (m_craftingUI) {
		m_craftingUI->Release();
		delete m_craftingUI;
		m_craftingUI = nullptr;
	}

	// GameScene에서 사용한 매니저/포인터 정리 (소멸자에서 호출)
	// 콜라이더 목록을 먼저 비워서 오브젝트 해제 시 댕글링 포인터 방지
	ColliderManager::GetInstance()->Release();
	InventoryManager::GetInstance()->Release();
	RenderManager::GetInstance()->Release();
	CameraManager::GetInstance()->Release();
	ObjectManager::GetInstance()->Release();
	InputManager::GetInstance()->Release();
	UIManager::GetInstance()->Release();
	
	// MapData는 SceneManager가 소유하므로 여기서는 포인터만 해제
	m_mapData = nullptr;
}

void GameScene::CreateGameObjectsFromMapData()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager || !m_mapData) {
		return;
	}

	int createdCount = 0;
	for (const ResourcePathUtils::ObjectResourceDef& objData : m_mapData->gameObjects)
	{
		// 맵에는 배치(type, id, x, y)만 있음. 오브젝트 정보는 ObjectManager가 ResourceManager에서 조회
		GameObject* obj = objectManager->CreateGameObject(objData.id, objData.x, objData.y, nullptr);
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
	GameObject* player = objectManager->CreateGameObject(m_selectedCharacterID, m_mapData->playerSpawn.x, m_mapData->playerSpawn.y);

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
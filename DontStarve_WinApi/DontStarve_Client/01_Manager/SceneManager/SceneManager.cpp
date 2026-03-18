#include "99_Default/pch.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "GameScene.h"
#include "ForestScene.h"
#include "BossHoundScene.h"
#include "BossSpiderQueenScene.h"
#include "../GameProgressManager/GameProgressManager.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"
#include "../CameraManager/CameraManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../RenderManager/RenderManager.h"
#include "../InventoryManager/InventoryManager.h"
#include "../ColliderManager/ColliderManager.h"
#include "../ResourceManager/ResourceManager.h"
#include "../../02_GameObject/Entity/Player/Player.h"

SceneManager::SceneManager()
	: m_currentScene(nullptr)
	, m_nextScene(nullptr)
	, m_currentMapData(nullptr)
{
}

SceneManager::~SceneManager()
{
	Release();
}

void SceneManager::Init()
{
	LoadAllMapData();
	LoadTitleScene();
}

void SceneManager::Update(float deltaTime)
{
	// 프레임 시작 시 예약된 씬이 있다면 교체 
	ChangeSceneIfReserved();

	if (m_currentScene) {
		m_currentScene->Update(deltaTime);
	}
}

void SceneManager::LateUpdate()
{
	if (m_currentScene) {
		m_currentScene->LateUpdate();
	}
}

void SceneManager::Render()
{
	if (m_currentScene) {
		m_currentScene->Render();
	}
}

void SceneManager::Release()
{
	if (m_currentScene) {
		delete m_currentScene;
		m_currentScene = nullptr;
	}
	if (m_nextScene) {
		delete m_nextScene;
		m_nextScene = nullptr;
	}
	
	for (auto& pair : m_mapDataStorage) {
		MapData& mapData = pair.second;
		for (auto& objDef : mapData.gameObjects) {
			objDef.baseDir.clear(); objDef.baseDir.shrink_to_fit();
			objDef.imageName.clear(); objDef.imageName.shrink_to_fit();
		}
		mapData.gameObjects.clear(); mapData.gameObjects.shrink_to_fit();
		
		for (int y = 0; y < MAP_HEIGHT; ++y) {
			for (int x = 0; x < MAP_WIDTH; ++x) {
				mapData.tiles[x][y].baseDir.clear(); mapData.tiles[x][y].baseDir.shrink_to_fit();
				mapData.tiles[x][y].imageName.clear(); mapData.tiles[x][y].imageName.shrink_to_fit();
			}
		}
		mapData.mapName.clear(); mapData.mapName.shrink_to_fit();
		mapData.mapFilePath.clear(); mapData.mapFilePath.shrink_to_fit();
	}
	
	m_mapDataStorage.clear();
	m_currentMapData = nullptr;
}

void SceneManager::LoadTitleScene()
{
	if (m_nextScene) return;

	TitleScene* titleScene = new TitleScene();
	m_nextScene = titleScene;
}

void SceneManager::LoadCharacterSelectScene()
{
	if (m_nextScene) return;

	CharacterSelectScene* characterSelectScene = new CharacterSelectScene();
	m_nextScene = characterSelectScene;
}

void SceneManager::LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID)
{
	if (m_nextScene) return;

	// 현재 플레이어 상태 저장
	ObjectManager* objMgr = ObjectManager::GetInstance();
	Player* currentPlayer = objMgr->GetPlayer();
	if (currentPlayer) {
		GameProgressManager::GetInstance()->SavePlayerState(currentPlayer->SaveState());
	}

	GameScene* gameScene = nullptr;
	if (mapFileName == L"GameData/01_BossHound.dsm") {
		gameScene = new BossHoundScene();
	}
	else if (mapFileName == L"GameData/02_BossSpiderQueen.dsm") {
		gameScene = new BossSpiderQueenScene();
	}
	else {
		gameScene = new ForestScene();
	}
	
	gameScene->SetSelectedCharacterID(selectedCharacterID);

	auto it = m_mapDataStorage.find(mapFileName);
	if (it != m_mapDataStorage.end()) {
		m_currentMapData = &it->second;
		// 주의: Init은 ChangeSceneIfReserved에서 새 씬이 활성화될 때 호출함
	}
	else {
		m_currentMapData = nullptr;
	}

	m_nextScene = gameScene;
}

void SceneManager::ChangeSceneIfReserved()
{
	if (!m_nextScene) return;

	OutputDebugStringW(L"SceneManager: 예약된 씬으로 안전하게 교체 수행\n");

	// 1. 기존 씬 삭제
	if (m_currentScene) {
		delete m_currentScene;
		m_currentScene = nullptr;
	}

	// 2. 새 씬 활성화 및 초기화
	m_currentScene = m_nextScene;
	m_nextScene = nullptr;

	// 씬 활성화 시점에 Init 호출 (여기서 MapData 전달)
	m_currentScene->Init(m_currentMapData);
}

void SceneManager::LoadAllMapData()
{
	std::vector<std::wstring> mapFiles = {
		L"GameData/00_map.dsm",
		L"GameData/01_BossHound.dsm",
		L"GameData/02_BossSpiderQueen.dsm",
	};

	for (const std::wstring& mapFileName : mapFiles) {
		MapData mapData;
		if (ParseMapFile(mapFileName, mapData)) {
			m_mapDataStorage[mapFileName] = std::move(mapData);
		}
	}
}

bool SceneManager::ParseMapFile(const std::wstring& mapFileName, MapData& outMapData)
{
	ResourceManager* resMgr = ResourceManager::GetInstance();
	auto getObjectResourceInfo = [resMgr](GameObjectID id) -> const ResourcePathUtils::ObjectResourceDef* {
		return resMgr->GetObjectResourceInfo(id);
	};
	return ResourcePathUtils::ParseMapFileInto(mapFileName, outMapData, getObjectResourceInfo);
}

SceneType SceneManager::GetCurrentSceneType() const
{
	if (!m_currentScene) return SCENE_NONE;
	return m_currentScene->GetSceneType();
}

#include "99_Default/pch.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "GameScene.h"
#include "ForestScene.h"
#include "BossHoundScene.h"
#include "BossSpiderQueenScene.h"
#include "../GameProgressManager/GameProgressManager.h"
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
	// 모든 맵 데이터 로드 (게임 시작 시 한 번만 수행)
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
	
	// m_mapDataStorage 정리 (스택 할당 회피)
	for (auto it = m_mapDataStorage.begin(); it != m_mapDataStorage.end(); ++it) {
		it->second.gameObjects.clear();
		it->second.gameObjects.shrink_to_fit();
		it->second.mapName.clear();
		it->second.mapName.shrink_to_fit();
		it->second.mapFilePath.clear();
		it->second.mapFilePath.shrink_to_fit();
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

void SceneManager::LoadGameScene(SceneType sceneType, GameObjectID selectedCharacterID)
{
	if (m_nextScene) return;

	// 현재 플레이어 상태 저장
	ObjectManager* objMgr = ObjectManager::GetInstance();
	Player* currentPlayer = objMgr->GetPlayer();
	if (currentPlayer) {
		GameProgressManager::GetInstance()->SavePlayerState(currentPlayer->SaveState());
	}

	GameScene* gameScene = nullptr;
	switch (sceneType)
	{
	case SCENE_GAME_HOUND_FOREST:
		gameScene = new BossHoundScene();
		break;
	case SCENE_GAME_SPIDER_QUEEN_HOUSE:
		gameScene = new BossSpiderQueenScene();
		break;
	case SCENE_GAME_FARMING_AREA:
	default:
		gameScene = new ForestScene();
		break;
	}
	
	gameScene->SetSelectedCharacterID(selectedCharacterID);

	// 맵 데이터 설정 (m_mapDataStorage에서 가져옴)
	auto it = m_mapDataStorage.find(sceneType);
	if (it != m_mapDataStorage.end()) {
		m_currentMapData = &it->second;
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

	// 1. 기존 씬 정리
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

SceneType SceneManager::GetCurrentSceneType() const
{
	if (!m_currentScene) return SCENE_NONE;
	return m_currentScene->GetSceneType();
}

void SceneManager::LoadAllMapData()
{
	// 게임 시작 시 모든 맵 데이터 로드
	for (const auto& entry : EnumTables::SceneTypeTable) {
		MapData mapData;
		if (SceneManager::ParseMapFile(entry.path, mapData)) {
			m_mapDataStorage[entry.value] = std::move(mapData);
		}
	}
	OutputDebugStringW(L"SceneManager: 모든 맵 데이터 로드 완료\n");
}

bool SceneManager::ParseMapFile(const std::wstring& mapFileName, MapData& outMapData)
{
	ResourceManager* resMgr = ResourceManager::GetInstance();
	auto getObjectResourceInfo = [resMgr](GameObjectID id) -> const ResourcePathUtils::ObjectResourceDef* {
		return resMgr->GetObjectResourceInfo(id);
	};
	return ResourcePathUtils::ParseMapFileInto(mapFileName, outMapData, getObjectResourceInfo);
}

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
	
	// m_mapDataBackup 정리 (스택 할당 회피)
	for (auto it = m_mapDataBackup.begin(); it != m_mapDataBackup.end(); ++it) {
		it->second.gameObjects.clear();
		it->second.gameObjects.shrink_to_fit();
		it->second.mapName.clear();
		it->second.mapName.shrink_to_fit();
		it->second.mapFilePath.clear();
		it->second.mapFilePath.shrink_to_fit();
	}
	m_mapDataBackup.clear();
	
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

	// 보스 씬으로 진입하는 경우: 파밍 씬 상태 백업
	bool isBossScene = (sceneType == SCENE_GAME_HOUND_FOREST || 
	                     sceneType == SCENE_GAME_SPIDER_QUEEN_HOUSE);
	if (isBossScene) {
		SaveGameSceneState(SCENE_GAME_FARMING_AREA);
	}
	// 파밍 씬으로 복귀하는 경우: 보스 이전 상태 복원
	else if (sceneType == SCENE_GAME_FARMING_AREA) {
		// 현재 씬이 보스 씬인지 확인
		if (m_currentScene) {
			SceneType currentSceneType = GetCurrentSceneType();
			if (currentSceneType == SCENE_GAME_HOUND_FOREST || 
				currentSceneType == SCENE_GAME_SPIDER_QUEEN_HOUSE) {
				// 보스 씬에서 돌아오는 것이므로 백업된 상태 복원
				RestoreGameSceneState(SCENE_GAME_FARMING_AREA);
			}
		}
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

void SceneManager::SaveGameSceneState(SceneType sceneType)
{
	// 현재 씬의 상태를 백업으로 저장
	GameScene* currentGameScene = dynamic_cast<GameScene*>(m_currentScene);
	if (!currentGameScene) return;

	MapData backupData;
	// 게임 씬에서 현재 상태를 추출
	currentGameScene->SaveCurrentObjectsToMapData(backupData);
	m_mapDataBackup[sceneType] = backupData;
	
	OutputDebugStringW((L"SceneManager: 게임 씬 상태 백업 완료 - SceneType: " + std::to_wstring((int)sceneType) + L"\n").c_str());
}

void SceneManager::RestoreGameSceneState(SceneType sceneType)
{
	// 백업된 씬의 상태를 현재 상태로 복원
	auto backupIt = m_mapDataBackup.find(sceneType);
	if (backupIt != m_mapDataBackup.end()) {
		m_currentMapData = &backupIt->second;
		OutputDebugStringW((L"SceneManager: 게임 씬 상태 복원 완료 - SceneType: " + std::to_wstring((int)sceneType) + L"\n").c_str());
	}
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

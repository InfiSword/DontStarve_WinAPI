#include "99_Default/pch.h"
#include "SceneManager.h"
#include "TitleScene.h"
#include "CharacterSelectScene.h"
#include "GameScene.h"
#include "../UIManager/UIManager.h"
#include "../InputManager/InputManager.h"
#include "../CameraManager/CameraManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../RenderManager/RenderManager.h"
#include "../InventoryManager/InventoryManager.h"
#include "../ColliderManager/ColliderManager.h"
#include "../ResourceManager/ResourceManager.h"

SceneManager::SceneManager()
	: m_currentScene(nullptr)
	, m_currentMapData(nullptr)
	, m_pendingScene(PendingSceneType::None)
	, m_pendingCharacterID(GOID_NONE)
{
}

SceneManager::~SceneManager()
{
	Release();
}

void SceneManager::Init()
{
	// 게임 시작 시 모든 맵 데이터 미리 로드 (씬 전환 시 재사용)
	LoadAllMapData();
	
	// 첫 번째 씬 로드 요청 후 즉시 처리
	LoadTitleScene();
	ProcessPendingSceneLoad();
}

void SceneManager::Update(float deltaTime)
{
	if (m_currentScene) {
		m_currentScene->Update(deltaTime);
	}
	
	if (m_pendingScene != PendingSceneType::None) 
		ProcessPendingSceneLoad();
}

void SceneManager::LateUpdate()
{
	// 현재 씬 LateUpdate (해당 씬의 매니저들 LateUpdate)
	if (m_currentScene) {
		m_currentScene->LateUpdate();
	}
}

void SceneManager::Render()
{
	// 현재 씬 렌더링 (해당 씬의 매니저들 렌더링)
	if (m_currentScene) {
		m_currentScene->Render();
	}
}

void SceneManager::Release()
{
	ReleaseCurrentScene();
	
	// 모든 맵 데이터 명시적 정리 (메모리 누수 방지)
	for (auto& pair : m_mapDataStorage) {
		MapData& mapData = pair.second;
		
		// 1. gameObjects 벡터 내부 각 ObjectResourceDef의 문자열 정리
		for (auto& objDef : mapData.gameObjects) {
			objDef.baseDir.clear();
			objDef.baseDir.shrink_to_fit();
			objDef.imageName.clear();
			objDef.imageName.shrink_to_fit();
		}
		mapData.gameObjects.clear();
		mapData.gameObjects.shrink_to_fit();
		
		// 2. tiles 배열의 각 TileResourceDef 문자열 정리 (50x50 = 2500개)
		for (int y = 0; y < MAP_HEIGHT; ++y) {
			for (int x = 0; x < MAP_WIDTH; ++x) {
				mapData.tiles[x][y].baseDir.clear();
				mapData.tiles[x][y].baseDir.shrink_to_fit();
				mapData.tiles[x][y].imageName.clear();
				mapData.tiles[x][y].imageName.shrink_to_fit();
			}
		}
		
		// 3. MapData 자체의 문자열 정리
		mapData.mapName.clear();
		mapData.mapName.shrink_to_fit();
		mapData.mapFilePath.clear();
		mapData.mapFilePath.shrink_to_fit();
	}
	
	// 4. 맵 저장소 전체 해제 (키로 사용된 std::wstring도 해제됨)
	m_mapDataStorage.clear();
	m_currentMapData = nullptr;
}

void SceneManager::ReleaseCurrentScene()
{
	if (!m_currentScene) return;
	OutputDebugStringW(L"SceneManager: 이전 씬 삭제 (소멸자에서 Release 호출)...\n");
	delete m_currentScene;
	m_currentScene = nullptr;
}

void SceneManager::LoadAllMapData()
{
	// 게임 시작 시 모든 맵 파일 로드 (MapData 폴더의 .dsm 파일들)
	// 주의: CharacterSelectScene에서 사용하는 경로와 일치해야 함!
	std::vector<std::wstring> mapFiles = {
		L"MapData/00_map.dsm"
		// 향후 추가 맵 파일들을 여기에 등록 (예: L"MapData/01_map.dsm")
	};

	for (const std::wstring& mapFileName : mapFiles) {
		MapData mapData;
		if (ParseMapFile(mapFileName, mapData)) {
			m_mapDataStorage[mapFileName] = std::move(mapData);
			OutputDebugStringW((L"SceneManager: 맵 데이터 로드 완료 - " + mapFileName + L"\n").c_str());
		}
		else {
			OutputDebugStringW((L"SceneManager: 맵 데이터 로드 실패 - " + mapFileName + L"\n").c_str());
		}
	}
}

bool SceneManager::ParseMapFile(const std::wstring& mapFileName, MapData& outMapData)
{
	// ResourceManager를 통해 오브젝트 리소스 정보 조회 (콜백)
	ResourceManager* resMgr = ResourceManager::GetInstance();
	auto getObjectResourceInfo = [resMgr](GameObjectType /*type*/, GameObjectID id) -> const ResourcePathUtils::ObjectResourceDef* {
		return resMgr->GetObjectResourceInfo(id);
	};
	
	// Function.h의 공통 파싱 함수 사용 (성공 여부 반환)
	return ResourcePathUtils::ParseMapFileInto(mapFileName, outMapData, getObjectResourceInfo);
}

void SceneManager::LoadTitleScene()
{
	m_pendingScene = PendingSceneType::Title;
	m_pendingMapFileName.clear();
	m_pendingCharacterID = GOID_NONE;
}

void SceneManager::LoadCharacterSelectScene()
{
	m_pendingScene = PendingSceneType::CharacterSelect;
	m_pendingMapFileName.clear();
	m_pendingCharacterID = GOID_NONE;
}

void SceneManager::LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID)
{
	m_pendingScene = PendingSceneType::Game;
	m_pendingMapFileName = mapFileName;
	m_pendingCharacterID = selectedCharacterID;
}

// 프레임 끝에 씬 전환 요청을 실제로 처리
void SceneManager::ProcessPendingSceneLoad()
{
	switch (m_pendingScene) {
	case PendingSceneType::Title:
		DoLoadTitleScene();
		break;
	case PendingSceneType::CharacterSelect:
		DoLoadCharacterSelectScene();
		break;
	case PendingSceneType::Game:
		DoLoadGameScene(m_pendingMapFileName, m_pendingCharacterID);
		break;
	default:
		break;
	}
	
	// 요청 처리 후 플래그 초기화
	m_pendingScene = PendingSceneType::None;
	m_pendingMapFileName.clear();
	m_pendingCharacterID = GOID_NONE;
}

// 실제 씬 로드 구현
void SceneManager::DoLoadTitleScene()
{
	OutputDebugStringW(L"SceneManager: 타이틀 씬 로드 시작\n");
	ReleaseCurrentScene();

	TitleScene* titleScene = new TitleScene();
	titleScene->Init();
	m_currentScene = titleScene;

	OutputDebugStringW(L"SceneManager: 타이틀 씬 로드 완료\n");
}

void SceneManager::DoLoadCharacterSelectScene()
{
	OutputDebugStringW(L"SceneManager: 캐릭터 선택 씬 로드 시작\n");
	ReleaseCurrentScene();

	CharacterSelectScene* characterSelectScene = new CharacterSelectScene();
	characterSelectScene->Init();
	m_currentScene = characterSelectScene;

	OutputDebugStringW(L"SceneManager: 캐릭터 선택 씬 로드 완료\n");
}

void SceneManager::DoLoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID)
{
	OutputDebugStringW(L"SceneManager: 게임 씬 로드 시작\n");
	ReleaseCurrentScene();

	GameScene* gameScene = new GameScene();
	gameScene->SetSelectedCharacterID(selectedCharacterID);

	// 미리 로드된 맵 저장소에서 찾기 (Init에서 이미 로드됨)
	auto it = m_mapDataStorage.find(mapFileName);
	if (it != m_mapDataStorage.end()) {
		m_currentMapData = &it->second;
		// GameScene은 포인터만 받음 (복사/파싱 없음, 즉시 사용)
		gameScene->Init(m_currentMapData);
	}
	else {
		OutputDebugStringW((L"SceneManager: 맵 데이터를 찾을 수 없음 - " + mapFileName + L"\n").c_str());
		m_currentMapData = nullptr;
	}
	m_currentScene = gameScene;

	OutputDebugStringW((L"SceneManager: 게임 씬 로드 완료 - 맵: " + mapFileName +
		L", 캐릭터 ID: " + std::to_wstring(selectedCharacterID) + L"\n").c_str());
}

// 현재 타입 씬, 로드, 전환 시에 사용할 SceneType을 반환하기 위한 함수
SceneType SceneManager::GetCurrentSceneType() const
{
	if (!m_currentScene) {
		return SCENE_NONE; // 기본값
	}
	return m_currentScene->GetSceneType();
}


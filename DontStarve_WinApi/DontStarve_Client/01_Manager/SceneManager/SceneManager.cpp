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
	// 첫 번째 씬 (타이틀 씬) 로드 요청 후 즉시 처리
	LoadTitleScene();
	ProcessPendingSceneLoad();
}

void SceneManager::Update(float deltaTime)
{
	if (m_currentScene) {
		m_currentScene->Update(deltaTime);
	}
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
}

void SceneManager::ReleaseCurrentScene()
{
	if (!m_currentScene) return;
	OutputDebugStringW(L"SceneManager: 이전 씬 삭제 (소멸자에서 Release 호출)...\n");
	delete m_currentScene;
	m_currentScene = nullptr;
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

void SceneManager::ProcessPendingSceneLoad()
{
	if (m_pendingScene == PendingSceneType::None) return;

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
	m_pendingScene = PendingSceneType::None;
	m_pendingMapFileName.clear();
	m_pendingCharacterID = GOID_NONE;
}

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

	const MapData* mapData = ResourceManager::GetInstance()->LoadMapData(mapFileName);
	if (mapData) {
		gameScene->Init(*mapData);
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


#pragma once

class BaseScene;
class TitleScene;
class CharacterSelectScene;
class GameScene;

// Unity 스타일: 씬 전환 요청 타입 (프레임 끝에 실제 전환 처리)
enum class PendingSceneType
{
	None,
	Title,
	CharacterSelect,
	Game
};

class SceneManager : public CSingleTon<SceneManager>
{
	friend class CSingleTon<SceneManager>;
public:
	SceneManager();
	~SceneManager();

	void Init();
	void Update(float deltaTime);
	void LateUpdate();
	void Render();
	void Release();

	// Unity 스타일: 씬 로드 요청 (버튼 콜백 등에서 호출해도 안전, 프레임 끝에 실제 전환)
	void LoadTitleScene();
	void LoadCharacterSelectScene();
	void LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID = GOID_NONE);

	// 현재 씬 타입 반환
	SceneType GetCurrentSceneType() const;
	
	// 씬 전환 요청 처리 (메인 루프에서 Update 이후 한 번만 호출)
	void ProcessPendingSceneLoad();

private:
	BaseScene* m_currentScene;
	
	// 게임 시작 시 모든 맵 데이터 로드 (씬 전환 시 재사용)
	std::map<std::wstring, MapData> m_mapDataStorage;
	const MapData* m_currentMapData;  // 현재 활성 맵 (m_mapDataStorage의 참조)

	// Unity 스타일: 지연 전환 (프레임 끝에 한 번에 처리)
	PendingSceneType m_pendingScene;
	std::wstring m_pendingMapFileName;
	GameObjectID m_pendingCharacterID;

	void ReleaseCurrentScene();
	void LoadAllMapData();  // 게임 시작 시 모든 맵 파일 로드
	bool ParseMapFile(const std::wstring& mapFileName, MapData& outMapData);  // 맵 파일 파싱 (성공 여부 반환)
	
	// 실제 씬 로드 구현 (ProcessPendingSceneLoad에서 호출)
	void DoLoadTitleScene();
	void DoLoadCharacterSelectScene();
	void DoLoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID);
};

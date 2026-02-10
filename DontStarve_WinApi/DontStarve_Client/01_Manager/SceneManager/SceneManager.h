#pragma once

class BaseScene;
class TitleScene;
class CharacterSelectScene;
class GameScene;

// 지연 씬 전환용 (버튼 콜백 등에서 즉시 전환 시 자기 자신이 삭제되는 것 방지)
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

	// 씬 로드 요청 (다음 프레임 시작 시 실제 전환 — 콜백 안에서 호출해도 안전)
	void LoadTitleScene();
	void LoadCharacterSelectScene();
	void LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID = GOID_NONE);

	// 현재 씬 타입 반환
	SceneType GetCurrentSceneType() const;

	// 지연된 씬 전환 1회 처리 (메인 루프에서 Update 직후 1번만 호출)
	void ProcessPendingSceneLoad();

private:
	BaseScene* m_currentScene;

	// 지연 전환 (ProcessPendingSceneLoad()에서 한 번에 처리)
	PendingSceneType m_pendingScene;
	std::wstring m_pendingMapFileName;
	GameObjectID m_pendingCharacterID;

	void ReleaseCurrentScene();
	void DoLoadTitleScene();
	void DoLoadCharacterSelectScene();
	void DoLoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID);
};

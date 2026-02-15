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

	// Unity 스타일: 지연 전환 (프레임 끝에 한 번에 처리)
	PendingSceneType m_pendingScene;
	std::wstring m_pendingMapFileName;
	GameObjectID m_pendingCharacterID;

	void ReleaseCurrentScene();
	
	// 실제 씬 로드 구현 (ProcessPendingSceneLoad에서 호출)
	void DoLoadTitleScene();
	void DoLoadCharacterSelectScene();
	void DoLoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID);
};

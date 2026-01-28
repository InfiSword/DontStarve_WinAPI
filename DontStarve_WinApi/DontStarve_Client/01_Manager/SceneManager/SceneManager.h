#pragma once
#include "BaseScene.h"

class BaseScene;
class TitleScene;
class CharacterSelectScene;
class GameScene;

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

	// 씬 로드 함수들
	void LoadTitleScene();
	void LoadCharacterSelectScene();
	void LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID = GOID_NONE);

	// 지연된 씬 전환 요청
	void RequestLoadTitleScene();
	void RequestLoadCharacterSelectScene();
	void RequestLoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID = GOID_NONE);

	// 맵 데이터 파싱
	void ParseMapFileInto(const std::wstring& mapFileName, MapData& mapData);

	// 현재 씬 타입 반환
	SceneType GetCurrentSceneType() const;

private:
	BaseScene* m_currentScene;

	// 씬 전환 대기 큐
	enum class PendingSceneType {
		NONE,
		TITLE,
		CHARACTER_SELECT,
		GAME
	};

	PendingSceneType m_pendingSceneType;
	std::wstring m_pendingMapFileName;
	GameObjectID m_pendingCharacterID;

	void ProcessPendingSceneChange();
};

#pragma once

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

	// 씬 전환 요청 (프레임 끝에 안전하게 교체)
	void LoadTitleScene();
	void LoadCharacterSelectScene();
	void LoadGameScene(SceneType sceneType, GameObjectID selectedCharacterID = GOID_NONE);

	// 현재 씬 타입 반환
	SceneType GetCurrentSceneType() const;
	BaseScene* GetCurrentScene() const { return m_currentScene; }

private:
	void ChangeSceneIfReserved(); // 실제로 씬을 교체하는 내부 함수

private:
	BaseScene* m_currentScene;
	BaseScene* m_nextScene; // 교체 예약된 씬
	
	std::map<SceneType, MapData> m_mapDataStorage;  // 각 씬의 맵 데이터 (초기 상태)
	const MapData* m_currentMapData;

	void LoadAllMapData();
	static bool ParseMapFile(const std::wstring& mapFileName, MapData& outMapData);
};

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
	void LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID = GOID_NONE);

	// 현재 씬 타입 반환
	SceneType GetCurrentSceneType() const;
	BaseScene* GetCurrentScene() const { return m_currentScene; }

private:
	void ChangeSceneIfReserved(); // 실제로 씬을 교체하는 내부 함수

private:
	BaseScene* m_currentScene;
	BaseScene* m_nextScene; // 교체 예약된 씬
	
	std::map<std::wstring, MapData> m_mapDataStorage;
	const MapData* m_currentMapData;

	void LoadAllMapData();
	bool ParseMapFile(const std::wstring& mapFileName, MapData& outMapData);
};

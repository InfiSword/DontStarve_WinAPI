#pragma once
#include "BaseScene.h"

class GameObject;
class Player;

class GameScene : public BaseScene
{
public:
	GameScene();
	virtual ~GameScene();

	// BaseScene 가상 함수 구현
	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override;
	virtual void Render() override;
	virtual void Release() override;

	virtual SceneType GetSceneType() const override { return SCENE_GAME_FARMING_AREA; }
	
	// 매니저 업데이트 함수들 구현
	virtual void UpdateManagers(float deltaTime) override;
	virtual void LateUpdateManagers() override;
	virtual void RenderManagers() override;
	virtual void ReleaseManagers() override;

	// 매니저 초기화/해제 함수들
	virtual void InitializeManagers() override;
	virtual void ReleaseAllManagers() override;

	// 맵 데이터 초기화
	void Init(const MapData& mapData);

	// 플레이어 생성 함수 (public으로 노출)
	void SpawnPlayer();
	
	// 씬 클리어 관련 함수
	void ClearScene(SceneType sceneType);
	bool IsSceneCleared(SceneType sceneType) const;
	bool CheckSceneClearCondition(SceneType sceneType) const; 
	const GameProgress& GetGameProgress() const { return m_gameProgress; }
	
	// 선택된 캐릭터 ID 설정
	void SetSelectedCharacterID(GameObjectID characterID) { m_selectedCharacterID = characterID; }
	GameObjectID GetSelectedCharacterID() const { return m_selectedCharacterID; }
	
	// 게임 진행 정보 저장/로드
	void SaveGameProgress();
	void LoadGameProgress();

protected:
	virtual void CreateUI() override;

private:
	void CreateGameObjectsFromMapData();
	
private:
	// 맵 데이터
	MapData m_mapData;
	
	// 게임 진행 정보
	GameProgress m_gameProgress;
	
	// 선택된 캐릭터 ID
	GameObjectID m_selectedCharacterID;
};

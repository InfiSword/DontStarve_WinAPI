#pragma once
#include "BaseScene.h"

class GameObject;
class Player;
class CraftingUI;

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
	
	// 맵 데이터 초기화 (SceneManager가 소유한 MapData 포인터만 받음)
	void Init(const MapData* mapData);

	// 플레이어 생성 함수 (public으로 노출)
	void SpawnPlayer();
	
	// 선택된 캐릭터 ID 설정
	void SetSelectedCharacterID(GameObjectID characterID) { m_selectedCharacterID = characterID; }
	GameObjectID GetSelectedCharacterID() const { return m_selectedCharacterID; }

private:
	void CreateGameObjectsFromMapData();
	
private:
	// 맵 데이터 (SceneManager가 소유, GameScene은 포인터로만 참조)
	const MapData* m_mapData;
	
	// 선택된 캐릭터 ID
	GameObjectID m_selectedCharacterID;

	// 크래프팅 UI
	CraftingUI* m_craftingUI;
};

#pragma once
#include "BaseScene.h"

class GameObject;
class Player;
class MenuUI;
class HPUI;
class GameOverUI;

class GameScene : public BaseScene
{
public:
	GameScene();
	virtual ~GameScene();

	// BaseScene 가상 함수 구현
	virtual void Init(const MapData* mapData) override;
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override;
	virtual void Render() override;
	virtual void Release() override;

	virtual SceneType GetSceneType() const override { return SCENE_GAME_FARMING_AREA; }

	// Walkable 영역 정보 조회 (몬스터, 기타 시스템에서 사용)
	bool HasWalkableBounds() const { return m_hasWalkableBounds; }
	void GetWalkableBounds(float& outMinX, float& outMinY, float& outMaxX, float& outMaxY) const
	{
		if (!m_hasWalkableBounds)
		{
			outMinX = outMinY = outMaxX = outMaxY = 0.0f;
			return;
		}
		outMinX = m_walkableMinX;
		outMinY = m_walkableMinY;
		outMaxX = m_walkableMaxX;
		outMaxY = m_walkableMaxY;
	}

	// 플레이어 생성 함수 (public으로 노출)
	void SpawnPlayer();
	
	// 선택된 캐릭터 ID 설정
	void SetSelectedCharacterID(GameObjectID characterID) { m_selectedCharacterID = characterID; }
	GameObjectID GetSelectedCharacterID() const { return m_selectedCharacterID; }

	// 현재 월드의 오브젝트 상태를 MapData에 저장 (런타임 유지용)
	virtual void SaveCurrentObjectsToMapData(MapData& outMapData);

protected:
	void CreateGameObjectsFromMapData();
	
protected:
	// 맵 데이터 (SceneManager가 소유, GameScene은 포인터로만 참조)
	const MapData* m_mapData;

	// Walkable 영역 경계 (월드 좌표)
	bool m_hasWalkableBounds;
	float m_walkableMinX;
	float m_walkableMinY;
	float m_walkableMaxX;
	float m_walkableMaxY;
	
	// 선택된 캐릭터 ID
	GameObjectID m_selectedCharacterID;

private:
	// 크래프팅 UI
	MenuUI* m_craftingUI;

	// 플레이어 HP UI (우측 상단 게이지)
	HPUI* m_playerHPUI;

	// 게임 오버 UI
	GameOverUI* m_gameOverUI;
};

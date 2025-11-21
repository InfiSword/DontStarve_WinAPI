#pragma once
#include "BaseScene.h"

class GameObject;
class Player;

class GameScene : public BaseScene
{
public:
	GameScene();
	virtual ~GameScene();

	// BaseScene �����Լ� ����
	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override;
	virtual void Render() override;
	virtual void Release() override;
	// TODO: �� Ÿ�ϸ��� �ٸ� Ÿ�ϼ����� ��ȯ�ϵ��� ���� �ʿ�
	virtual SceneType GetSceneType() const override { return SCENE_GAME_FARMING_AREA; }
	
	// �Ŵ��� ������Ʈ �Լ��� ����
	virtual void UpdateManagers(float deltaTime) override;
	virtual void LateUpdateManagers() override;
	virtual void RenderManagers() override;
	virtual void ReleaseManagers() override;

	// �Ŵ��� �ʱ�ȭ/���� �Լ���
	virtual void InitializeManagers() override;
	virtual void ReleaseAllManagers() override;

	// GameScene ���� �ʱ�ȭ �Լ� (�� �����Ϳ� �Բ�)
	void Init(const MapData& mapData);

	// �÷��̾� ���� �޼ҵ� (public���� ����)
	void SpawnPlayer();
	
	// �� Ŭ���� ���� �޼ҵ�
	void ClearScene(SceneType sceneType);
	bool IsSceneCleared(SceneType sceneType) const;
	bool CheckSceneClearCondition(SceneType sceneType) const;  // �� Ŭ���� ���� Ȯ��
	const GameProgress& GetGameProgress() const { return m_gameProgress; }
	
	// ���õ� ĳ���� ID ����
	void SetSelectedCharacterID(GameObjectID characterID) { m_selectedCharacterID = characterID; }
	GameObjectID GetSelectedCharacterID() const { return m_selectedCharacterID; }
	
	// ���� ���� ���� ����
	void SaveGameProgress();
	void LoadGameProgress();

protected:
	virtual void CreateUI() override;

private:
	void CreateGameObjectsFromMapData();
	void HandlePlayerMovement(); // Moved to private as it's an internal scene logic
	
private:
	// �� ������
	MapData m_mapData;
	
	// ���� ���� ����
	GameProgress m_gameProgress;
	
	// ���õ� ĳ���� ID
	GameObjectID m_selectedCharacterID;
}; 
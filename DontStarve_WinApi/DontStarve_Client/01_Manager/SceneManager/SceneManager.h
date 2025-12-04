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

	// �� �ε� �Լ���
	void LoadTitleScene();
	void LoadCharacterSelectScene();
	void LoadGameScene(const std::wstring& mapFileName, GameObjectID selectedCharacterID = GOID_NONE);
	void ReturnToTitle();  // �ٽ� Ÿ��Ʋ��

	// �� ���� �Ľ�
	void ParseMapFileInto(const std::wstring& mapFileName, MapData& mapData);

	// ���̵� ȿ��
	void StartFadeOut();
	void StartFadeIn();
	void UpdateFadeEffect(float deltaTime);
	void RenderFadeEffect();

	// ���� �� Ÿ�� ��ȯ
	SceneType GetCurrentSceneType() const;

private:
	// ���̵� ȿ�� ����
	enum class TransitionState {
		NONE,
		FADE_OUT,
		SCENE_SWITCH,  // �� ��ȯ �� (�� �� �ʱ�ȭ)
		FADE_IN
	};

	BaseScene* m_currentScene;
	BaseScene* m_nextScene;
	TransitionState m_transitionState;
	float m_fadeAlpha;
	float m_fadeDuration;
	
	// �ӽ� �� ������ �����
	std::wstring m_tempMapFileName;
	GameObjectID m_tempSelectedCharacterID;
}; 
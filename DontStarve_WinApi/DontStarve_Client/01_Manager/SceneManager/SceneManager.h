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
	void ReturnToTitle();  // 다시 타이틀로

	// 맵 파일 파싱
	void ParseMapFileInto(const std::wstring& mapFileName, MapData& mapData);

	// 페이드 효과
	void StartFadeOut();
	void StartFadeIn();
	void UpdateFadeEffect(float deltaTime);
	void RenderFadeEffect();

	// 현재 씬 타입 반환
	SceneType GetCurrentSceneType() const;

private:
	// 페이드 효과 관련
	enum class TransitionState {
		NONE,
		FADE_OUT,
		SCENE_SWITCH,  // 씬 전환 중 (새 씬 초기화)
		FADE_IN
	};

	BaseScene* m_currentScene;
	BaseScene* m_nextScene;
	TransitionState m_transitionState;
	float m_fadeAlpha;
	float m_fadeDuration;
	
	// 임시 맵 데이터 저장용
	std::wstring m_tempMapFileName;
	GameObjectID m_tempSelectedCharacterID;
}; 
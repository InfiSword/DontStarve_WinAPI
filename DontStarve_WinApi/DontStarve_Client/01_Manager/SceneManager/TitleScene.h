#pragma once
#include "BaseScene.h"

class TitleScene : public BaseScene
{
public:
	TitleScene();
	virtual ~TitleScene();

	// BaseScene 가상함수 구현
	virtual void Init() override;
	virtual void Update(float deltaTime) override;
	virtual void LateUpdate() override;
	virtual void Render() override;
	virtual void Release() override;
	virtual SceneType GetSceneType() const override { return SCENE_TITLE; }
	
	virtual void UpdateManagers(float deltaTime) override;
	virtual void LateUpdateManagers() override;
	virtual void RenderManagers() override;
	virtual void ReleaseManagers() override;

	// 매니저 초기화/해제 함수들
	virtual void InitializeManagers() override;
	virtual void ReleaseAllManagers() override;

protected:
	virtual void CreateUI() override;

private:
	void OnStartButtonClicked();
	void OnExitButtonClicked();
};

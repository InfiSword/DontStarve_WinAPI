#pragma once

// BaseScene: 모든 Scene의 부모가 되는 추상 클래스
class BaseScene
{
public:
	BaseScene() = default;
	virtual ~BaseScene() = default;

	// 기본 필수 함수들 - 모든 Scene에서 구현해야 함
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void LateUpdate() = 0;
	virtual void Render() = 0;
	virtual void Release() = 0;

	// 씬의 SceneType 반환 (UI/게임 씬 전환 시 사용, 각 Enum에서 정의)
	virtual SceneType GetSceneType() const = 0;
	
	virtual void UpdateManagers(float deltaTime) = 0;
	virtual void LateUpdateManagers() = 0;
	virtual void RenderManagers() = 0;
	virtual void ReleaseManagers() = 0;

	// 매니저 초기화/해제 함수들
	virtual void InitializeManagers() = 0;
	virtual void ReleaseAllManagers() = 0;

protected:
	virtual void CreateUI() = 0;
};
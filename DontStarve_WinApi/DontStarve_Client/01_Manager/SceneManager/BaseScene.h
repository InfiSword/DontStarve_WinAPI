#pragma once

// BaseScene: ��� Scene�� �θ� �Ǵ� �߻� Ŭ����
class BaseScene
{
public:
	BaseScene() = default;
	virtual ~BaseScene() = default;

	// �⺻ �ʼ� �Լ��� - ��� Scene���� �����ؾ� ��
	virtual void Init() = 0;
	virtual void Update(float deltaTime) = 0;
	virtual void LateUpdate() = 0;
	virtual void Render() = 0;
	virtual void Release() = 0;

	// ���� SceneType ��ȯ (UI/���� �� ��ȯ �� ���, �� Enum���� ����)
	virtual SceneType GetSceneType() const = 0;
	
	virtual void UpdateManagers(float deltaTime) = 0;
	virtual void LateUpdateManagers() = 0;
	virtual void RenderManagers() = 0;
	virtual void ReleaseManagers() = 0;

	// �Ŵ��� �ʱ�ȭ/���� �Լ���
	virtual void InitializeManagers() = 0;
	virtual void ReleaseAllManagers() = 0;

protected:
	virtual void CreateUI() = 0;
};
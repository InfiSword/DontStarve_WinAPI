#pragma once

class GameObject;

class Component
{
protected:
	GameObject* m_owner;	// 이 컴포넌트를 소유한 게임 오브젝트
	bool m_enabled;			// 컴포넌트 활성화 여부

public:
	Component(GameObject* owner);
	virtual ~Component();

	virtual void Init() {}
	virtual void LateInit() {}
	virtual void Update(float deltaTime) {}
	virtual void LateUpdate() {}
	virtual void Release() {}

	GameObject* GetOwner() const { return m_owner; }
	
	void SetEnable(bool enable) { m_enabled = enable; }
	bool IsEnabled() const { return m_enabled; }
};


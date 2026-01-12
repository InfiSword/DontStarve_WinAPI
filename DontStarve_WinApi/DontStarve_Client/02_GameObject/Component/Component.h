#pragma once

#include <Enum.h>  // RenderLayer, Direction 등 정의 (Object.h보다 먼저 포함)
#include "../Object.h"

class GameObject;

class Component : public Object
{
protected:
	GameObject* m_owner;	// 이 컴포넌트를 소유한 게임 오브젝트

public:
	Component(GameObject* owner);
	virtual ~Component();

	virtual void Init() {}
	virtual void LateInit() {}
	virtual void Update(float deltaTime) {}
	virtual void LateUpdate() {}
	virtual void Release() {}

	GameObject* GetOwner() const { return m_owner; }
};


#include "99_Default/pch.h"
#include "GameObject.h"
#include "Component/Transform/Transform.h"
#include "Component/Sprite/SpriteRenderer.h"

GameObject::GameObject(GameObjectType type, GameObjectID id,
	const std::wstring& resourcePath, const std::wstring& imageName,
	bool isActive, bool isInteractive)
	: Object(), m_type(type), m_id(id), m_isInteractive(isInteractive)
{
	SetActive(isActive);
}

GameObject::~GameObject() { Release(); }

void GameObject::Init() {
	for (auto& component : m_components) {
		component->Init();
	}
}

void GameObject::LateInit() {
	for (auto& component : m_components) {
		component->LateInit();
	}
}

void GameObject::Update(float deltaTime) {
	for (auto& component : m_components) {
		if (component->IsEnabled()) {
			component->Update(deltaTime);
		}
	}
}

void GameObject::LateUpdate() {
	for (auto& component : m_components) {
		if (component->IsEnabled()) {
			component->LateUpdate();
		}
	}
}

void GameObject::Release() 
{ 
	// 컴포넌트 해제
	for (auto& component : m_components) {
		component->Release();
		SafeDelete(component);
	}
	m_components.clear();
}

void GameObject::OnInteraction(GameObject* obj)
{
}

#include "99_Default/pch.h"
#include "GameObject.h"
#include "Component/Transform/Transform.h"
#include "Component/Sprite/SpriteRenderer.h"

GameObject::GameObject(GameObjectType type, GameObjectID id,
	const std::wstring& resourcePath, const std::wstring& imageName,
	bool isActive, bool isInteractive)
	: Object(), m_type(type), m_id(id), m_isInteractive(isInteractive), m_bReleased(false)
{
	SetActive(isActive);
}

GameObject::~GameObject() { Release(); }

void GameObject::Init() {
	for (auto& component : m_components) {
		if (component) {
			component->Init();
		}
	}
}

void GameObject::LateInit() {
	for (auto& component : m_components) {
		if (component) {
			component->LateInit();
		}
	}
}

void GameObject::Update(float deltaTime) {
	// Release()가 호출되었으면 업데이트하지 않음
	if (m_bReleased) {
		return;
	}

	for (auto& component : m_components) {
		if (component && component->IsEnabled()) {
			component->Update(deltaTime);
		}
	}
}

void GameObject::LateUpdate() {
	// Release()가 호출되었으면 업데이트하지 않음
	if (m_bReleased) {
		return;
	}

	for (auto& component : m_components) {
		if (component && component->IsEnabled()) {
			component->LateUpdate();
		}
	}
}

void GameObject::Release() 
{ 
	// 이미 Release()가 호출되었으면 중복 호출 방지
	if (m_bReleased) {
		return;
	}
	m_bReleased = true;

	// 컴포넌트 해제
	for (auto& component : m_components) {
		if (component) {
			component->Release();
			SafeDelete(component);
		}
	}
	m_components.clear();
}

void GameObject::OnInteraction(GameObject* obj)
{
	if (!obj || !IsEnabled() || !CanInteract()) return;
	// 파생 클래스에서 오버라이드하여 구체적인 상호작용 로직 구현
}

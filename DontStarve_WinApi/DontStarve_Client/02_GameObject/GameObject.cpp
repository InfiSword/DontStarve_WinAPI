#include "99_Default/pch.h"
#include "GameObject.h"
#include "Component/Transform/Transform.h"
#include "Component/Sprite/SpriteRenderer.h"
#include "../01_Manager/TimeManager/TimeManager.h"

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

	// 코루틴은 모든 Update(애니메이션 이벤트 등) 후 같은 프레임에 실행하여 렌더에 반영
	float deltaTime = TimeManager::GetInstance() ? TimeManager::GetInstance()->GetDeltaTime() : 0.0f;
	UpdateCoroutines(deltaTime);
}

void GameObject::Release() 
{ 
	// 이미 Release()가 호출되었으면 중복 호출 방지
	if (m_bReleased) {
		return;
	}
	m_bReleased = true;

	StopAllCoroutines();

	// 문자열 멤버 강제 해제 (swap으로 CRT 누수 탐지에 반영)
	std::wstring().swap(m_name);

	// 컴포넌트 해제
	for (auto& component : m_components) {
		if (component) {
			component->Release();
			Utils::SafeDelete(component);
		}
	}
	m_components.clear();
	m_components.shrink_to_fit(); 
}

bool GameObject::OnInteraction(GameObject* obj)
{
	if (!obj || !IsEnabled() || !CanInteract()) 
		return false;
	
	return true;
}

void GameObject::StartCoroutine(CoroutineHandle coroutine)
{
	if (coroutine)
		m_coroutines.push_back(std::move(coroutine));
}

void GameObject::StopAllCoroutines()
{
	m_coroutines.clear();
}

void GameObject::UpdateCoroutines(float deltaTime)
{
	if (m_coroutines.empty()) return;

	// 완료된 코루틴(false 반환)을 제거하면서 순회
	auto it = m_coroutines.begin();
	while (it != m_coroutines.end()) {
		bool stillRunning = (*it)(deltaTime);
		if (!stillRunning)
			it = m_coroutines.erase(it);
		else
			++it;
	}
}

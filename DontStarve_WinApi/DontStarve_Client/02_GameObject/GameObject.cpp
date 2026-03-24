#include "99_Default/pch.h"
#include "GameObject.h"
#include "Component/Transform/Transform.h"
#include "Component/Sprite/SpriteRenderer.h"
#include "../01_Manager/TimeManager/TimeManager.h"

bool GameObject::g_bRenderDebugOverlay = false;

GameObject::GameObject(GameObjectID id,
	const std::wstring& resourcePath, const std::wstring& imageName,
	bool isActive, bool isInteractive)
	: Object(), m_id(id), m_isInteractive(isInteractive), m_bReleased(false), m_type(GameObjectType::GO_TYPE_NONE)
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
	UpdateCoroutines(deltaTime);
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
	if (!coroutine) return;
	// 첫 코루틴 등록 시 재할당 횟수 감소 (프록시/할당 압력 완화)
	if (m_coroutines.capacity() == 0)
		m_coroutines.reserve(4);
	m_coroutines.push_back(std::move(coroutine));
}

void GameObject::StopAllCoroutines()
{
	m_coroutines.clear();
	m_coroutines.shrink_to_fit();
}

void GameObject::UpdateCoroutines(float deltaTime)
{
	if (m_coroutines.empty()) return;

	size_t i = 0;
	while (i < m_coroutines.size()) {
		bool stillRunning = m_coroutines[i](deltaTime);
		if (!stillRunning) {
			if (i == m_coroutines.size() - 1) {
				m_coroutines.pop_back();
			} else {
				m_coroutines[i] = std::move(m_coroutines.back());
				m_coroutines.pop_back();
			}
		} else {
			++i;
		}
	}
	// 코루틴이 모두 끝나면 예약된 capacity 반환 (메모리 누적 완화)
	if (m_coroutines.empty())
		m_coroutines.shrink_to_fit();
}

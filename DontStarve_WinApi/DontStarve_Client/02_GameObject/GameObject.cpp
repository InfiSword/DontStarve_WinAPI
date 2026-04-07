#include "99_Default/pch.h"
#include "GameObject.h"
#include "Component/Transform/Transform.h"
#include "Component/Sprite/SpriteRenderer.h"
#include "Component/Collider/Collider.h"
#include "../01_Manager/TimeManager/TimeManager.h"
#include "../03_Animation/Animator.h"
#include "../01_Manager/ObjectManager/ObjectManager.h"

bool GameObject::g_bRenderDebugOverlay = false;

GameObject::GameObject(GameObjectID id,
	const std::wstring& resourcePath, const std::wstring& imageName,
	bool isActive, bool isInteractive)
	: Object(), m_id(id), m_isInteractive(isInteractive), m_type(GameObjectType::GO_TYPE_NONE), m_isDead(false)
{
	SetActive(isActive);
}

GameObject::~GameObject() 
{ 
	Release(); 
}

void GameObject::Init() {
	for (auto& component : m_components) {
		if (component) {
			component->Init();
		}
	}
	// 초기 위치에 따른 그리드 셀 설정
	// if (!IsUI()) {
	// 	ObjectManager::GetInstance()->UpdateObjectGridCell(this);
	// }
}

void GameObject::LateInit() {
	for (auto& component : m_components) {
		if (component) {
			component->LateInit();
		}
	}
}

void GameObject::Update(float deltaTime) {

	for (auto& component : m_components) {
		if (component && component->IsEnabled()) {
			component->Update(deltaTime);
		}
	}
	UpdateCoroutines(deltaTime);
}

void GameObject::LateUpdate() {

	// 위치/크기 변경이 있었다면 바운딩 박스와 그리드 셀 갱신 (LateUpdate 최상단)
	if (m_isBoundsDirty) {
		GetBounds();
	}

	for (auto& component : m_components) {
		if (component && component->IsEnabled()) {
			component->LateUpdate();
		}
	}
}

void GameObject::Release() 
{ 

	// 공간 분할 그리드에서 제거
	if (!IsUI()) {
		ObjectManager::GetInstance()->RemoveGameObject(this); 
	}

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

Gdiplus::RectF GameObject::GetBounds()
{
	if (!m_isBoundsDirty) return m_cachedBounds;

	Transform* t = GetComponent<Transform>();
	if (!t) {
		m_cachedBounds = { 0,0,0,0 };
		m_isBoundsDirty = false;
		return m_cachedBounds;
	}

	float w = 32, h = 32, px = 0.5f, py = 0.5f;
	if (auto* anim = GetComponent<Animator>()) {
		if (auto sprite = anim->GetCurrentFrame().sprite) {
			w = sprite->sourceRect.Width; h = sprite->sourceRect.Height;
			px = sprite->pivot.X; py = sprite->pivot.Y;
		}
	}
	else if (auto* sr = GetComponent<SpriteRenderer>()) {
		if (auto sprite = sr->GetSpriteHandle()) {
			w = sprite->sourceRect.Width; h = sprite->sourceRect.Height;
			px = sprite->pivot.X; py = sprite->pivot.Y;
		}
	}
	w *= t->GetScaleX(); h *= t->GetScaleY();

	m_cachedBounds = { t->GetX() - w * px, t->GetY() - h * py, w, h };
	m_isBoundsDirty = false;
	return m_cachedBounds;
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

void GameObject::Render()
{
}

void GameObject::MainColliderGizmo()
{
	Collider* pMainCol = GetMainCollider();
	if (pMainCol && pMainCol->IsEnabled()) {
		pMainCol->RenderGizmo();
	}
}

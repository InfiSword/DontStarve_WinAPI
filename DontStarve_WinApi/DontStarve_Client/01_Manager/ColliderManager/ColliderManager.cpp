#include "99_Default/pch.h"
#include "ColliderManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../02_GameObject/GameObject.h"
#include "../../02_GameObject/Component/Collider/Collider.h"
#include <Enum.h>

ColliderManager::ColliderManager()
{
}

ColliderManager::~ColliderManager()
{
}

void ColliderManager::Init()
{
	// 씬 재진입 시 콜라이더 목록을 비워 깨끗한 상태로 시작 (Release 호출 없이 Init만 호출되는 경우 대비)
	m_colliders.clear();
}

void ColliderManager::LateInit()
{

}

void ColliderManager::Update(float deltaTime)
{
    // 모든 게임오브젝트의 콜라이더 위치 업데이트 
}

void ColliderManager::LateUpdate()
{
    // 모든 게임오브젝트의 충돌 검사 처리 
}

void ColliderManager::RenderGizmos()
{
    // 모든 콜라이더의 Gizmo 렌더링 (각 Collider가 활성화 상태인지 확인)
    for (Collider* pCollider : m_colliders) {
        pCollider->RenderGizmo();
    }
}

void ColliderManager::Release()
{
    // Collider 해제는 Component 해제시에 처리되므로 여기서는 리스트만 정리
    m_colliders.clear();
    m_colliders.shrink_to_fit();
}

void ColliderManager::AddCollider(Collider* pCollider)
{
    if (!pCollider) return;
    // 중복 등록 방지 (AddComponent가 Init을 즉시 호출하고 InitializeObjects가 Init을 다시 호출하는 구조 대비)
    if (std::find(m_colliders.begin(), m_colliders.end(), pCollider) == m_colliders.end()) {
        m_colliders.push_back(pCollider);
    }
}

void ColliderManager::RemoveCollider(Collider* pCollider)
{
    if (!pCollider) return;
    // 리스트에서 콜라이더 포인터 제거 (실제 Component 해제시에 처리)
    m_colliders.erase(std::remove(m_colliders.begin(), m_colliders.end(), pCollider), m_colliders.end());
}

bool ColliderManager::CheckCollision(GameObject* obj1, GameObject* obj2)
{
    // 유효성 검사
    if (!obj1 || !obj2 || obj1 == obj2) {
        return false;
    }

    // 두 게임오브젝트가 모두 활성화되어 있는지
    if (!obj1->IsEnabled() || !obj2->IsEnabled()) {
        return false;
    }

    // 두 게임오브젝트의 Collider Component 가져오기
    Collider* collider1 = obj1->GetComponent<Collider>();
    Collider* collider2 = obj2->GetComponent<Collider>();

    // 두 게임오브젝트 모두 Collider가 있어야 충돌 검사 가능
    if (!collider1 || !collider2) {
        return false;
    }

    // 각 콜라이더 간의 충돌 검사 (실제로는 각 게임오브젝트에서 처리)
    return collider1->IntersectsCollider(collider2);
}

bool ColliderManager::Intersects(Collider* a, Collider* b)
{
	if (!a || !b) return false;
	if (!a->IsEnabled() || !b->IsEnabled()) return false;
	return a->IntersectsCollider(b);
}

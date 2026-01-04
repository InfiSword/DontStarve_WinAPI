#include "../../99_Default/pch.h"
#include "ColliderManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../02_GameObject/GameObject.h"
#include "../../02_GameObject/Component/Collider.h"

ColliderManager::ColliderManager()
{
}

ColliderManager::~ColliderManager()
{
}

void ColliderManager::Init()
{

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
    // 모든 오브젝트의 충돌 검사 처리 
}

void ColliderManager::RenderGizmos()
{
    // 모든 콜라이더의 Gizmo 렌더링 (각 Collider가 활성화 여부를 확인함)
    for (Collider* pCollider : m_colliders) {
        pCollider->RenderGizmo();
    }
}

void ColliderManager::Release()
{
    // Collider 삭제는 Component 생명주기에서 처리하므로 여기서는 리스트만 비움
    m_colliders.clear();
}

void ColliderManager::AddCollider(Collider* pCollider)
{
    if (pCollider) {
        m_colliders.push_back(pCollider);
    }
}

void ColliderManager::RemoveCollider(Collider* pCollider)
{
    // 리스트에서 콜라이더 제거만 수행 (삭제는 Component 생명주기에서 처리)
    m_colliders.erase(std::remove(m_colliders.begin(), m_colliders.end(), pCollider), m_colliders.end());
}

bool ColliderManager::CheckCollision(GameObject* obj1, GameObject* obj2)
{
    // 유효성 검사
    if (!obj1 || !obj2 || obj1 == obj2) {
        return false;
    }

    // 두 오브젝트가 모두 활성화되어 있어야 함
    if (!obj1->IsEnabled() || !obj2->IsEnabled()) {
        return false;
    }

    // 두 오브젝트의 Collider Component 가져오기
    Collider* collider1 = obj1->GetComponent<Collider>();
    Collider* collider2 = obj2->GetComponent<Collider>();

    // 두 오브젝트 모두 Collider가 있어야 충돌 검사 가능
    if (!collider1 || !collider2) {
        return false;
    }

    // 두 콜라이더 간의 충돌 검사 (접촉한 두 오브젝트만 처리)
    return collider1->IntersectsCollider(collider2);
}

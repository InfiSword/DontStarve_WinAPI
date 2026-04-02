#include "99_Default/pch.h"
#include "ColliderManager.h"
#include "../ObjectManager/ObjectManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../02_GameObject/GameObject.h"
#include "../../02_GameObject/Component/Collider/Collider.h"
#include "../../02_GameObject/Component/Collider/BoxCollider.h"
#include "../../02_GameObject/Component/Collider/CircleCollider.h"
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
	// 모든 활성 콜라이더 간의 충돌 검사를 공간 분할 그리드를 활용해 수행
	ObjectManager* objMgr = ObjectManager::GetInstance();
	if (!objMgr) return;

	for (size_t i = 0; i < m_colliders.size(); ++i) {
		Collider* pSrc = m_colliders[i];
		if (!pSrc || !pSrc->IsEnabled()) continue;

		GameObject* pSrcOwner = pSrc->GetOwner();
		if (!pSrcOwner || !pSrcOwner->IsEnabled()) continue;

		// 현재 콜라이더의 영역을 기준으로 주변 객체들만 쿼리
		Gdiplus::RectF srcRect = pSrc->GetWorldRect();
		std::vector<GameObject*> potentialTargets;
		objMgr->GetObjectsInRect(srcRect, potentialTargets);

		for (GameObject* pDstOwner : potentialTargets) {
			if (!pDstOwner || pDstOwner == pSrcOwner || !pDstOwner->IsEnabled()) continue;

			// 대상 객체의 모든 콜라이더와 검사
			std::vector<Collider*> dstColliders = pDstOwner->GetComponents<Collider>();
			for (Collider* pDst : dstColliders) {
				if (!pDst || !pDst->IsEnabled()) continue;

				// 실제 정밀 충돌 검사
				if (Intersects(pSrc, pDst)) {
					pSrcOwner->OnCollision(pDstOwner);
				}
			}
		}
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

    // 두 게임오브젝트의 모든 Collider Component 가져오기
    std::vector<Collider*> colliders1 = obj1->GetComponents<Collider>();
    std::vector<Collider*> colliders2 = obj2->GetComponents<Collider>();

    // 각 게임오브젝트의 모든 콜라이더 쌍에 대해 충돌 검사 수행
    for (Collider* c1 : colliders1) {
        if (!c1->IsEnabled()) continue;
        for (Collider* c2 : colliders2) {
            if (!c2->IsEnabled()) continue;
            if (Intersects(c1, c2)) {
                return true;
            }
        }
    }

    return false;
}

bool ColliderManager::Intersects(Collider* a, Collider* b)
{
	if (!a || !b) return false;
	if (!a->IsEnabled() || !b->IsEnabled()) return false;

	ColliderType typeA = a->GetColliderType();
	ColliderType typeB = b->GetColliderType();

	if (typeA == COLLIDER_BOX && typeB == COLLIDER_BOX) {
		BoxCollider* boxA = static_cast<BoxCollider*>(a);
		BoxCollider* boxB = static_cast<BoxCollider*>(b);
		RECT rectA = boxA->GetWorldBoundingBox();
		RECT rectB = boxB->GetWorldBoundingBox();
		return !(rectA.right < rectB.left || rectA.left > rectB.right ||
			rectA.bottom < rectB.top || rectA.top > rectB.bottom);
	}
	else if (typeA == COLLIDER_CIRCLE && typeB == COLLIDER_CIRCLE) {
		CircleCollider* circleA = static_cast<CircleCollider*>(a);
		CircleCollider* circleB = static_cast<CircleCollider*>(b);
		float cxA, cyA, rA; circleA->GetWorldCircle(cxA, cyA, rA);
		float cxB, cyB, rB; circleB->GetWorldCircle(cxB, cyB, rB);
		float dx = cxA - cxB; float dy = cyA - cyB;
		float distSq = dx * dx + dy * dy;
		float rSum = rA + rB;
		return distSq <= (rSum * rSum);
	}
	else if ((typeA == COLLIDER_BOX && typeB == COLLIDER_CIRCLE) || (typeA == COLLIDER_CIRCLE && typeB == COLLIDER_BOX)) {
		BoxCollider* box = (typeA == COLLIDER_BOX) ? static_cast<BoxCollider*>(a) : static_cast<BoxCollider*>(b);
		CircleCollider* circle = (typeA == COLLIDER_CIRCLE) ? static_cast<CircleCollider*>(a) : static_cast<CircleCollider*>(b);

		RECT rect = box->GetWorldBoundingBox();
		float cx, cy, r; circle->GetWorldCircle(cx, cy, r);

		float closestX = (std::max)((float)rect.left, (std::min)(cx, (float)rect.right));
		float closestY = (std::max)((float)rect.top, (std::min)(cy, (float)rect.bottom));

		float dx = cx - closestX; float dy = cy - closestY;
		return (dx * dx + dy * dy) <= (r * r);
	}

	return false;
}

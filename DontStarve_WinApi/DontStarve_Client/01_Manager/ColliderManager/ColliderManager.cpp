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
	Release();
}

void ColliderManager::Init()
{
	m_colliders.clear();
	m_queryBuffer.clear();
}

void ColliderManager::LateInit()
{
}

void ColliderManager::Update(float deltaTime)
{
}

void ColliderManager::LateUpdate()
{
	ObjectManager* objMgr = ObjectManager::GetInstance();
	if (!objMgr) return;

	// 최적화: 모든 콜라이더를 순회하는 대신, 능동적으로 움직이는 객체들만 충돌 검사를 시작함
	for (size_t i = 0; i < m_colliders.size(); ++i) {
		Collider* pSrc = m_colliders[i];
		if (!pSrc || !pSrc->IsEnabled()) continue;

		// 물리 충돌(몸통 밀기 등)은 물리 콜라이더끼리만 수행
		if (!pSrc->IsPhysicalCollider()) continue;

		GameObject* pSrcOwner = pSrc->GetOwner();
		if (!pSrcOwner || !pSrcOwner->IsEnabled()) continue;

		// 정적 객체(나무, 돌 등)는 스스로 충돌 검사를 시작하지 않음 (연산량 절감)
		GameObjectType type = pSrcOwner->GetType();
		if (type != GO_TYPE_PLAYER && type != GO_TYPE_MONSTER) {
			continue;
		}

		// 현재 콜라이더의 영역을 기준으로 주변 객체들만 쿼리
		Gdiplus::RectF srcRect = pSrc->GetWorldRect();
		
		m_queryBuffer.clear();
		objMgr->GetObjectsInRect(srcRect, m_queryBuffer);

		for (GameObject* pDstOwner : m_queryBuffer) {
			if (!pDstOwner || pDstOwner == pSrcOwner || !pDstOwner->IsEnabled()) continue;

			// 최적화: 대상의 모든 콜라이더를 뒤지는 대신, 메인(몸통) 콜라이더와만 검사
			Collider* pDst = pDstOwner->GetMainCollider();
			if (pDst && pDst->IsEnabled() && pDst->IsPhysicalCollider()) {
				if (Intersects(pSrc, pDst)) {
					pSrcOwner->OnCollision(pDstOwner);
				}
			}
		}
	}
}

void ColliderManager::Release()
{
    m_colliders.clear();
    m_colliders.shrink_to_fit();
	m_queryBuffer.clear();
	m_queryBuffer.shrink_to_fit();
}

void ColliderManager::AddCollider(Collider* pCollider)
{
    if (!pCollider) return;
    if (std::find(m_colliders.begin(), m_colliders.end(), pCollider) == m_colliders.end()) {
        m_colliders.push_back(pCollider);
    }
}

void ColliderManager::RemoveCollider(Collider* pCollider)
{
    if (!pCollider) return;
    m_colliders.erase(std::remove(m_colliders.begin(), m_colliders.end(), pCollider), m_colliders.end());
}

bool ColliderManager::CheckCollision(GameObject* obj1, GameObject* obj2)
{
    if (!obj1 || !obj2 || obj1 == obj2) return false;
    if (!obj1->IsEnabled() || !obj2->IsEnabled()) return false;

    Collider* c1 = obj1->GetMainCollider();
    Collider* c2 = obj2->GetMainCollider();

    if (c1 && c2 && c1->IsEnabled() && c2->IsEnabled()) {
        return Intersects(c1, c2);
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

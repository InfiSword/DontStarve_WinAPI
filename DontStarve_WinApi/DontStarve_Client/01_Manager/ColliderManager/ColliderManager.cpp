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

	// [역할] Broad-phase 시작점: 능동 주체(플레이어/몬스터)만 충돌 탐색을 시작한다.
	// [최적화] 정적/수동 주체의 시작 검사를 생략해 N^2 탐색 폭발을 줄인다.
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

		// [역할] Broad-phase 후보 수집: 그리드 쿼리로 srcRect 주변 객체만 가져온다.
		// [최적화] 전체 월드 순회 대신 지역 후보만 검사해 내로우페이즈 호출 수를 줄인다.
		Gdiplus::RectF srcRect = pSrc->GetWorldRect();
		
		m_queryBuffer.clear();
		objMgr->QueryObjectsInRect(srcRect, m_queryBuffer);

		for (GameObject* pDstOwner : m_queryBuffer) {
			if (!pDstOwner || pDstOwner == pSrcOwner || !pDstOwner->IsEnabled()) continue;

			// [역할] Narrow-phase 진입 전 필터: 대상의 메인 콜라이더만 사용한다.
			// [최적화] 다중 콜라이더 전수 검사 대신 핵심 콜라이더 1개만 검사해 비용을 절감한다.
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

	// [역할] Narrow-phase 실제 충돌 판정.
	// [최적화] 타입 조합별 최소 연산 경로(Box-Box, Circle-Circle, Box-Circle)로 분기한다.
	// TODO(선택): 빈번한 조합에서 먼저 AABB 사전 컷을 넣으면 연산량을 더 줄일 수 있다.

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

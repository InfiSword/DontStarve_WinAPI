#include "../../99_Default/pch.h"
#include "ColliderManager.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../02_GameObject/GameObject.h"

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

void ColliderManager::Render(Gdiplus::Graphics* pGraphics)
{
    // 디버그용 콜라이더 박스 그리기 
    if (!pGraphics || !CameraManager::GetInstance()) return;

    Gdiplus::Pen debugPen(Gdiplus::Color(255, 255, 0, 0), 1); 

    for (Collider* pCollider : m_colliders) {
        if (pCollider && pCollider->m_pOwner) {
            // 콜라이더의 월드 좌표를 화면 좌표로 변환
            Gdiplus::PointF screenTopLeft = CameraManager::GetInstance()->WorldToScreen(pCollider->m_boundingBox.left, pCollider->m_boundingBox.top);
            Gdiplus::PointF screenBottomRight = CameraManager::GetInstance()->WorldToScreen(pCollider->m_boundingBox.right, pCollider->m_boundingBox.bottom);

            float scaledWidth = screenBottomRight.X - screenTopLeft.X;
            float scaledHeight = screenBottomRight.Y - screenTopLeft.Y;

            pGraphics->DrawRectangle(&debugPen, Gdiplus::RectF(screenTopLeft.X, screenTopLeft.Y, scaledWidth, scaledHeight));
        }
    }
}

void ColliderManager::Release()
{
    for (Collider* pCollider : m_colliders) {
        SafeDelete(pCollider); 
    }
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
    // 리스트에서 콜라이더 제거 및 삭제
    m_colliders.erase(std::remove(m_colliders.begin(), m_colliders.end(), pCollider), m_colliders.end());
    SafeDelete(pCollider);
}

GameObject* ColliderManager::CheckPointCollision(POINT screenPos)
{
    if (!CameraManager::GetInstance()) return nullptr;

    // 화면 좌표를 월드 좌표로 변환
    Gdiplus::PointF worldPos = CameraManager::GetInstance()->ScreenToWorld(screenPos.x, screenPos.y);

    for (Collider* pCollider : m_colliders) {
        if (pCollider && pCollider->m_pOwner) {

            RECT worldBoundingBox = pCollider->m_boundingBox;

            RECT actualWorldRect = {
                (int)pCollider->m_pOwner->GetX() + worldBoundingBox.left,
                (int)pCollider->m_pOwner->GetY() + worldBoundingBox.top,
                (int)pCollider->m_pOwner->GetX() + worldBoundingBox.right,
                (int)pCollider->m_pOwner->GetY() + worldBoundingBox.bottom
            };

            if (PtInRect(&actualWorldRect, { (int)worldPos.X, (int)worldPos.Y })) {
                return pCollider->m_pOwner;
            }
        }
    }
    return nullptr;
}

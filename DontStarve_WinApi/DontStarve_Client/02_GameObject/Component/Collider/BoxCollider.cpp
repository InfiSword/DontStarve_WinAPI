#include "99_Default/pch.h"
#include "BoxCollider.h"
#include "CircleCollider.h"
#include "../Transform/Transform.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../GameObject.h"


BoxCollider::BoxCollider(GameObject* owner)
	: Collider(owner)
{
	// 기본 boundingBox는 나중에 Entity::Init()에서 설정됨
	m_boundingBox = { 0, 0, 0, 0 };
}

bool BoxCollider::IntersectsCollider(const Collider* other) const
{
	if (!IsEnabled() || !other || !other->IsEnabled()) {
		return false;
	}

	// 다른 콜라이더가 BoxCollider인지 CircleCollider인지 확인
	const BoxCollider* boxCollider = dynamic_cast<const BoxCollider*>(other);
	const CircleCollider* circleCollider = dynamic_cast<const CircleCollider*>(other);

	if (boxCollider) {
		// BoxCollider와의 충돌 검사
		RECT thisBox = GetWorldBoundingBox();
		RECT otherBox = boxCollider->GetWorldBoundingBox();
		return !(thisBox.right < otherBox.left || thisBox.left > otherBox.right ||
			thisBox.bottom < otherBox.top || thisBox.top > otherBox.bottom);
	}
	else if (circleCollider) {
		// CircleCollider와의 충돌 검사
		// 원과 사각형의 충돌: 사각형에서 원의 중심에 가장 가까운 점을 찾아 거리 계산
		RECT thisBox = GetWorldBoundingBox();
		float worldCenterX, worldCenterY, worldRadius;
		circleCollider->GetWorldCircle(worldCenterX, worldCenterY, worldRadius);

		// 사각형에서 가장 가까운 점 찾기
		float closestX = (std::max)((float)thisBox.left, (std::min)(worldCenterX, (float)thisBox.right));
		float closestY = (std::max)((float)thisBox.top, (std::min)(worldCenterY, (float)thisBox.bottom));

		// 원의 중심과 가장 가까운 점 사이의 거리
		float dx = worldCenterX - closestX;
		float dy = worldCenterY - closestY;
		float distance = sqrtf(dx * dx + dy * dy);

		return distance <= worldRadius;
	}

	return false;
}

bool BoxCollider::ContainsPoint(float worldX, float worldY) const
{
	const RECT& box = GetWorldBoundingBox();
	return worldX >= (float)box.left && worldX < (float)box.right
		&& worldY >= (float)box.top && worldY < (float)box.bottom;
}

void BoxCollider::GetCenterWorld(float& outX, float& outY) const
{
	const RECT& box = GetWorldBoundingBox();
	outX = ((float)box.left + (float)box.right) * 0.5f;
	outY = ((float)box.top + (float)box.bottom) * 0.5f;
}

void BoxCollider::SetObjectCollider(int offsetX, int offsetY, int width, int height)
{
	m_boundingBox = {
		offsetX,
		offsetY,
		offsetX + width,
		offsetY + height
	};
}

RECT BoxCollider::GetWorldBoundingBox() const
{
	GameObject* owner = GetOwner();
	Transform* transform = owner ? owner->GetComponent<Transform>() : nullptr;

	float ox = transform ? transform->GetX() : 0.0f;
	float oy = transform ? transform->GetY() : 0.0f;

	RECT worldBox;
	worldBox.left   = static_cast<LONG>(ox + static_cast<float>(m_boundingBox.left));
	worldBox.top    = static_cast<LONG>(oy + static_cast<float>(m_boundingBox.top));
	worldBox.right  = static_cast<LONG>(ox + static_cast<float>(m_boundingBox.right));
	worldBox.bottom = static_cast<LONG>(oy + static_cast<float>(m_boundingBox.bottom));

	return worldBox;
}

void BoxCollider::RenderGizmo()
{
	/*if (!IsEnabled()) {
		return;
	}*/

	RenderManager* renderManager = RenderManager::GetInstance();
	CameraManager* cameraManager = CameraManager::GetInstance();

	// 월드 좌표로 변환된 boundingBox 가져오기
	RECT worldBox = GetWorldBoundingBox();

	// 월드 좌표를 화면 좌표로 변환
	Gdiplus::PointF screenTopLeft = cameraManager->WorldToScreen((float)worldBox.left, (float)worldBox.top);
	Gdiplus::PointF screenBottomRight = cameraManager->WorldToScreen((float)worldBox.right, (float)worldBox.bottom);

	renderManager->AddDrawRectCommand(Gdiplus::RectF(screenTopLeft.X, screenTopLeft.Y, screenBottomRight.X - screenTopLeft.X, screenBottomRight.Y - screenTopLeft.Y), Gdiplus::Color(255, 0, 0), 2.0f, LAYER_DEBUG_OVERLAY, 10.0f);

}

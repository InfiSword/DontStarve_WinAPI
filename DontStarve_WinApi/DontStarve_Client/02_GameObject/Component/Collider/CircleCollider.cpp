#include "99_Default/pch.h"
#include "CircleCollider.h"
#include "BoxCollider.h"
#include "../Transform/Transform.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../GameObject.h"


CircleCollider::CircleCollider(GameObject* owner, float centerX, float centerY, float radius)
	: Collider(owner), m_centerX(centerX), m_centerY(centerY), m_radius(radius)
{
}

bool CircleCollider::IntersectsCollider(const Collider* other) const
{
	if (!IsEnabled() || !other || !other->IsEnabled()) {
		return false;
	}

	const BoxCollider* boxCollider = dynamic_cast<const BoxCollider*>(other);
	const CircleCollider* circleCollider = dynamic_cast<const CircleCollider*>(other);

	if (boxCollider) {

		RECT otherBox = boxCollider->GetWorldBoundingBox();
		float thisCenterX, thisCenterY, thisRadius;
		GetWorldCircle(thisCenterX, thisCenterY, thisRadius);

		// 사각형에서 가장 가까운 점 찾기
		float closestX = (std::max)((float)otherBox.left, (std::min)(thisCenterX, (float)otherBox.right));
		float closestY = (std::max)((float)otherBox.top, (std::min)(thisCenterY, (float)otherBox.bottom));

		// 원의 중심과 가장 가까운 점 사이의 거리
		float dx = thisCenterX - closestX;
		float dy = thisCenterY - closestY;
		float distance = sqrtf(dx * dx + dy * dy);

		return distance <= thisRadius;
	}
	else if (circleCollider) {
		// CircleCollider와의 충돌 검사
		float thisCenterX, thisCenterY, thisRadius;
		GetWorldCircle(thisCenterX, thisCenterY, thisRadius);

		float otherCenterX, otherCenterY, otherRadius;
		circleCollider->GetWorldCircle(otherCenterX, otherCenterY, otherRadius);

		// 두 원의 중심 사이의 거리
		float dx = thisCenterX - otherCenterX;
		float dy = thisCenterY - otherCenterY;
		float distance = sqrtf(dx * dx + dy * dy);

		// 두 반지름의 합보다 거리가 작으면 충돌
		return distance <= (thisRadius + otherRadius);
	}

	return false;
}

bool CircleCollider::ContainsPoint(float worldX, float worldY) const
{
	float centerX, centerY, radius;
	GetWorldCircle(centerX, centerY, radius);
	float dx = worldX - centerX;
	float dy = worldY - centerY;
	return (dx * dx + dy * dy) <= (radius * radius);
}

void CircleCollider::GetCenterWorld(float& outX, float& outY) const
{
	float radius;
	GetWorldCircle(outX, outY, radius);
}

void CircleCollider::SetObjectCollider(float centerX, float centerY, float radius)
{
	m_centerX = centerX;
	m_centerY = centerY;
	m_radius = radius;
}

void CircleCollider::GetWorldCircle(float& centerX, float& centerY, float& radius) const
{
	GameObject* owner = GetOwner();
	Transform* transform = owner ? owner->GetComponent<Transform>() : nullptr;
	float ox = transform ? transform->GetX() : 0.0f;
	float oy = transform ? transform->GetY() : 0.0f;
	float sx = transform ? transform->GetScaleX() : 1.0f;
	float sy = transform ? transform->GetScaleY() : 1.0f;

	centerX = ox + m_centerX * sx;
	centerY = oy + m_centerY * sy;
	radius = m_radius * (sx + sy) * 0.5f; // 평균 스케일 적용
}

void CircleCollider::RenderGizmo()
{
	if (!IsEnabled()) {
		return;
	}

	RenderManager* renderManager = RenderManager::GetInstance();
	CameraManager* cameraManager = CameraManager::GetInstance();

	float worldCenterX, worldCenterY, worldRadius;
	GetWorldCircle(worldCenterX, worldCenterY, worldRadius);

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(worldCenterX, worldCenterY);

	Gdiplus::PointF screenRight = cameraManager->WorldToScreen(worldCenterX + worldRadius, worldCenterY);
	float screenRadius = abs(screenRight.X - screenCenter.X);

	Gdiplus::RectF gizmoRect(
		screenCenter.X - screenRadius,
		screenCenter.Y - screenRadius,
		screenRadius * 2.0f,
		screenRadius * 2.0f
	);

	Gdiplus::Color gizmoColor(255, 255, 0, 0); 
	Gdiplus::Color bgColor(30, 255, 0, 0); 

	renderManager->AddFillRectangleCommand(gizmoRect, bgColor, LAYER_DEBUG_OVERLAY, 9998.0f);

	renderManager->AddDrawRectCommand(gizmoRect, gizmoColor, 2.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
}

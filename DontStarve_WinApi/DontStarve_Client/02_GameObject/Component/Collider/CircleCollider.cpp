#include "../../../99_Default/pch.h"
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

	// 알 수 없는 타입이면 boundingBox로 검사 (원과 사각형 충돌 검사)
	RECT otherBox = other->GetWorldBoundingBox();
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

RECT CircleCollider::GetWorldBoundingBox() const
{
	return {
		(int)(m_centerX - m_radius),
		(int)(m_centerY - m_radius),
		(int)(m_centerX + m_radius),
		(int)(m_centerY + m_radius)
	};
}

void CircleCollider::SetCircle(float centerX, float centerY, float radius)
{
	m_centerX = centerX;
	m_centerY = centerY;
	m_radius = radius;
}

void CircleCollider::GetWorldCircle(float& centerX, float& centerY, float& radius) const
{
	GameObject* owner = GetOwner();
	if (!owner) {
		centerX = m_centerX;
		centerY = m_centerY;
		radius = m_radius;
		return;
	}

	Transform* transform = owner->GetComponent<Transform>();
	centerX = transform->GetX() + m_centerX;
	centerY = transform->GetY() + m_centerY;
	radius = m_radius;
}

void CircleCollider::RenderGizmo()
{
	// 비활성화된 콜라이더는 그리지 않음
	if (!IsEnabled()) {
		return;
	}

	RenderManager* renderManager = RenderManager::GetInstance();
	CameraManager* cameraManager = CameraManager::GetInstance();

	if (!renderManager || !cameraManager) {
		return;
	}

	// 월드 좌표로 변환된 원의 중심점과 반지름
	float worldCenterX, worldCenterY, worldRadius;
	GetWorldCircle(worldCenterX, worldCenterY, worldRadius);

	// 월드 좌표를 화면 좌표로 변환
	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(worldCenterX, worldCenterY);

	// 반지름의 오른쪽 끝점의 월드 좌표를 계산하여 화면 좌표로 변환한 후 화면 반지름 계산
	Gdiplus::PointF screenRight = cameraManager->WorldToScreen(worldCenterX + worldRadius, worldCenterY);
	float screenRadius = abs(screenRight.X - screenCenter.X);

	// 원을 감싸는 사각형으로 Gizmo 그리기 (원 그리기는 복잡하므로 사각형으로 근사)
	Gdiplus::RectF gizmoRect(
		screenCenter.X - screenRadius,
		screenCenter.Y - screenRadius,
		screenRadius * 2.0f,
		screenRadius * 2.0f
	);

	Gdiplus::Color gizmoColor(255, 255, 0, 0); // 빨간색
	Gdiplus::Color bgColor(30, 255, 0, 0); // 반투명 빨간색

	// 반투명 배경
	renderManager->AddFillRectangleCommand(gizmoRect, bgColor, LAYER_DEBUG_OVERLAY, 9998.0f);

	// 외곽선 (원을 사각형으로 근사)
	renderManager->AddDrawCommand(gizmoRect, gizmoColor, 2.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
}

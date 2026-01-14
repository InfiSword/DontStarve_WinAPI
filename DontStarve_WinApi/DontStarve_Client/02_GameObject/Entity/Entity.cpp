#include "../../99_Default/pch.h"
#include "Entity.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Collider/BoxCollider.h"
#include "../../02_GameObject/Component/Collider/CircleCollider.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Entity::Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
	const std::wstring& resourcePath, const std::wstring& imageName, bool isActive, bool isInteractive)
	:GameObject(type, id, resourcePath, imageName, isActive, isInteractive),
	m_animator(nullptr)
{
	// Transform 컴포넌트 추가
	Transform* transform = AddComponent<Transform>();
	transform->SetPosition(x, y);
	transform->SetPivot(pivotX, pivotY);
	transform->SetDirection(_dir);

	// SpriteRenderer 컴포넌트 추가
	SpriteRenderer* spriteRenderer = AddComponent<SpriteRenderer>();
	spriteRenderer->SetLayer(LAYER_WORLD_OBJECT);
	if (!imageName.empty())
	{
		// 리소스 매니저를 통해 비트맵 로드
		ResourceManager* resourceManager = ResourceManager::GetInstance();
		if (resourceManager)
		{
			std::wstring fullPath = resourceManager->BuildObjectResourcePath(id, L"", imageName);
			Gdiplus::Bitmap* bitmap = resourceManager->LoadBitmap(fullPath);
			if (bitmap)
			{
				float width = static_cast<float>(bitmap->GetWidth());
				float height = static_cast<float>(bitmap->GetHeight());
				Gdiplus::RectF fullRect(0.0f, 0.0f, width, height);

				// Transform의 피벗을 그대로 SpriteRenderer에도 사용
				spriteRenderer->SetSprite(bitmap, fullRect, transform->GetPivotX(), transform->GetPivotY());
			}
		}

		// Transform은 이제 Scale만 관리 (기본값 1.0f)
		// 크기는 sprite의 실제 크기를 사용하므로 Transform에 설정할 필요 없음
	}
}

Entity::~Entity()
{
}

void Entity::Init()
{
	GameObject::Init();

	// Transform 컴포넌트 캐싱
	Transform* transform = GetComponent<Transform>();
	SpriteRenderer* spriteRenderer = GetComponent<SpriteRenderer>();	
	if (!transform || !spriteRenderer) return;

	// ResourceManager에서 콜라이더 정보 가져오기
	ResourceManager* resourceManager = ResourceManager::GetInstance();
	const GameObjectData* resourceData = resourceManager ? resourceManager->GetObjectResourceInfo(m_id) : nullptr;

	ColliderType colliderType = COLLIDER_BOX; // 기본값
	if (resourceData && resourceData->hasCollider) {
		colliderType = resourceData->colliderType;
	}

	if (colliderType == COLLIDER_BOX) {
		// BoxCollider 컴포넌트 추가
		BoxCollider* collider = AddComponent<BoxCollider>();

		if (collider && resourceData && resourceData->hasCollider) {
			// ResourceManager의 GameObjectData에서 콜라이더 정보를 BoxCollider에 설정
			collider->SetMapColliderData(
				resourceData->hasCollider,
				resourceData->colliderOffsetX,
				resourceData->colliderOffsetY,
				resourceData->colliderWidth,
				resourceData->colliderHeight
			);
			collider->SetBoundingBox(
				resourceData->colliderOffsetX,
				resourceData->colliderOffsetY,
				resourceData->colliderWidth,
				resourceData->colliderHeight
			);
		}
	}
	else if (colliderType == COLLIDER_CIRCLE) {
		// CircleCollider 컴포넌트 추가
		CircleCollider* collider = AddComponent<CircleCollider>();

		if (collider && resourceData && resourceData->hasCollider) {
			// ResourceManager의 GameObjectData에서 콜라이더 정보를 CircleCollider에 설정
			collider->SetMapColliderData(
				resourceData->hasCollider,
				(int)resourceData->colliderCenterX,
				(int)resourceData->colliderCenterY,
				(int)(resourceData->colliderRadius * 2.0f),
				(int)(resourceData->colliderRadius * 2.0f)
			);
			collider->SetCircle(
				resourceData->colliderCenterX,
				resourceData->colliderCenterY,
				resourceData->colliderRadius
			);
		}
	}
}

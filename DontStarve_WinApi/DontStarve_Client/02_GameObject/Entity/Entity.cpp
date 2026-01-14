#include "99_Default/pch.h"
#include "Entity.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Collider/BoxCollider.h"
#include "../../02_GameObject/Component/Collider/CircleCollider.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Entity::Entity(GameObjectType type, GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
	const std::wstring& imageName, bool isActive, bool isInteractive)
	:GameObject(type, id, L"", imageName, isActive, isInteractive),
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
		ResourceManager* pRM = ResourceManager::GetInstance();
		std::wstring fullPath = pRM->BuildObjectResourcePath(id, L"", imageName);

		if (!fullPath.empty()) {
			if (auto sprite = pRM->LoadSprite(fullPath)) {
				spriteRenderer->SetSprite(sprite);
			}
		}
	}
}

Entity::~Entity()
{
}

void Entity::Init()
{
	GameObject::Init();

	// Transform 컴포넌트 캐싱
	this->transform = GetComponent<Transform>();
	this->spriteRenderer = GetComponent<SpriteRenderer>();
}

GameObjectID Entity::GetDropItemID() const
{
	return m_dropItemID;
}

int Entity::GetDropItemCount() const
{
	return m_dropItemCount;
}

void Entity::SetDropItem(GameObjectID itemID, int count)
{
	m_dropItemID = itemID;
	m_dropItemCount = count;
}

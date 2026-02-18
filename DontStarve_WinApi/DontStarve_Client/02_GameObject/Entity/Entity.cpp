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
	const std::wstring& baseDir, const std::wstring& imageName, bool isActive, bool isInteractive)
	:GameObject(type, id, L"", imageName, isActive, isInteractive),
	m_animator(nullptr)
{
	// Transform 컴포넌트 추가
	Transform* transform = AddComponent<Transform>();
	transform->SetPosition(x, y);
	transform->SetPivot(pivotX, pivotY);
	transform->SetDirection(_dir);

	// SpriteRenderer 컴포넌트 추가 (baseDir + imageName으로 전체 경로 구성)
	SpriteRenderer* spriteRenderer = AddComponent<SpriteRenderer>();
	spriteRenderer->SetLayer(LAYER_WORLD_OBJECT);
	if (!imageName.empty())
	{
		ResourceManager* pRM = ResourceManager::GetInstance();
		std::wstring fullPath;
		if (baseDir.empty()) {
			fullPath = imageName;
		} else {
			fullPath = baseDir;
			if (!fullPath.empty() && fullPath.back() != L'\\' && fullPath.back() != L'/') {
				fullPath += L"\\";
			}
			fullPath += imageName;
		}
		if (auto sprite = pRM->LoadSprite(fullPath)) {
			spriteRenderer->SetSprite(sprite);
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

void Entity::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	GameObject::Update(deltaTime);
}

void Entity::Release()
{
	// Entity 전용 정리 작업
	m_animator = nullptr;
	transform = nullptr;
	spriteRenderer = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	GameObject::Release();
}
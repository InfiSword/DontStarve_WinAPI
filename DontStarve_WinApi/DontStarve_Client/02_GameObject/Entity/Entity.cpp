#include "99_Default/pch.h"
#include "Entity.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/RenderManager/RenderManager.h"
#include "../../03_Animation/Animator.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Collider/BoxCollider.h"
#include "../../02_GameObject/Component/Collider/CircleCollider.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Entity::Entity(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction _dir,
	const std::wstring& baseDir, const std::wstring& imageName, bool isActive, bool isInteractive, ColliderType colliderType)
	:GameObject(id, L"", imageName, isActive, isInteractive),
	m_dropItemID(GOID_NONE),
	m_dropItemCount(0),
	m_isDead(false),
	m_hp(100),
	m_maxHp(100),
	m_state(0),
	m_animator(nullptr),
	m_entityCollider(nullptr),
	m_colliderType(colliderType)
{
	// Transform 컴포넌트 추가
	Transform* transform = AddComponent<Transform>();
	transform->SetPosition(x, y);
	transform->SetDirection(_dir);

	// SpriteRenderer 컴포넌트 추가
	SpriteRenderer* spriteRenderer = AddComponent<SpriteRenderer>();
	spriteRenderer->SetLayer(LAYER_WORLD_OBJECT);
	if (!imageName.empty())
	{
		ResourceManager* pRM = ResourceManager::GetInstance();
		std::wstring fullPath = ResourcePathUtils::BuildResourcePath(baseDir, imageName);
		if (!fullPath.empty()) {
			// 로드 시점에 전달받은 피벗 적용
			if (auto sprite = pRM->LoadSprite(fullPath, { pivotX, pivotY })) {
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

	// 콜라이더 타입에 따라 생성 및 캐싱
	if (m_colliderType == COLLIDER_BOX)
	{
		m_entityCollider = GetComponent<BoxCollider>();
	}
	else if (m_colliderType == COLLIDER_CIRCLE)
	{
		m_entityCollider = GetComponent<CircleCollider>();
	}
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

bool Entity::OnInteraction(GameObject* obj)
{
	if (m_isDead) return false;
	return GameObject::OnInteraction(obj);
}

void Entity::ChangeState(int newState, bool restart)
{
	m_state = newState;
	if (m_animator && transform) {
		m_animator->SetState(m_state, transform->GetDirection(), restart);
	}
}

void Entity::Damaged(int damage)
{
	if (m_isDead || damage <= 0) return;

	m_hp -= damage;
	if (m_hp <= 0) {
		m_hp = 0;
		m_isDead = true;

		// 죽었을 때 콜라이더를 즉시 비활성화하여 상호작용(클릭 등) 방지
		if (m_entityCollider) m_entityCollider->SetColliderEnabled(false);

		Die();
	}
}

void Entity::Render()
{
	if (!IsEnabled() || !transform) return;

	if (spriteRenderer && spriteRenderer->IsEnabled()) 
	{
		spriteRenderer->Render();
	}
}

void Entity::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	GameObject::Update(deltaTime);
}

void Entity::Release()
{
	m_animator = nullptr;
	transform = nullptr;
	spriteRenderer = nullptr;
	m_entityCollider = nullptr;

	GameObject::Release();
}

void Entity::ClampPositionToMapBounds()
{
	if (!transform) return;

	float x = transform->GetX();
	float y = transform->GetY();

	const auto bounds = GetBounds();

	const float mapMinX = 0.0f;
	const float mapMinY = 0.0f;
	const float mapMaxX = static_cast<float>(MAP_WIDTH * TILE_SIZE);
	const float mapMaxY = static_cast<float>(MAP_HEIGHT * TILE_SIZE);

	float offsetX = 0.0f;
	float offsetY = 0.0f;

	if (bounds.X < mapMinX) {
		offsetX = mapMinX - bounds.X;
	}
	else if (bounds.X + bounds.Width > mapMaxX) {
		offsetX = mapMaxX - (bounds.X + bounds.Width);
	}

	if (bounds.Y < mapMinY) {
		offsetY = mapMinY - bounds.Y;
	}
	else if (bounds.Y + bounds.Height > mapMaxY) {
		offsetY = mapMaxY - (bounds.Y + bounds.Height);
	}

	if (offsetX != 0.0f || offsetY != 0.0f) {
		transform->SetPosition(x + offsetX, y + offsetY);
	}
}


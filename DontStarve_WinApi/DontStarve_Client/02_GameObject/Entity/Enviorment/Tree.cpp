#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../Component/Collider/BoxCollider.h"
#include "../../Component/Transform/Transform.h"
#include "Tree.h"

Tree::Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName, true, true),
	m_hp(100), m_baseX(0.0f), m_baseY(0.0f),
	m_shakeDuration(0.5f), m_shakeAmount(14.0f), m_shakeSpeed(40.0f), m_isShaking(false)
{
	maxHp = m_hp;
}

Tree::~Tree() {}

void Tree::Init()
{
	Entity::Init();
	SetDropItem(GOID_ITEM_NORMAL_TREE_LOG, 1);

	const int TREE_COLLIDER_WIDTH = 64;
	const int TREE_COLLIDER_HEIGHT = 128;
	BoxCollider* collider = AddComponent<BoxCollider>();
	if (collider) {
		collider->SetBoundingBox(-TREE_COLLIDER_WIDTH / 2, -TREE_COLLIDER_HEIGHT, TREE_COLLIDER_WIDTH, TREE_COLLIDER_HEIGHT);
	}
}

void Tree::LateInit()
{
	if (transform) {
		m_baseX = transform->GetX();
		m_baseY = transform->GetY();
	}
}

void Tree::Update(float deltaTime)
{
	if (m_isDead) return;
	Entity::Update(deltaTime);
}

void Tree::LateUpdate()
{
}

void Tree::Release()
{
	Entity::Release();
}

bool Tree::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Tree::Damaged(int damage)
{
	if (m_isDead) return;

	m_hp -= damage;

	// 쉐이킹 중이면 원래 위치로 복원한 뒤 그 위치를 새 기준점으로 사용
	if (m_isShaking && transform) {
		transform->SetPosition(m_baseX, m_baseY);
	}
	else if (transform) {
		m_baseX = transform->GetX();
		m_baseY = transform->GetY();
	}

	StopAllCoroutines();
	m_isShaking = true;

	float baseX = m_baseX;
	float baseY = m_baseY;
	float elapsed = 0.0f;
	float duration = m_shakeDuration;
	float amount = m_shakeAmount;
	float speed = m_shakeSpeed;
	Transform* tr = transform;
	bool shouldDie = (m_hp <= 0);

	StartCoroutine([=](float dt) mutable -> bool {
		elapsed += dt;
		if (elapsed >= duration) {
			if (tr) tr->SetPosition(baseX, baseY);
			m_isShaking = false;
			if (shouldDie) {
				m_isDead = true;
				Die();
			}
			return false;
		}
		if (tr) {
			float offsetX = sinf(elapsed * speed) * amount;
			float offsetY = cosf(elapsed * speed) * amount;
			tr->SetPosition(baseX + offsetX, baseY + offsetY);
		}
		return true;
	});
}

void Tree::Die()
{
	// 원래 위치 복원 (코루틴에서 호출될 때는 이미 복원되어 있으나, 직접 호출 경우 대비)
	m_isShaking = false;
	if (transform)
		transform->SetPosition(m_baseX, m_baseY);

	float tx = transform ? transform->GetX() : 0.0f;
	float ty = transform ? transform->GetY() : 0.0f;

	ObjectManager* objMgr = ObjectManager::GetInstance();
	if (objMgr) {
		GameObjectID dropItemID = GetDropItemID();
		int dropCount = GetDropItemCount();
		if (dropItemID != GOID_NONE && dropCount > 0) {
			const float spreadRadius = 20.0f;
			for (int i = 0; i < dropCount; ++i) {
				float angle = (float)i / dropCount * 2.0f * 3.14159f;
				float offsetX = cosf(angle) * spreadRadius;
				float offsetY = sinf(angle) * spreadRadius;
				objMgr->CreateGameObject(dropItemID, tx + offsetX, ty + offsetY, nullptr, true);
			}
		}
		objMgr->RemoveGameObject(this);
	}
}

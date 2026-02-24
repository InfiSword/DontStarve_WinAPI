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
	// 부모 호출 필수: GameObject::LateUpdate()에서 UpdateCoroutines()가 실행되어 쉐이킹 코루틴이 동작함
	Entity::LateUpdate();
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

	// 연타 대응: 이미 쉐이킹 중이면 현재 오프셋 위치가 아닌 기준점(m_baseX/Y)으로 복원 후, 기존 코루틴 제거하고 새 쉐이킹 시작 (기준점은 갱신하지 않음)
	if (m_isShaking && transform) {
		transform->SetPosition(m_baseX, m_baseY);
	}
	else if (transform) {
		m_baseX = transform->GetX();
		m_baseY = transform->GetY();
	}

	// HP 0 이면 쉐이킹 없이 즉시 제거 및 통나무 드롭
	if (m_hp <= 0) {
		if (transform) transform->SetPosition(m_baseX, m_baseY);
		m_isShaking = false;
		m_isDead = true;
		Die();
		return;
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

	StartCoroutine([=](float dt) mutable -> bool {
		elapsed += dt;
		if (elapsed >= duration) {
			if (tr) tr->SetPosition(baseX, baseY);
			m_isShaking = false;
			return false;
		}
		// 좌우로만 흔들기 (X만 오프셋)
		if (tr) {
			float offsetX = sinf(elapsed * speed) * amount;
			tr->SetPosition(baseX + offsetX, baseY);
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

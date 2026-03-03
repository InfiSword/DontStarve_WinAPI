#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../Component/Transform/Transform.h"
#include "Tree.h"

Tree::Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Entity(id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName, true, true)
	, m_treeState(TreeState::IDLE)
	, m_hp(100), m_baseX(0.0f), m_baseY(0.0f),
	m_shakeDuration(0.5f), m_shakeAmount(14.0f), m_shakeSpeed(40.0f), m_isShaking(false)
{
	m_type = GO_TYPE_NATURAL_ENVIRONMENT;
	SetDropItem(GOID_ITEM_NORMAL_TREE_LOG, 1); // 기본 드롭템
}

Tree::~Tree() {}

void Tree::Init()
{
	Entity::Init();

	if (transform)
	{
		m_baseX = transform->GetX();
		m_baseY = transform->GetY();
	}
}

void Tree::Update(float deltaTime)
{
	Entity::Update(deltaTime);
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
	if (m_treeState != TreeState::IDLE) return;

	m_hp -= damage;

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

		if (tr) {
			// 시간에 따라 감쇄하는 쉐이킹 효과
			float currentAmount = amount * (1.0f - (elapsed / duration));
			float offsetX = sinf(elapsed * speed) * currentAmount;
			tr->SetPosition(baseX + offsetX, baseY);
		}
		return true;
	});
}

void Tree::Die()
{
	m_isShaking = false;
	if (transform)
		transform->SetPosition(m_baseX, m_baseY);

	ObjectManager* objMgr = ObjectManager::GetInstance();
	if (objMgr)
	{
		GameObjectID dropItemID = GetDropItemID();
		int count = GetDropItemCount();

		if (dropItemID != GOID_NONE && transform)
		{
			float tx = transform->GetX();
			float ty = transform->GetY();

			for (int i = 0; i < count; ++i)
			{
				float angle = (rand() / (float)RAND_MAX) * 6.28f;
				float spreadRadius = 30.0f + (rand() / (float)RAND_MAX) * 40.0f;
				float offsetX = cosf(angle) * spreadRadius;
				float offsetY = sinf(angle) * spreadRadius;
				objMgr->CreateGameObject(dropItemID, tx + offsetX, ty + offsetY, nullptr, true);
			}
		}
		objMgr->RemoveGameObject(this);
	}
}

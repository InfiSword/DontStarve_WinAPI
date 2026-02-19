#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../Component/Collider/BoxCollider.h"
#include "../../Component/Transform/Transform.h"
#include "Tree.h"

Tree::Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName, true, true),
	m_hp(100), m_hitAnimTimer(0.0f), m_state(TreeState::TREE_IDLE), m_baseX(0.0f), m_baseY(0.0f)
{
	maxHp = m_hp;
}

Tree::~Tree() {}

void Tree::Init()
{
	Entity::Init();
	SetDropItem(GOID_ITEM_NORMAL_TREE_LOG, 1);

	// 클릭/상호작용 검출용 BoxCollider (FindInteractableObjectAtPosition에서 필요)
	const int TREE_COLLIDER_WIDTH = 64;
	const int TREE_COLLIDER_HEIGHT = 128;
	BoxCollider* collider = AddComponent<BoxCollider>();
	if (collider) {
		// 피벗(하단 중심) 기준: 왼쪽 -width/2, 위쪽 -height
		collider->SetBoundingBox(-TREE_COLLIDER_WIDTH / 2, -TREE_COLLIDER_HEIGHT, TREE_COLLIDER_WIDTH, TREE_COLLIDER_HEIGHT);
	}
}

void Tree::LateInit()
{
	// 초기 기준 위치 저장 (셰이킹용)
	if (transform) {
		m_baseX = transform->GetX();
		m_baseY = transform->GetY();
	}
}

void Tree::Update(float deltaTime)
{
	if (m_isDead) return;  // 죽은 상태면 Update 중단

	if (m_state == TreeState::TREE_CHOP) {
		m_hitAnimTimer += deltaTime;
		// 셰이킹: 기준 위치 + sin/cos 오프셋
		const float shakeAmount = 2.0f;
		const float shakeSpeed = 40.0f;
		float offsetX = sinf(m_hitAnimTimer * shakeSpeed) * shakeAmount;
		float offsetY = cosf(m_hitAnimTimer * shakeSpeed) * shakeAmount;
		if (transform) {
			transform->SetPosition(m_baseX + offsetX, m_baseY + offsetY);
		}
		if (m_hitAnimTimer >= 0.6f) {
			m_state = TreeState::TREE_IDLE;
			m_hitAnimTimer = 0.0f;
			if (transform) transform->SetPosition(m_baseX, m_baseY);
		}
	}
	Entity::Update(deltaTime);
}

void Tree::LateUpdate()
{
}

void Tree::Release()
{
	// Tree 전용 정리 작업
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Entity::Release();
}

bool Tree::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}


void Tree::Damaged(int damage)
{
	if (m_isDead) return;  // 이미 죽은 상태면 데미지 무시

	m_hp -= damage;
	// 셰이킹용 기준 위치 저장 (원래 위치 기준, 셰이킹 중이면 원래 위치로 복원)
	if (transform) {
		// 셰이킹 중이면 원래 위치로 복원 후 기준 위치 저장
		if (m_state == TreeState::TREE_CHOP) {
			transform->SetPosition(m_baseX, m_baseY);
		}
		// 기준 위치 업데이트 (현재 위치가 기준 위치)
		m_baseX = transform->GetX();
		m_baseY = transform->GetY();
	}
	m_state = TreeState::TREE_CHOP;
	m_hitAnimTimer = 0.0f;  // 셰이킹 타이머 리셋

	if (m_hp <= 0) {
		m_isDead = true;
		Die();  // 죽음 판정 시 Die() 호출
	}
}

void Tree::Die()
{
	// 통나무를 나무 원래 위치에 생성 후 나무 제거
	// 셰이킹 중이면 원래 위치로 복원
	if (transform && m_state == TreeState::TREE_CHOP) {
		transform->SetPosition(m_baseX, m_baseY);
	}
	
	float tx = transform ? transform->GetX() : 0.0f;
	float ty = transform ? transform->GetY() : 0.0f;
	
	ObjectManager* objMgr = ObjectManager::GetInstance();
	if (objMgr) {
		// 드롭 아이템 수만큼 통나무 생성
		GameObjectID dropItemID = GetDropItemID();
		int dropCount = GetDropItemCount();
		if (dropItemID != GOID_NONE && dropCount > 0) {
			// 여러 개의 아이템을 약간씩 떨어뜨려 배치
			const float spreadRadius = 20.0f;  // 아이템 간 간격
			for (int i = 0; i < dropCount; ++i) {
				float angle = (float)i / dropCount * 2.0f * 3.14159f;  // 원형 배치
				float offsetX = cosf(angle) * spreadRadius;
				float offsetY = sinf(angle) * spreadRadius;
				objMgr->CreateGameObject(dropItemID, tx + offsetX, ty + offsetY, nullptr, true);
			}
		}
		objMgr->RemoveGameObject(this);
	}
}

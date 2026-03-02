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
	, m_fallTimer(0.0f)
{
	m_hp = 30; // 기본 체력
	m_type = GO_TYPE_NATURAL_ENVIRONMENT;
	SetDropItem(GOID_ITEM_NORMAL_TREE_LOG, 2); // 기본 드롭템
}

Tree::~Tree() {}

void Tree::Init()
{
	Entity::Init();
	m_treeState = TreeState::IDLE;
}

void Tree::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (m_treeState == TreeState::FALL)
	{
		m_fallTimer += deltaTime;
		if (m_fallTimer >= 1.0f)
		{
			m_treeState = TreeState::FALLEN;
			Die();
		}
	}
}

void Tree::Release()
{
	Entity::Release();
}

void Tree::Damaged(int damage)
{
	if (m_treeState != TreeState::IDLE) return;

	m_hp -= damage;
	if (m_hp <= 0)
	{
		m_hp = 0;
		m_treeState = TreeState::FALL;
		m_fallTimer = 0.0f;
		
		// 쓰러지는 효과 (간단히 각도 조절 등 가능)
		if (transform)
		{
			// transform->SetRotation(90.0f); // 나중에 회전 기능 생기면 추가
		}
	}
}

void Tree::Die()
{
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

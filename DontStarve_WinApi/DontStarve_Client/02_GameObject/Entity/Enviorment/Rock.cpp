#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../Component/Transform/Transform.h"
#include "Rock.h"

Rock::Rock(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Entity(id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName, true, true)
	, m_rockState(RockState::INTACT)
{
	m_hp = 40; // 기본 체력
	m_type = GO_TYPE_NATURAL_ENVIRONMENT;
	SetDropItem(GOID_ITEM_NORMAL_ROCK, 3); // 기본 드롭템
}

Rock::~Rock() {}

void Rock::Init()
{
	Entity::Init();
	m_rockState = RockState::INTACT;
}

void Rock::Release()
{
	Entity::Release();
}

void Rock::Damaged(int damage)
{
	if (m_rockState == RockState::DESTROYED) return;

	m_hp -= damage;
	if (m_hp <= 0)
	{
		m_hp = 0;
		m_rockState = RockState::DESTROYED;
		Die();
	}
	else if (m_hp <= 15)
	{
		m_rockState = RockState::BROKEN;
	}
	else if (m_hp <= 30)
	{
		m_rockState = RockState::CRACKED;
	}
}

void Rock::Die()
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
				float spreadRadius = 20.0f + (rand() / (float)RAND_MAX) * 30.0f;
				float offsetX = cosf(angle) * spreadRadius;
				float offsetY = sinf(angle) * spreadRadius;
				objMgr->CreateGameObject(dropItemID, tx + offsetX, ty + offsetY, nullptr, true);
			}
		}
		objMgr->RemoveGameObject(this);
	}
}

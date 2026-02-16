#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "BerryBush.h"

BerryBush::BerryBush(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, imageName, true, true), m_state(GrassState::GRASS_IDLE)
{
	m_dropItemID = GOID_ITEM_BERRY;
	m_dropItemCount = 1;
}

BerryBush::~BerryBush() {}

void BerryBush::Init()
{
	Entity::Init();
}

void BerryBush::LateInit()
{
}

void BerryBush::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Entity::Update(deltaTime);
}

void BerryBush::LateUpdate()
{
}

void BerryBush::Release()
{
	// BerryBush 전용 정리 작업
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Entity::Release();
}

void BerryBush::OnInteraction(GameObject* obj)
{
	// 기본 상호작용 사용
}


void BerryBush::SetDropItem(GameObjectID itemID, int count)
{
	m_dropItemID = itemID;
	m_dropItemCount = count;
}

void BerryBush::Damaged(int damage)
{
}

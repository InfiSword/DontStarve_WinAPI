#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "BerryBush.h"

BerryBush::BerryBush(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName, true, true)
{
	m_type = GO_TYPE_NATURAL_ENVIRONMENT;
	// Struct.h ObjectResourceTable: GOID_ITEM_BERRY (베리 부쉬 상호작용 시 드롭)
	SetDropItem(GOID_ITEM_BERRY, 1);
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

bool BerryBush::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void BerryBush::Damaged(int damage)
{
}

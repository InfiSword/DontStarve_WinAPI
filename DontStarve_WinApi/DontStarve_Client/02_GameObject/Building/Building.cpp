#include "99_Default/pch.h"
#include "Building.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Building::Building(GameObjectID id, float x, float y, float pivotX, float pivotY, 
	Direction _dir, const std::wstring& resourcePath, const std::wstring& imageName, int hp, 
	ColliderType colliderType, bool isActive, bool isInteractive)
    : Entity(id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, colliderType, isActive, isInteractive),
      m_hp(hp), m_maxHp(hp), m_buildingState(BuildingState::NOON)
{
	m_type = GO_TYPE_BUILDING;
}

Building::~Building()
{
}

void Building::Init()
{
    Entity::Init();
}

void Building::LateInit()
{
}

void Building::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Entity::Update(deltaTime);
	
    // 필요 시 상태에 따른 업데이트
}

void Building::LateUpdate()
{
	// 부모 클래스의 LateUpdate() 호출하여 컴포넌트 업데이트
	Entity::LateUpdate();
}

void Building::Release()
{
	// Building 전용 정리 작업
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Entity::Release();
}

void Building::Damaged(int damage)
{
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
       
        m_buildingState = BuildingState::DESTROYED;
    }
    else if (m_hp <= m_maxHp / 2)
    {

        m_buildingState = BuildingState::DAMAGED;
    }
}

void Building::SetTimeState(BuildingState buildingState)
{
    
    m_buildingState = buildingState;
}

BuildingState Building::GetTimeState() const
{
    return m_buildingState;
}

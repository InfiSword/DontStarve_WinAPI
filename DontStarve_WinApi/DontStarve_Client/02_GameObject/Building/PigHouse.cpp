#include "99_Default/pch.h"
#include "PigHouse.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

PigHouse::PigHouse(GameObjectID id, float x, float y, float pivotX, float pivotY, 
    Direction _dir, const std::wstring& resourcePath,
    const std::wstring& imageName, int hp)
    : Building(id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, hp)
{
}

PigHouse::~PigHouse()
{
}

void PigHouse::Init()
{
	// 부모 클래스의 Init() 호출
	Building::Init();
	
	m_buildingState = BUILDING_NOON;
}

void PigHouse::LateInit()
{
}

void PigHouse::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Building::Update(deltaTime);
	
    // 필요 시 상태에 따른 업데이트
}

void PigHouse::LateUpdate()
{
	Building::LateUpdate();
}

void PigHouse::Release()
{
	Building::Release();
}

void PigHouse::Damaged(int damage)
{
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
        m_buildingState = BUILDING_DESTROYED;
        OutputDebugStringW(L"PigHouse: 파괴됨\n");
    }
    else if (m_hp <= m_maxHp / 2)
    {
        m_buildingState = BUILDING_DAMAGED;
        OutputDebugStringW(L"PigHouse: 손상됨\n");
    }
}

void PigHouse::SetTimeState(BuildingState buildingState)
{
    m_buildingState = buildingState;
}

BuildingState PigHouse::GetTimeState() const
{
    return m_buildingState;
}

//std::wstring PigHouse::GetAnimKey(BuildingState state)
//{
//    std::wstring key;
//    if (state == BUILDING_NOON) {
//        key = L"PigHouse_Noon";
//    }
//    else if (state == BUILDING_NIGHT) {
//        key = L"PigHouse_Night";
//    }
//    else if (state == BUILDING_DAMAGED) {
//        key = L"PigHouse_Damaged";
//    }
//    else {
//        key = L"PigHouse_Destroyed";
//    }
//    return key;
//}

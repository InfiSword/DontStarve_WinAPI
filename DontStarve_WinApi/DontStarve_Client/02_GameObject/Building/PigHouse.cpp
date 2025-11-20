#include "../../99_Default/pch.h"
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
    m_buildingState = BUILDING_NOON;
    m_direction = DIR_DOWN;
    
    // �̹��� �ε�
    LoadBitmap();
    
    // ��Ʈ�ʿ��� ũ�� ��������
    if (m_bitmap) {
        this->m_width = static_cast<float>(m_bitmap->GetWidth());
        this->m_height = static_cast<float>(m_bitmap->GetHeight());
    }
}

void PigHouse::LateInit()
{
}

void PigHouse::Update(float deltaTime)
{
    // �ʿ��� ������Ʈ ����
}

void PigHouse::LateUpdate()
{
}

void PigHouse::Release()
{

}

void PigHouse::Damaged(int damage)
{
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
        m_buildingState = BUILDING_DESTROYED;
        OutputDebugStringW(L"PigHouse: �ı���\n");
    }
    else if (m_hp <= m_maxHp / 2)
    {
        m_buildingState = BUILDING_DAMAGED;
        OutputDebugStringW(L"PigHouse: �ջ��\n");
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

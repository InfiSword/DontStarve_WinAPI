#include "../../99_Default/pch.h"
#include "Building.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Building::Building(GameObjectID id, float x, float y, float pivotX, float pivotY, 
    Direction _dir, const std::wstring& resourcePath,
                   const std::wstring& imageName, int hp)
    : Entity<BuildingState>(GOBJ_BUILDING, id, x, y, pivotX, pivotY, _dir, resourcePath, imageName),
      m_hp(hp), m_maxHp(hp), m_buildingState(BUILDING_NOON), pAnimator(nullptr)
{
}

Building::~Building()
{
}

void Building::Init()
{
    SetActive(true);
    m_state = BUILDING_NOON;
    m_buildingState = BUILDING_NOON;
    m_direction = DIR_DOWN;
    
    // 이미지 로드
    LoadBitmap();
    
    // 비트맵에서 크기 가져오기
    if (m_bitmap) {
        this->m_width = static_cast<float>(m_bitmap->GetWidth());
        this->m_height = static_cast<float>(m_bitmap->GetHeight());
    }
}

void Building::LateInit()
{
}

void Building::Update(float deltaTime)
{
    // 필요한 업데이트 로직
}

void Building::LateUpdate()
{
}

void Building::Release()
{
    for (auto& pair : m_animClips)
    {
        SafeDelete(pair.second);
    }
    m_animClips.clear();
}

void Building::Damaged(int damage)
{
    m_hp -= damage;
    if (m_hp <= 0)
    {
        m_hp = 0;
        m_state = BUILDING_DESTROYED;
        m_buildingState = BUILDING_DESTROYED;
    }
    else if (m_hp <= m_maxHp / 2)
    {
        m_state = BUILDING_DAMAGED;
        m_buildingState = BUILDING_DAMAGED;
    }
}

void Building::SetTimeState(BuildingState buildingState)
{
    m_state = buildingState;
    m_buildingState = buildingState;
}

BuildingState Building::GetTimeState() const
{
    return m_buildingState;
}

std::wstring Building::GetAnimKey(BuildingState state)
{
    std::wstring key;
    if (state == BUILDING_NOON) {
        key = L"Building_Noon";
    }
    else if (state == BUILDING_NIGHT) {
        key = L"Building_Night";
    }
    else if (state == BUILDING_DAMAGED) {
        key = L"Building_Damaged";
    }
    else {
        key = L"Building_Destroyed";
    }
    return key;
}

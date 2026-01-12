#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "Rock.h"

Rock::Rock(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName, true, true),
	m_hp(100), m_hitAnimTimer(0.0f), m_state(RockState::ROCK_INTACT)
{
	maxHp = m_hp;
	m_rockCracked = new Gdiplus::Bitmap((resourcePath + L"rock01-1").c_str());
	m_rockBroken = new Gdiplus::Bitmap((resourcePath + L"rock01-2").c_str());
}

Rock::~Rock() {}

void Rock::Init()
{
	Entity::Init();
}

void Rock::LateInit()
{
}

void Rock::Update(float deltaTime)
{
	// 필요한 컴포넌트 업데이트
}

void Rock::LateUpdate()
{
}

void Rock::Release()
{
	// 필요한 정리 작업
}

void Rock::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}


void Rock::Damaged(int damage)
{
	m_hp -= damage;
	
	if (m_hp <= 0) {
		m_state = ROCK_BROKEN;
		OutputDebugStringW(L"Rock: 바위가 부서졌습니다!\n");
	}
	else if (m_hp <= maxHp / 2) {
		m_state = ROCK_CRACKED;
	}
}

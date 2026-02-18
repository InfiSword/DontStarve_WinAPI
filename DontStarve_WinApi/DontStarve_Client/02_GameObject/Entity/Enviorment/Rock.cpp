#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "Rock.h"

Rock::Rock(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName, true, true),
	m_hp(100), m_hitAnimTimer(0.0f), m_state(RockState::ROCK_INTACT)
{
	maxHp = m_hp;
	std::wstring pathCracked = resourcePath;
	if (!pathCracked.empty() && pathCracked.back() != L'\\' && pathCracked.back() != L'/') {
		pathCracked += L"\\";
	}
	pathCracked += L"rock01-1.png";
	
	std::wstring pathBroken = resourcePath;
	if (!pathBroken.empty() && pathBroken.back() != L'\\' && pathBroken.back() != L'/') {
		pathBroken += L"\\";
	}
	pathBroken += L"rock01-2.png";
	
	m_rockCracked = new Gdiplus::Bitmap(pathCracked.c_str());
	m_rockBroken = new Gdiplus::Bitmap(pathBroken.c_str());
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
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Entity::Update(deltaTime);
}

void Rock::LateUpdate()
{
}

void Rock::Release()
{
	// Rock 전용 정리 작업
	if (m_rockCracked) {
		delete m_rockCracked;
		m_rockCracked = nullptr;
	}
	if (m_rockBroken) {
		delete m_rockBroken;
		m_rockBroken = nullptr;
	}
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Entity::Release();
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

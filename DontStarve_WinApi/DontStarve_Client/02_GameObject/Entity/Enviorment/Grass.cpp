#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "Grass.h"

Grass::Grass(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName, true, true), m_state(GrassState::GRASS_IDLE)
{
	m_dropItemID = GOID_ITEM_CUT_NORMAL_GRASS;
	m_dropItemCount = 1;
}

Grass::~Grass() {}

void Grass::Init()
{
	Entity::Init();
	// 비트맵은 생성자에서 이미 로드됨
	// Transform은 이제 Scale만 관리 (기본값 1.0f)
	// 크기는 sprite의 실제 크기를 사용하므로 Transform에 설정할 필요 없음
}

void Grass::LateInit()
{
}

void Grass::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Entity::Update(deltaTime);
}

void Grass::LateUpdate()
{
}

void Grass::Release()
{
	// Grass 전용 정리 작업
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	Entity::Release();
}

void Grass::OnInteraction(GameObject* obj)
{
	// 기본 상호작용 처리
}

void Grass::SetDropItem(GameObjectID itemID, int count)
{
	m_dropItemID = itemID;
	m_dropItemCount = count;
}

void Grass::Damaged(int damage)
{
}

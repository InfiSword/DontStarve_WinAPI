#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "Sapling.h"

Sapling::Sapling(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, imageName, true, true), m_state(GrassState::GRASS_IDLE)
{
	m_dropItemID = GOID_ITEM_NORMAL_TWIGS;
	m_dropItemCount = 1;
}

Sapling::~Sapling() {}

void Sapling::Init()
{
	Entity::Init();
	// 비트맵은 생성자에서 이미 로드됨
	// Transform은 이제 Scale만 관리 (기본값 1.0f)
	// 크기는 sprite의 실제 크기를 사용하므로 Transform에 설정할 필요 없음
}

void Sapling::LateInit()
{
}

void Sapling::Update(float deltaTime)
{
	// 필요한 컴포넌트 업데이트 로직을 여기서 추가
}

void Sapling::LateUpdate()
{
}

void Sapling::Release()
{
	// 필요한 정리 작업
}

void Sapling::OnInteraction(GameObject* obj)
{
	if (IsEnabled()) {
		obj->OnInteraction(this);
	}
}

void Sapling::Damaged(int damage)
{
}

void Sapling::SetDropItem(GameObjectID itemID, int count)
{
	m_dropItemID = itemID;
	m_dropItemCount = count;
}

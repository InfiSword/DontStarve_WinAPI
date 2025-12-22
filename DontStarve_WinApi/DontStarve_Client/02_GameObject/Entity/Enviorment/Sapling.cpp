#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "Sapling.h"

Sapling::Sapling(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName, true, true), m_state(GrassState::GRASS_IDLE)
{
	m_dropItemID = GOID_ITEM_NORMAL_TWIGS;
	m_dropItemCount = 1;
}

Sapling::~Sapling() {}

void Sapling::Init()
{
	// 이미지 로드
	LoadBitmap();
	
	// 스프라이트 크기 설정
	if (m_orignalBitmap) {
		this->m_width = static_cast<float>(m_orignalBitmap->GetWidth());
		this->m_height = static_cast<float>(m_orignalBitmap->GetHeight());
	}
}

void Sapling::LateInit()
{
}

void Sapling::Update(float deltaTime)
{
	// 필요한 컴포넌트 업데이트가 있다면 여기서 추가
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
	if (GetActive() && CanInteract()) {
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

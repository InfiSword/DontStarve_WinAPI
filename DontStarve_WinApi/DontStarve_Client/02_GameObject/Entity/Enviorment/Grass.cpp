#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "Grass.h"

Grass::Grass(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName), m_state(GrassState::GRASS_IDLE)
{
	m_dropItemID = GOID_ITEM_CUT_NORMAL_GRASS;
	m_dropItemCount = 1;
}

Grass::~Grass() {}

void Grass::Init()
{
	SetActive(true);
	SetInteractive(true);
	
	// 이미지 로드
	LoadBitmap();
	
	// 비트맵에서 크기 가져오기
	if (m_bitmap) {
		this->m_width = static_cast<float>(m_bitmap->GetWidth());
		this->m_height = static_cast<float>(m_bitmap->GetHeight());
	}
}

void Grass::LateInit()
{
}

void Grass::Update(float deltaTime)
{
	// 필요한 업데이트 로직이 있다면 여기에 추가
}

void Grass::LateUpdate()
{
}

void Grass::Release()
{
	// 필요한 정리 작업
}

void Grass::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

void Grass::SetDropItem(GameObjectID itemID, int count)
{
	m_dropItemID = itemID;
	m_dropItemCount = count;
}

void Grass::Damaged(int damage)
{
}

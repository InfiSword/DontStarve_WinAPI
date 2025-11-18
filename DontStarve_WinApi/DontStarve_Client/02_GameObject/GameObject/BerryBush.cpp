#include "../../99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../02_GameObject/Player/Player.h"
#include "BerryBush.h"

BerryBush::BerryBush(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity<GrassState>(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName)
{
	m_dropItemID = GOID_ITEM_BERRY;
	m_dropItemCount = 1;
}

BerryBush::~BerryBush() {}

void BerryBush::Init()
{
	SetActive(true);
	SetInteractive(true);
	m_direction = DIR_DOWN;
	m_state = GRASS_IDLE;
	
	m_dropItemID = GOID_ITEM_BERRY;
	m_dropItemCount = 1;
	
	// 이미지 로드
	LoadBitmap();
	
	// 비트맵에서 크기 가져오기
	if (m_bitmap) {
		this->m_width = static_cast<float>(m_bitmap->GetWidth());
		this->m_height = static_cast<float>(m_bitmap->GetHeight());
	}
}

void BerryBush::LateInit()
{
}

void BerryBush::Update(float deltaTime)
{
	// 필요한 업데이트 로직이 있다면 여기에 추가
}

void BerryBush::LateUpdate()
{
}

void BerryBush::Release()
{
	// 필요한 정리 작업
}

void BerryBush::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

void BerryBush::OnPlayerInteraction(Player* player)
{
	if (GetActive() && CanInteract()) {
		// 플레이어가 이 BerryBush와 상호작용하도록 요청
		player->OnInteraction(this);
	}
}

void BerryBush::SetDropItem(GameObjectID itemID, int count)
{
	m_dropItemID = itemID;
	m_dropItemCount = count;
}

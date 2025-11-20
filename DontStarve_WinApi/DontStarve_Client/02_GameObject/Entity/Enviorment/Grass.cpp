#include "../../../99_Default/pch.h"
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
	// 비트맵은 생성자에서 이미 로드됨
	// 비트맵의 크기 설정 (생성자에서 설정했지만, 혹시 모를 경우를 대비해 재설정)
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
	// 필요 시 컴포넌트 업데이트 로직을 여기에 추가
}

void Grass::LateUpdate()
{
}

void Grass::Release()
{
	// 필요한 정리 작업 수행
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

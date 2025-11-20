#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "Tree.h"

Tree::Tree(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName), m_hp(100), m_hitAnimTimer(0.0f), m_state(TreeState::TREE_IDLE)
{
	maxHp = m_hp;
}

Tree::~Tree() {}

void Tree::Init()
{
	SetActive(true);
	SetInteractive(true);
	
	// 이미지 로드
	LoadBitmap();
	
	// 비트맵의 크기 설정
	if (m_bitmap) {
		this->m_width = static_cast<float>(m_bitmap->GetWidth());
		this->m_height = static_cast<float>(m_bitmap->GetHeight());
	}
}

void Tree::LateInit()
{
}

void Tree::Update(float deltaTime)
{
	if (m_state == TreeState::TREE_CHOP) {
		m_hitAnimTimer += deltaTime;
		if (m_hitAnimTimer >= 0.6f) { // 애니메이션 지속시간으로 상태 타이머
			m_state = TreeState::TREE_IDLE;
			m_hitAnimTimer = 0.0f; 
		}
	}
}

void Tree::LateUpdate()
{
}

void Tree::Release()
{
	// 필요한 정리 작업
}

void Tree::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}


void Tree::Damaged(int damage)
{
	m_hp -= damage;
	m_state = TreeState::TREE_CHOP;
	
	if (m_hp <= 0) {
		m_state = TreeState::TREE_FALL;
		OutputDebugStringW(L"Tree: 나무가 쓰러졌습니다!\n");
	}
}

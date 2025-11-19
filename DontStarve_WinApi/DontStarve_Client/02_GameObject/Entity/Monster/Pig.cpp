#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Pig.h"

Pig::Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName)
{
	m_hp = 100;
	maxHp = m_hp;
}

Pig::~Pig() {}

void Pig::Init()
{
	Monster::Init();
	
	OutputDebugStringW((L"Pig: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
	// 초기 크기 설정 (애니메이션 클립에서 첫 번째 프레임으로 크기 설정)
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		const AnimationFrame& frame = animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Pig: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Pig: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Pig: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Pig::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

// Unity Animator 스타일 애니메이션 등록
void Pig::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 리소스 로드
	ResourceManager* pRM = ResourceManager::GetInstance();
	
	Animator* animator = GetComponent<Animator>();
	if (!animator) return;
	
	// PIG 애니메이션 등록
	if (m_id == GOID_MONSTER_PIG)
	{
		// IDLE 애니메이션들
		animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			 pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_down.png"), 
			 120, 150, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_up.png"),
			120, 150, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_side.png"),
			120, 150, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Action", L"pig_pigman_idle_loop_side.png"),
			120, 150, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"down_pigman_atk_down.png"),
			150, 180, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"up_pigman_atk_up.png"),
			150, 180, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"side_pigman_atk_side.png"),
			150, 180, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Attack", L"side_pigman_atk_side.png"),
			150, 180, 6, 6, 0.1f, m_pivotX, m_pivotY, false);
		
		// HIT 애니메이션
		animator->RegisterAnimation(MONSTER_HIT, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Hit", L"Hit_pigman_hit.png"),
			120, 150, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
		
		// DEATH 애니메이션
		animator->RegisterAnimation(MONSTER_DEATH, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_PIG, L"Death", L"Death_pigman_death.png"),
			150, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	
	OutputDebugStringW(L"Pig: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

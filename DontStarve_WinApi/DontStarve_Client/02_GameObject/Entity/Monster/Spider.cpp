#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Spider.h"

Spider::Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName)
{
	m_hp = 80;
	maxHp = m_hp;
}

Spider::~Spider() {}

void Spider::Init()
{
	Monster::Init();
	
	OutputDebugStringW((L"Spider: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
	// 초기 크기 설정 (애니메이션 클립에서 첫 번째 프레임으로 크기 설정)
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		const AnimationFrame& frame = animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Spider: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Spider: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Spider: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Spider::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

// Unity Animator 스타일 애니메이션 등록
void Spider::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 리소스 로드
	auto* pRM = ResourceManager::GetInstance();
	
	// SPIDER 애니메이션 등록
	Animator* animator = GetComponent<Animator>();
	if (!animator) return;
	
	if (m_id == GOID_MONSTER_SPIDER)
	{
		// IDLE 애니메이션들
		animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// WALK 애니메이션들
		animator->RegisterAnimation(MONSTER_WALK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_down.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_up.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_side.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_side.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_down.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_up.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_side.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_side.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		// HIT 애니메이션
		animator->RegisterAnimation(MONSTER_HIT, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_hit.png"),
			80, 80, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
		
		// DEATH 애니메이션
		animator->RegisterAnimation(MONSTER_DEATH, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_death.png"),
			80, 80, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	else if (m_id == GOID_MONSTER_WARRIOR_SPIDER)
	{
		// WARRIOR SPIDER 애니메이션들
		// IDLE 애니메이션들
		animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
			80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// WALK 애니메이션들
		animator->RegisterAnimation(MONSTER_WALK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_down.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_up.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_side.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_side.png"),
			80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_down.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_up.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_side.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_side.png"),
			100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		// HIT 애니메이션
		animator->RegisterAnimation(MONSTER_HIT, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_hit.png"),
			80, 80, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
		
		// DEATH 애니메이션
		animator->RegisterAnimation(MONSTER_DEATH, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_death.png"),
			80, 80, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	
	OutputDebugStringW(L"Spider: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Boss_SpiderQueen.h"

Boss_SpiderQueen::Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName), m_bossPhase(1), m_specialAttackCooldown(0.0f)
{
	// 보스 특성 초기화
	m_hp = 200; // 일반 스파이더보다 높은 체력
	maxHp = m_hp;
}

Boss_SpiderQueen::~Boss_SpiderQueen() {}

void Boss_SpiderQueen::Init()
{
	Monster::Init(); // 부모 클래스 초기화
	
	// 보스 특성 초기화
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;
	
	OutputDebugStringW((L"Boss_SpiderQueen: 보스 초기화 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
}

void Boss_SpiderQueen::OnInteraction(GameObject* obj)
{
    // 보스 상호작용
}

void Boss_SpiderQueen::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 리소스 로드
	auto* pRM = ResourceManager::GetInstance();
	
	// QUEEN SPIDER 애니메이션 등록
	Animator* animator = GetComponent<Animator>();
	if (!animator) return;
	
	if (m_id == GOID_MONSTER_QUEEN_SPIDER)
	{
		// IDLE 애니메이션들
		animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png"),
			120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png"),
			120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png"),
			120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_Image.png"),
			120, 120, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
		
		// WALK 애니메이션들
		animator->RegisterAnimation(MONSTER_WALK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png"),
			120, 120, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png"),
			120, 120, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png"),
			120, 120, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_WALK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Walk_spider_queen_walk_loop_side.png"),
			120, 120, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png"),
			140, 140, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png"),
			140, 140, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png"),
			140, 140, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_atk_side.png"),
			140, 140, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		// HIT 애니메이션
		animator->RegisterAnimation(MONSTER_HIT, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_hit_side.png"),
			120, 120, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
		
		// DEATH 애니메이션
		animator->RegisterAnimation(MONSTER_DEATH, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_QUEEN_SPIDER, L"", L"Queen_spider_queen_death.png"),
			120, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	
	OutputDebugStringW(L"Boss_SpiderQueen: 보스 애니메이션 등록 완료\n");
}

void Boss_SpiderQueen::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	UpdateAnimatorState();
	
	// 보스 페이즈 체크
	if (m_hp <= maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW(L"Boss_SpiderQueen: 보스 페이즈가 2 단계로 전환!\n");
	}
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		UpdateAnimatorState();
		
		// 보스 처치 시 특별한 보상
		OutputDebugStringW(L"Boss_SpiderQueen: 보스가 처치되었습니다!\n");
	}
}

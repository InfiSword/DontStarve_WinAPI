#include "../../99_Default/pch.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../02_GameObject/Player/Player.h"
#include "../../03_Animation/SpriteSheet.h"
#include "../../../Header/Struct.h"
#include "Boss_Hound.h"

Boss_Hound::Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Hound(id, x, y, pivotX, pivotY, resourcePath, imageName), m_bossPhase(1), m_specialAttackCooldown(0.0f)
{
	// 보스 전용 초기화
	m_hp = 150; // 보스는 더 많은 체력
	maxHp = m_hp;
	
	// Hound 타입 설정
	if (id == GOID_MONSTER_REDHOUNDDOG) {
		m_houndType = L"Red";
	} else if (id == GOID_MONSTER_ICEHOUNDDOG) {
		m_houndType = L"Ice";
	}
}

Boss_Hound::~Boss_Hound() {}

void Boss_Hound::Init()
{
	Hound::Init(); // 부모 클래스 초기화
	
	// 보스 전용 초기화
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;
	
	OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스 초기화 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
}

void Boss_Hound::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 리소스 로드
	auto* pRM = ResourceManager::GetInstance();
	
	// BOSS HOUND 애니메이션 등록
	if (m_id == GOID_MONSTER_REDHOUNDDOG)
	{
		// RED HOUND 보스 애니메이션들
		// IDLE 애니메이션들
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_idle_down.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_idle_up.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_idle_side.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_idle_side.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_atk_down.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_atk_up.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_atk_side.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_REDHOUNDDOG, L"Red_Hound", L"RedHound_redhound_atk_side.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	else if (m_id == GOID_MONSTER_ICEHOUNDDOG)
	{
		// ICE HOUND 보스 애니메이션들
		// IDLE 애니메이션들
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_idle_down.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_idle_up.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_idle_side.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		m_animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_idle_side.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_atk_down.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_atk_up.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_atk_side.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		m_animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_ICEHOUNDDOG, L"Ice_Hound", L"IceHound_icehound_atk_side.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	
	OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스 애니메이션 등록 완료\n").c_str());
}

void Boss_Hound::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	UpdateAnimatorState();
	
	// 보스 페이즈 체크
	if (m_hp <= maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스 페이즈 2 시작!\n").c_str());
	}
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		UpdateAnimatorState();
		
		// 보스 처치 시 특별한 보상
		OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스가 처치되었습니다!\n").c_str());
	}
} 
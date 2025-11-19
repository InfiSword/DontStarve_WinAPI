#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Hound.h"

Hound::Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName)
{
	m_hp = 90;
	maxHp = m_hp;
}

Hound::~Hound() {}

void Hound::Init()
{
	Monster::Init();
	
	OutputDebugStringW((L"Hound: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	
	// 초기 크기 설정 (애니메이션 클립에서 첫 번째 프레임으로 크기 설정)
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		const AnimationFrame& frame = animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Hound: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Hound: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Hound: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Hound::OnInteraction(GameObject* obj)
{
	// 기본 상호작용
}

// Unity Animator 스타일 애니메이션 등록
void Hound::RegisterAllAnimations()
{
	// ResourceManager를 사용하여 리소스 로드
	auto* pRM = ResourceManager::GetInstance();
	
	Animator* animator = GetComponent<Animator>();
	if (!animator) return;
	
	// HOUND 애니메이션 등록
	if (m_id == GOID_MONSTER_HOUNDDOG)
	{
		// IDLE 애니메이션들
		animator->RegisterAnimation(MONSTER_IDLE, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_down.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_up.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_side.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		animator->RegisterAnimation(MONSTER_IDLE, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_side.png"),
			120, 100, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
		
		// ATTACK 애니메이션들
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_DOWN,
			pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_down.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_UP,
			pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_up.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_LEFT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_side.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		
		animator->RegisterAnimation(MONSTER_ATTACK, DIR_RIGHT,
			pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_side.png"),
			140, 120, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
	}
	
	OutputDebugStringW(L"Hound: Unity Animator 스타일로 모든 애니메이션 등록 완료\n");
}

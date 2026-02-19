#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "Boss_Hound.h"

Boss_Hound::Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, baseDir, imageName), m_bossPhase(1), m_specialAttackCooldown(0.0f)
{
	// 보스 특성 초기화
	m_hp = 150; // 일반 하운드보다 높은 체력
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
	Monster::Init(); // 부모 클래스 초기화
	
	// Transform 컴포넌트 확인
	if (!this->transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Boss_Hound: Transform component not found!\n");
			return;
		}
	}
	
	// 보스 특성 초기화
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;
	
	OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스 초기화 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	// Animator 생성 및 애니메이션 등록 (AnimationDefinition 클래스 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();

		if (m_id == GOID_MONSTER_REDHOUNDDOG) {
			const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_REDHOUNDDOG);
			if (objData) {
				std::wstring base = objData->baseDir + L"\\Red_Hound\\";
				
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN, base + L"RedHound_redhound_idle_down.png",
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP, base + L"RedHound_redhound_idle_up.png",
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
				std::wstring idleSidePath = base + L"RedHound_redhound_idle_side.png";
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT, idleSidePath,
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT, idleSidePath,
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN, base + L"RedHound_redhound_atk_down.png",
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP, base + L"RedHound_redhound_atk_up.png",
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
				std::wstring atkSidePath = base + L"RedHound_redhound_atk_side.png";
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT, atkSidePath,
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT, atkSidePath,
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			}
		}
		else if (m_id == GOID_MONSTER_ICEHOUNDDOG) {
			const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_ICEHOUNDDOG);
			if (objData) {
				std::wstring base = objData->baseDir + L"\\Ice_Hound\\";
				
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN, base + L"IceHound_icehound_idle_down.png",
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP, base + L"IceHound_icehound_idle_up.png",
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
				std::wstring idleSidePath = base + L"IceHound_icehound_idle_side.png";
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT, idleSidePath,
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT, idleSidePath,
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN, base + L"IceHound_icehound_atk_down.png",
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP, base + L"IceHound_icehound_atk_up.png",
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
				std::wstring atkSidePath = base + L"IceHound_icehound_atk_side.png";
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT, atkSidePath,
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT, atkSidePath,
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			}
		}

		m_animator->SetState((int)m_state, transform->GetDirection());
	}
}

bool Boss_Hound::OnInteraction(GameObject* obj)
{
    // 보스 상호작용 처리 (추후 구현 예정)
    return Monster::OnInteraction(obj);
}

void Boss_Hound::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	
	// 보스 페이즈 체크
	if (m_hp <= maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스 페이즈가 2단계로 전환!\n").c_str());
	}
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		
		// 보스 처치 시 특수 보상
		OutputDebugStringW((L"Boss_Hound: " + m_houndType + L" 보스가 처치되었습니다!\n").c_str());
	}
}

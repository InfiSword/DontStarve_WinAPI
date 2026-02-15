#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "Boss_Hound.h"

void Boss_Hound::RegisterResources(ResourceManager* rm)
{
	if (!rm) return;
	GameObjectData d;
	d.type = GOBJ_MONSTER;
	d.pivotX = 0.5f;
	d.pivotY = 1.0f;
	d.id = GOID_MONSTER_REDHOUNDDOG;
	d.objectAssetBaseDirectory = L"Resource/Objects/Monster/Hound/Red_Hound";
	d.assetImageName = L"RedHound_hound_Image.png";
	rm->RegisterObjectResource(GOID_MONSTER_REDHOUNDDOG, d);
	d.id = GOID_MONSTER_ICEHOUNDDOG;
	d.objectAssetBaseDirectory = L"Resource/Objects/Monster/Hound/Ice_Hound";
	d.assetImageName = L"IceHound_hound_Image.png";
	rm->RegisterObjectResource(GOID_MONSTER_ICEHOUNDDOG, d);
}

Boss_Hound::Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, imageName), m_bossPhase(1), m_specialAttackCooldown(0.0f)
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
			const GameObjectData* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_REDHOUNDDOG);
			if (objData) {
				const std::wstring& base = objData->objectAssetBaseDirectory;
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN,
					pRM->BuildResourcePath(base, L"Red_Hound", L"RedHound_redhound_idle_down.png"),
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP,
					pRM->BuildResourcePath(base, L"Red_Hound", L"RedHound_redhound_idle_up.png"),
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT,
					pRM->BuildResourcePath(base, L"Red_Hound", L"RedHound_redhound_idle_side.png"),
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT,
					pRM->BuildResourcePath(base, L"Red_Hound", L"RedHound_redhound_idle_side.png"),
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);

				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN,
					pRM->BuildResourcePath(base, L"Red_Hound", L"RedHound_redhound_atk_down.png"),
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP,
					pRM->BuildResourcePath(base, L"Red_Hound", L"RedHound_redhound_atk_up.png"),
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT,
					pRM->BuildResourcePath(base, L"Red_Hound", L"RedHound_redhound_atk_side.png"),
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT,
					pRM->BuildResourcePath(base, L"Red_Hound", L"RedHound_redhound_atk_side.png"),
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			}
		}
		else if (m_id == GOID_MONSTER_ICEHOUNDDOG) {
			const GameObjectData* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_ICEHOUNDDOG);
			if (objData) {
				const std::wstring& base = objData->objectAssetBaseDirectory;
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN,
					pRM->BuildResourcePath(base, L"Ice_Hound", L"IceHound_icehound_idle_down.png"),
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP,
					pRM->BuildResourcePath(base, L"Ice_Hound", L"IceHound_icehound_idle_up.png"),
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT,
					pRM->BuildResourcePath(base, L"Ice_Hound", L"IceHound_icehound_idle_side.png"),
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT,
					pRM->BuildResourcePath(base, L"Ice_Hound", L"IceHound_icehound_idle_side.png"),
					120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);

				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN,
					pRM->BuildResourcePath(base, L"Ice_Hound", L"IceHound_icehound_atk_down.png"),
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP,
					pRM->BuildResourcePath(base, L"Ice_Hound", L"IceHound_icehound_atk_up.png"),
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT,
					pRM->BuildResourcePath(base, L"Ice_Hound", L"IceHound_icehound_atk_side.png"),
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT,
					pRM->BuildResourcePath(base, L"Ice_Hound", L"IceHound_icehound_atk_side.png"),
					140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			}
		}

		m_animator->SetState((int)m_state, transform->GetDirection());
	}
}

void Boss_Hound::OnInteraction(GameObject* obj)
{
    // 보스 상호작용 처리 (추후 구현 예정)
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

#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "Pig.h"

Pig::Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, baseDir, imageName)
{
	m_hp = 100;
	maxHp = m_hp;
}

Pig::~Pig() {}

void Pig::Init()
{
	Monster::Init();
	
	// Transform 컴포넌트 확인
	if (!this->transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Pig: Transform component not found!\n");
			return;
		}
	}
	
	OutputDebugStringW((L"Pig: Init 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	// Animator 생성 및 애니메이션 등록 (AnimationDefinition 클래스 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_PIG);
		if (m_id == GOID_MONSTER_PIG && objData) {
			std::wstring base = objData->baseDir + L"\\";
			
			// IDLE
			std::wstring baseAction = base + L"Action\\";
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN, baseAction + L"pig_pigman_idle_loop_down.png",
				120, 150, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP, baseAction + L"pig_pigman_idle_loop_up.png",
				120, 150, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			std::wstring idleSidePath = baseAction + L"pig_pigman_idle_loop_side.png";
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT, idleSidePath,
				120, 150, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT, idleSidePath,
				120, 150, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

			// ATTACK
			std::wstring baseAttack = base + L"Attack\\";
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN, baseAttack + L"down_pigman_atk_down.png",
				150, 180, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP, baseAttack + L"up_pigman_atk_up.png",
				150, 180, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			std::wstring atkSidePath = baseAttack + L"side_pigman_atk_side.png";
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT, atkSidePath,
				150, 180, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT, atkSidePath,
				150, 180, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);

			// HIT / DEATH
			m_animator->RegisterAnimation((int)MONSTER_HIT, DIR_DOWN, base + L"Hit\\Hit_pigman_hit.png",
				120, 150, 3, 3, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_DEATH, DIR_DOWN, base + L"Death\\Death_pigman_death.png",
				150, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
		}

		m_animator->SetState((int)m_state, this->transform->GetDirection());
	}
	
	// Animator 초기화 확인
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Pig: Animator 초기화 성공 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Pig: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Pig: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

bool Pig::OnInteraction(GameObject* obj)
{
	return Monster::OnInteraction(obj);
}

void Pig::Damaged(int damage)
{
}

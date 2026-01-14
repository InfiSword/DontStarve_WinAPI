#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../Player/Player.h"
#include "../../Component/Transform/Transform.h"
#include "Hound.h"

Hound::Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, imageName)
{
	m_hp = 90;
	maxHp = m_hp;
}

Hound::~Hound() {}

void Hound::Init()
{
	Monster::Init();
	
	// Transform 컴포넌트 확인
	if (!this->transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Hound: Transform component not found!\n");
			return;
		}
	}
	
	OutputDebugStringW((L"Hound: Init 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	// Animator 생성 및 애니메이션 등록 (AnimationDefinition 클래스 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();

		if (m_id == GOID_MONSTER_HOUNDDOG) {
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_down.png"),
				120, 100, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), true);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP,
				pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_up.png"),
				120, 100, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), true);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_side.png"),
				120, 100, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), true);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_idle_side.png"),
				120, 100, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), true);

			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_down.png"),
				140, 120, 8, 8, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP,
				pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_up.png"),
				140, 120, 8, 8, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_side.png"),
				140, 120, 8, 8, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_HOUNDDOG, L"Normal_Hound", L"Hound_hound_atk_side.png"),
				140, 120, 8, 8, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
		}

		m_animator->SetState((int)m_state, this->transform->GetDirection());
	}
	
	// Animator 초기화 확인
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Hound: Animator 초기화 성공 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Hound: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Hound: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Hound::OnInteraction(GameObject* obj)
{
	// 기본 상호작용 사용
}

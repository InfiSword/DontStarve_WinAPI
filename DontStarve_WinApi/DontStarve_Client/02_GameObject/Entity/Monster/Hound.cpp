#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../Player/Player.h"
#include "../../Component/Transform/Transform.h"
#include "Hound.h"

Hound::Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, baseDir, imageName)
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
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_HOUNDDOG);

		if (m_id == GOID_MONSTER_HOUNDDOG && objData) {
			std::wstring base = objData->baseDir + L"\\Normal_Hound\\";
			
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN, base + L"Hound_hound_idle_down.png",
				120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP, base + L"Hound_hound_idle_up.png",
				120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			std::wstring idleSidePath = base + L"Hound_hound_idle_side.png";
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT, idleSidePath,
				120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT, idleSidePath,
				120, 100, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN, base + L"Hound_hound_atk_down.png",
				140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP, base + L"Hound_hound_atk_up.png",
				140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			std::wstring atkSidePath = base + L"Hound_hound_atk_side.png";
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT, atkSidePath,
				140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT, atkSidePath,
				140, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
		}

		m_animator->SetState((int)m_state, this->transform->GetDirection());
	}
}

bool Hound::OnInteraction(GameObject* obj)
{
	return Monster::OnInteraction(obj);
}

#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "Pig.h"

void Pig::RegisterResources(ResourceManager* rm)
{
	if (!rm) return;
	GameObjectData d;
	d.type = GOBJ_MONSTER;
	d.pivotX = 0.5f;
	d.pivotY = 1.0f;
	d.id = GOID_MONSTER_PIG;
	d.objectAssetBaseDirectory = L"Resource/Objects/Monster/Pig";
	d.assetImageName = L"pig_Image.png";
	rm->RegisterObjectResource(GOID_MONSTER_PIG, d);
}

Pig::Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, imageName)
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
		const GameObjectData* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_PIG);
		if (m_id == GOID_MONSTER_PIG && objData) {
			const std::wstring& base = objData->objectAssetBaseDirectory;
			// IDLE
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN,
				pRM->BuildResourcePath(base, L"Action", L"pig_pigman_idle_loop_down.png"),
				120, 150, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), true);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP,
				pRM->BuildResourcePath(base, L"Action", L"pig_pigman_idle_loop_up.png"),
				120, 150, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), true);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT,
				pRM->BuildResourcePath(base, L"Action", L"pig_pigman_idle_loop_side.png"),
				120, 150, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), true);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT,
				pRM->BuildResourcePath(base, L"Action", L"pig_pigman_idle_loop_side.png"),
				120, 150, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), true);

			// ATTACK
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN,
				pRM->BuildResourcePath(base, L"Attack", L"down_pigman_atk_down.png"),
				150, 180, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP,
				pRM->BuildResourcePath(base, L"Attack", L"up_pigman_atk_up.png"),
				150, 180, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT,
				pRM->BuildResourcePath(base, L"Attack", L"side_pigman_atk_side.png"),
				150, 180, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT,
				pRM->BuildResourcePath(base, L"Attack", L"side_pigman_atk_side.png"),
				150, 180, 6, 6, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);

			// HIT / DEATH
			m_animator->RegisterAnimation((int)MONSTER_HIT, DIR_DOWN,
				pRM->BuildResourcePath(base, L"Hit", L"Hit_pigman_hit.png"),
				120, 150, 3, 3, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
			m_animator->RegisterAnimation((int)MONSTER_DEATH, DIR_DOWN,
				pRM->BuildResourcePath(base, L"Death", L"Death_pigman_death.png"),
				150, 100, 8, 8, 0.1f, this->transform->GetPivotX(), this->transform->GetPivotY(), false);
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

void Pig::OnInteraction(GameObject* obj)
{
	// 기본 상호작용 사용
}

void Pig::Damaged(int damage)
{
}

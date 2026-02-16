#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "Spider.h"

Spider::Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, imageName)
{
	m_hp = 80;
	maxHp = m_hp;
}

Spider::~Spider() {}

void Spider::Init()
{
	Monster::Init();
	
	// Transform 컴포넌트 확인
	if (!this->transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Spider: Transform component not found!\n");
			return;
		}
	}
	
	OutputDebugStringW((L"Spider: Init 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	// Animator 생성 및 애니메이션 등록 (AnimationDefinition 클래스 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();

		if (m_id == GOID_MONSTER_SPIDER) {
			const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_SPIDER);
			if (!objData) return;
			const std::wstring& base = objData->baseDir;
			// IDLE
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)MONSTER_IDLE, (Direction)dir,
					pRM->BuildResourcePath(base, L"", L"Spider_spider_idle_01.png"),
					80, 80, 1, 1, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
			}

			// WALK
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_DOWN,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_walk_loop_down.png"),
				80, 80, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_UP,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_walk_loop_up.png"),
				80, 80, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_LEFT,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_walk_loop_side.png"),
				80, 80, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_RIGHT,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_walk_loop_side.png"),
				80, 80, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);

			// ATTACK
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_atk_down.png"),
				100, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_atk_up.png"),
				100, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_atk_side.png"),
				100, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_atk_side.png"),
				100, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);

			// HIT / DEATH
			m_animator->RegisterAnimation((int)MONSTER_HIT, DIR_DOWN,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_hit.png"),
				80, 80, 3, 3, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_DEATH, DIR_DOWN,
				pRM->BuildResourcePath(base, L"", L"Spider_spider_death.png"),
				80, 80, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
		}
		else if (m_id == GOID_MONSTER_WARRIOR_SPIDER) {
			const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_WARRIOR_SPIDER);
			if (!objData) return;
			const std::wstring& base = objData->baseDir;
			// IDLE
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)MONSTER_IDLE, (Direction)dir,
					pRM->BuildResourcePath(base, L"", L"Warrior_spider_idle_01.png"),
					80, 80, 1, 1, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
			}

			// WALK
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_DOWN,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_walk_loop_down.png"),
				80, 80, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_UP,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_walk_loop_up.png"),
				80, 80, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_LEFT,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_walk_loop_side.png"),
				80, 80, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_RIGHT,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_walk_loop_side.png"),
				80, 80, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, {}, false, 0.03f);

			// ATTACK
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_atk_down.png"),
				100, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_atk_up.png"),
				100, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_atk_side.png"),
				100, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_atk_side.png"),
				100, 100, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);

			// HIT / DEATH
			m_animator->RegisterAnimation((int)MONSTER_HIT, DIR_DOWN,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_hit.png"),
				80, 80, 3, 3, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_DEATH, DIR_DOWN,
				pRM->BuildResourcePath(base, L"", L"Warrior_spider_death.png"),
				80, 80, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, {}, false, 0.03f);
		}

		// 초기 상태 적용
		m_animator->SetState((int)m_state, this->transform->GetDirection());
	}
	
	// Animator 초기화 확인
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Spider: Animator 초기화 성공 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Spider: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Spider: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Spider::OnInteraction(GameObject* obj)
{
	// 기본 상호작용 사용
}

void Spider::Damaged(int damage)
{
}

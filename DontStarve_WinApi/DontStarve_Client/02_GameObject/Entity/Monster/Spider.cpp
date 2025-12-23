#include "../../../99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "Spider.h"

Spider::Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, resourcePath, imageName)
{
	m_hp = 80;
	maxHp = m_hp;
}

Spider::~Spider() {}

void Spider::Init()
{
	Monster::Init();
	
	OutputDebugStringW((L"Spider: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	// Animator 생성 및 애니메이션 등록 (AnimationDefinition 패턴 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();

		if (m_id == GOID_MONSTER_SPIDER) {
			// IDLE
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)MONSTER_IDLE, (Direction)dir,
					pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_idle_01.png"),
					80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
			}

			// WALK
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_down.png"),
				80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_UP,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_up.png"),
				80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_LEFT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_side.png"),
				80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_RIGHT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_walk_loop_side.png"),
				80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);

			// ATTACK
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_down.png"),
				100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_up.png"),
				100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_side.png"),
				100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_atk_side.png"),
				100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);

			// HIT / DEATH
			m_animator->RegisterAnimation((int)MONSTER_HIT, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_hit.png"),
				80, 80, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
			m_animator->RegisterAnimation((int)MONSTER_DEATH, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_SPIDER, L"", L"Spider_spider_death.png"),
				80, 80, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		}
		else if (m_id == GOID_MONSTER_WARRIOR_SPIDER) {
			// IDLE
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)MONSTER_IDLE, (Direction)dir,
					pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_idle_01.png"),
					80, 80, 1, 1, 0.1f, m_pivotX, m_pivotY, true);
			}

			// WALK
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_down.png"),
				80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_UP,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_up.png"),
				80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_LEFT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_side.png"),
				80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_RIGHT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_walk_loop_side.png"),
				80, 80, 6, 6, 0.1f, m_pivotX, m_pivotY, true);

			// ATTACK
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_down.png"),
				100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_up.png"),
				100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_side.png"),
				100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_atk_side.png"),
				100, 100, 8, 8, 0.1f, m_pivotX, m_pivotY, false);

			// HIT / DEATH
			m_animator->RegisterAnimation((int)MONSTER_HIT, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_hit.png"),
				80, 80, 3, 3, 0.1f, m_pivotX, m_pivotY, false);
			m_animator->RegisterAnimation((int)MONSTER_DEATH, DIR_DOWN,
				pRM->BuildObjectResourcePath(GOID_MONSTER_WARRIOR_SPIDER, L"", L"Warrior_spider_death.png"),
				80, 80, 8, 8, 0.1f, m_pivotX, m_pivotY, false);
		}

		// 초기 상태 적용
		m_animator->SetState((int)m_state, m_direction);
	}
	
	// 초기 크기 설정 (애니메이션 클립의 첫 프레임 기준)
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		const AnimationFrame& frame = animator->GetCurrentFrame();
		this->m_width = frame.width;
		this->m_height = frame.height;
		
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Spider: Animator 초기화 완료 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
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

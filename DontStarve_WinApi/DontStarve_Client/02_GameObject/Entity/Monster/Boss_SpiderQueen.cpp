#include "99_Default/pch.h"
#include "Boss_SpiderQueen.h"
#include "../Player/Player.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../../01_Manager/SceneManager/SceneManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"

Boss_SpiderQueen::Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Entity(id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName), m_bossPhase(1), m_specialAttackCooldown(0.0f)
{
	m_hp = 1000;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
}

Boss_SpiderQueen::~Boss_SpiderQueen() {}

void Boss_SpiderQueen::Init()
{
	Entity::Init();
	
	m_state = (int)SpiderQueenState::IDLE;
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;
	
	if (!transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Boss_SpiderQueen: Transform component not found!\n");
			return;
		}
	}
	
	OutputDebugStringW((L"Boss_SpiderQueen: 보스 초기화 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_QUEEN_SPIDER);
		if (objData) {
			std::wstring base = objData->baseDir + L"\\";
			float px = transform->GetPivotX();
			float py = transform->GetPivotY();
			
			std::wstring idlePath = base + L"Queen_spider_queen_idle_side.png";
			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderQueenState::IDLE, (Direction)dir, idlePath,
					0, 0, 10, 61, px, py, true, 0.03f);
			}

			std::wstring walkPath = base + L"Walk_spider_queen_walk_loop_side.png";
			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderQueenState::WALK, (Direction)dir, walkPath,
					0, 0, 10, 61, px, py, true, 0.03f);
			}

			std::wstring attackPath = base + L"Queen_spider_queen_atk_side.png";
			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderQueenState::ATTACK, (Direction)dir, attackPath,
					0, 0, 10, 61, px, py, false, 0.03f);
			}

			m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_DOWN, base + L"Queen_spider_queen_hit_side.png",
				0, 0, 5, 25, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_UP, base + L"Queen_spider_queen_hit_side.png",
				0, 0, 5, 25, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_LEFT, base + L"Queen_spider_queen_hit_side.png",
				0, 0, 5, 25, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_RIGHT, base + L"Queen_spider_queen_hit_side.png",
				0, 0, 5, 25, px, py, false, 0.03f);

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderQueenState::DEATH, (Direction)dir, base + L"Queen_spider_queen_death.png",
					0, 0, 10, 85, px, py, false, 0.03f);
			}

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderQueenState::TAUNT, (Direction)dir, base + L"Queen_spider_queen_taunt.png",
					0, 0, 10, 61, px, py, false, 0.03f);
			}
		}

		m_animator->SetState(m_state, this->transform->GetDirection());
	}
}

bool Boss_SpiderQueen::OnInteraction(GameObject* obj)
{
    return Entity::OnInteraction(obj);
}

void Boss_SpiderQueen::Damaged(int damage)
{
	m_hp -= damage;
	m_state = (int)SpiderQueenState::HIT;
	
	if (m_hp <= m_maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW(L"Boss_SpiderQueen: 보스 페이즈가 2단계로 전환!\n");
	}
	
	if (m_hp <= 0) {
		m_hp = 0;
		m_state = (int)SpiderQueenState::DEATH;
		m_isDead = true;
		SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
		GameProgressManager::GetInstance()->OnMonsterKilled(GetID(), currentScene);
		OutputDebugStringW(L"Boss_SpiderQueen: 보스가 처치되었습니다!\n");
	}
}

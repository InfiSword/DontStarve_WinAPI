#include "99_Default/pch.h"
#include "Boss_SpiderQueen.h"
#include "../Player/Player.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"

Boss_SpiderQueen::Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, baseDir, imageName), m_bossPhase(1), m_specialAttackCooldown(0.0f)
{
	// 보스 특성 초기화
	m_hp = 200; // 일반 스파이더보다 높은 체력
	maxHp = m_hp;
}

Boss_SpiderQueen::~Boss_SpiderQueen() {}

void Boss_SpiderQueen::Init()
{
	Monster::Init(); // 부모 클래스 초기화
	
	// Transform 컴포넌트 확인
	if (!transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Boss_SpiderQueen: Transform component not found!\n");
			return;
		}
	}
	
	// 보스 특성 초기화
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;
	
	OutputDebugStringW((L"Boss_SpiderQueen: 보스 초기화 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	// Animator 생성 및 애니메이션 등록 (AnimationDefinition 클래스 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();

		if (m_id == GOID_MONSTER_QUEEN_SPIDER) {
			const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_QUEEN_SPIDER);
			if (!objData) return;
			std::wstring base = objData->baseDir + L"\\";
			
			// IDLE
			std::wstring idlePath = base + L"Queen_spider_queen_Image.png";
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)MONSTER_IDLE, (Direction)dir, idlePath,
					120, 120, 1, 1, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			}

			// WALK
			std::wstring walkPath = base + L"Walk_spider_queen_walk_loop_side.png";
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)MONSTER_WALK, (Direction)dir, walkPath,
					120, 120, 6, 6, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			}

			// ATTACK
			std::wstring attackPath = base + L"Queen_spider_queen_atk_side.png";
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)MONSTER_ATTACK, (Direction)dir, attackPath,
					140, 140, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			}

			// HIT / DEATH
			m_animator->RegisterAnimation((int)MONSTER_HIT, DIR_DOWN, base + L"Queen_spider_queen_hit_side.png",
				120, 120, 3, 3, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);

			m_animator->RegisterAnimation((int)MONSTER_DEATH, DIR_DOWN, base + L"Queen_spider_queen_death.png",
				120, 120, 8, 8, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
		}

		m_animator->SetState((int)m_state, this->transform->GetDirection());
	}
}

bool Boss_SpiderQueen::OnInteraction(GameObject* obj)
{
    // 보스 상호작용 처리 (추후 구현 예정)
    return Monster::OnInteraction(obj);
}

void Boss_SpiderQueen::Damaged(int damage)
{
	m_hp -= damage;
	m_state = MONSTER_HIT;
	
	// 보스 페이즈 체크
	if (m_hp <= maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW(L"Boss_SpiderQueen: 보스 페이즈가 2단계로 전환!\n");
	}
	
	if (m_hp <= 0) {
		m_state = MONSTER_DEATH;
		
		// 보스 처치 시 특수 보상
		OutputDebugStringW(L"Boss_SpiderQueen: 보스가 처치되었습니다!\n");
	}
}

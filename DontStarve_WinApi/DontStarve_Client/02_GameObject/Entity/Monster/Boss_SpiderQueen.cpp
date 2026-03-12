#include "99_Default/pch.h"
#include "../../Component/Transform/Transform.h"
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
#include "../../Component/Collider/BoxCollider.h"

Boss_SpiderQueen::Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
                                   const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
	, m_bossPhase(1)
	, m_specialAttackCooldown(0.0f)
	, m_idleTimer(0.0f)
	, m_idleDuration(2.0f)
	, m_attackCollider(nullptr)
{
	m_hp = 1000;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;

	m_walkSpeed = 50.0f;
	
	// 공격 관련 설정
	m_attackRange = 70.0f;
	m_attackCooldown = 1.5f;
	m_attackHitFrame = 28;
	m_damage = 25;
	m_attackBoxWidth = 70;
	m_attackBoxHeight = 50;
}

Boss_SpiderQueen::~Boss_SpiderQueen() {}

void Boss_SpiderQueen::Init()
{
	Monster::Init();

	// 보스는 항상 플레이어를 추격하는 타입
	SetupAggro(AggroType::ALWAYS, 0.0f, 0.0f);

	ChangeState((int)SpiderQueenState::IDLE);
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;

	if (!m_animator)
	{
		m_animator = AddComponent<Animator>();
	}

	ResourceManager* pRM = ResourceManager::GetInstance();
	const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_QUEEN_SPIDER);
	if (objData) {
		std::wstring base = objData->baseDir + L"\\";
		float px = transform->GetPivotX();
		float py = transform->GetPivotY();

		std::wstring idlePath = base + L"Queen_spider_queen_idle_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::IDLE, (Direction)dir, idlePath,
				0, 0, 4, 50, px, py, true, 0.02f);
		}

		std::wstring walkPath = base + L"Walk_spider_queen_walk_loop_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::CHASE, (Direction)dir, walkPath,
				0, 0, 7, 65, px, py, true, 0.02f);
		}

		std::wstring attackPath = base + L"Queen_spider_queen_atk_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::ATTACK, (Direction)dir, attackPath,
				0, 0, 7, 53, px, py, false, 0.02f);
		}

		m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_DOWN, base + L"Queen_spider_queen_hit_side.png",
			0, 0, 7, 29, px, py, false, 0.02f);
		m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_UP, base + L"Queen_spider_queen_hit_side.png",
			0, 0, 7, 29, px, py, false, 0.02f);
		m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_LEFT, base + L"Queen_spider_queen_hit_side.png",
			0, 0, 7, 29, px, py, false, 0.02f);
		m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_RIGHT, base + L"Queen_spider_queen_hit_side.png",
			0, 0, 7, 29, px, py, false, 0.02f);

		for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::ATTACK, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
				clip->AddEventFrame(65, L"attack_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"attack_hit") this->OnAttackHit();
					else if (eventName == L"attack_end") this->OnAttackEnd();
					});
			}
		}

		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::DEATH, (Direction)dir, base + L"Queen_spider_queen_death.png",
				0, 0, 7, 45, px, py, false, 0.03f);
		}

		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::TAUNT, (Direction)dir, base + L"Queen_spider_queen_taunt.png",
				0, 0, 7, 50, px, py, false, 0.03f);
		}
	}

	m_animator->SetState(m_state, this->transform->GetDirection());
}

void Boss_SpiderQueen::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator)
		return;

	// 1. 공통 애니메이션 상태 처리 (HIT, DEATH, ATTACK)
	if (HandleCommonAnimationState((int)SpiderQueenState::HIT, (int)SpiderQueenState::DEATH, (int)SpiderQueenState::ATTACK))
		return;

	// 2. 메인 AI 로직 (AlwaysChase 타입 헬퍼 사용)
	// SpiderQueen은 CHASE 상태를 이동 상태로 사용합니다.
	UpdateAI_AlwaysChase(deltaTime, 
		(int)SpiderQueenState::CHASE, (int)SpiderQueenState::ATTACK, (int)SpiderQueenState::IDLE);
}

void Boss_SpiderQueen::UpdateMovement(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;
	if (m_state != (int)SpiderQueenState::CHASE) return;

	// 플레이어와 너무 가까우면 이동을 멈춰 흔들림(jitter) 방지
	if (m_distToPlayerSq < (m_attackRange * m_attackRange * 0.9f))
	{
		m_animator->SetState((int)SpiderQueenState::IDLE, transform->GetDirection());
		return;
	}

	Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
	transform->SetDirection(newDir);
	m_animator->SetState((int)SpiderQueenState::CHASE, transform->GetDirection());

	float moveDist = m_walkSpeed * deltaTime;
	transform->SetPosition(transform->GetX() + m_dirToPlayer.X * moveDist, transform->GetY() + m_dirToPlayer.Y * moveDist);
}

void Boss_SpiderQueen::OnAttackHit() {
	if (m_state != (int)SpiderQueenState::ATTACK) return;

	// Monster 기본 클래스의 공격 처리 사용
	ProcessAttackHit(m_damage);
}

void Boss_SpiderQueen::OnAttackEnd() {
	if (m_state != (int)SpiderQueenState::ATTACK) return;
	ChangeState((int)SpiderQueenState::CHASE);
}	

bool Boss_SpiderQueen::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Boss_SpiderQueen::Damaged(int damage)
{
	Entity::Damaged(damage);
	ChangeState((int)SpiderQueenState::HIT);

	if (m_hp <= m_maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW(L"Boss_SpiderQueen: 보스 페이즈가 2단계로 전환!\n");
	}

	if (m_hp <= 0) {
		m_hp = 0;
		ChangeState((int)SpiderQueenState::DEATH);
		m_isDead = true;
		SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
		GameProgressManager::GetInstance()->OnMonsterKilled(GetID(), currentScene);
		OutputDebugStringW(L"Boss_SpiderQueen: 보스가 처치되었습니다\n");
	}

	if (!IsDead() && IsEnabled()) {
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
		m_attackCooldownTimer = 0.0f;
	}
}


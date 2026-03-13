#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"
#include "Boss_Hound.h"

Boss_Hound::Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
                       const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
{
	m_hp = 300;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;

	m_walkSpeed = 100.0f;
	m_runSpeed = 250.0f;
	
	// 공격 관련 설정
	m_attackRange = 100.0f;
	m_attackCooldown = 2.0f;
	m_attackHitFrame = 4;
	m_damage = 30;
	m_attackBoxWidth = 100;
	m_attackBoxHeight = 60;

	// Initialize inherited members from Monster class
	m_wanderRadius = 300.0f;
	m_aggroRadius = 400.0f;
	m_deaggroRadius = 600.0f;
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f;
}

Boss_Hound::~Boss_Hound() {}

void Boss_Hound::Init()
{
	Monster::Init();
	
	// 보스는 항상 플레이어를 추격하는 타입
	SetupAggro(AggroType::ALWAYS, 0.0f, 0.0f);

	// 공격 박스 설정 (방향별 오프셋 자동 계산)
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)BossHoundState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	
	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}

	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}

	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(m_id);

		if (objData) {
			std::wstring base = objData->baseDir + L"\\";
			std::wstring prefix = (m_id == GOID_MONSTER_REDHOUNDDOG) ? L"RedHound_" : L"IceHound_";
			std::wstring houndPrefix = prefix + L"hound_";

			float px = transform->GetPivotX();
			float py = transform->GetPivotY();

			// IDLE
			m_animator->RegisterAnimation((int)BossHoundState::IDLE, DIR_DOWN, base + houndPrefix + L"idle_down.png",
				0, 0, 7, 20, px, py, true, 0.02f);
			m_animator->RegisterAnimation((int)BossHoundState::IDLE, DIR_UP, base + houndPrefix + L"idle_up.png",
				0, 0, 7, 20, px, py, true, 0.02f);
			std::wstring idleSidePath = base + houndPrefix + L"idle_side.png";
			m_animator->RegisterAnimation((int)BossHoundState::IDLE, DIR_LEFT, idleSidePath,
				0, 0, 7, 20, px, py, true, 0.02f, false);
			m_animator->RegisterAnimation((int)BossHoundState::IDLE, DIR_RIGHT, idleSidePath,
				0, 0, 7, 20, px, py, true, 0.02f);

			// RUN / CHASE
			for (int state = (int)BossHoundState::RUN; state <= (int)BossHoundState::CHASE; ++state) {
				if (state != (int)BossHoundState::RUN && state != (int)BossHoundState::CHASE) continue;
				m_animator->RegisterAnimation(state, DIR_DOWN, base + houndPrefix + L"run_loop_down.png",
					0, 0, 7, 16, px, py, true, 0.02f);
				m_animator->RegisterAnimation(state, DIR_UP, base + houndPrefix + L"run_loop_up.png",
					0, 0, 7, 16, px, py, true, 0.02f);
				std::wstring walkSidePath = base + houndPrefix + L"run_loop_side.png";
				m_animator->RegisterAnimation(state, DIR_LEFT, walkSidePath,
					0, 0, 7, 16, px, py, true, 0.02f, false);
				m_animator->RegisterAnimation(state, DIR_RIGHT, walkSidePath,
					0, 0, 7, 16, px, py, true, 0.02f);
			}

			// ATTACK_PRE
			m_animator->RegisterAnimation((int)BossHoundState::ATTACK_PRE, DIR_DOWN, base + houndPrefix + L"atk_pre_down.png",
				0, 0, 7, 29, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)BossHoundState::ATTACK_PRE, DIR_UP, base + houndPrefix + L"atk_pre_up.png",
				0, 0, 7, 29, px, py, false, 0.03f);
			std::wstring atkPreSidePath = base + houndPrefix + L"atk_pre_side.png";
			m_animator->RegisterAnimation((int)BossHoundState::ATTACK_PRE, DIR_LEFT, atkPreSidePath,
				0, 0, 7, 29, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)BossHoundState::ATTACK_PRE, DIR_RIGHT, atkPreSidePath,
				0, 0, 7, 29, px, py, false, 0.03f);

			// ATTACK
			m_animator->RegisterAnimation((int)BossHoundState::ATTACK, DIR_DOWN, base + houndPrefix + L"atk_down.png",
				0, 0, 7, 18, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)BossHoundState::ATTACK, DIR_UP, base + houndPrefix + L"atk_up.png",
				0, 0, 7, 18, px, py, false, 0.03f);
			std::wstring atkSidePath = base + houndPrefix + L"atk_side.png";
			m_animator->RegisterAnimation((int)BossHoundState::ATTACK, DIR_LEFT, atkSidePath,
				0, 0, 7, 18, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)BossHoundState::ATTACK, DIR_RIGHT, atkSidePath,
				0, 0, 7, 18, px, py, false, 0.03f);

			// Event registration for ATTACK
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossHoundState::ATTACK, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
					clip->AddEventFrame(7, L"attack_end"); // Match regular hound's attack frame timing
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"attack_hit") this->OnAttackHit();
						else if (eventName == L"attack_end") this->OnAttackEnd();
					});
				}
			}

			// HIT
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)BossHoundState::HIT, (Direction)dir, base + houndPrefix + L"hit_side.png",
					0, 0, 7, 27, px, py, false, 0.03f);
			}

			// DEATH
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)BossHoundState::DEATH, (Direction)dir, base + houndPrefix + L"death.png",
					0, 0, 7, 52, px, py, false, 0.03f);
			}

			// HOWL
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)BossHoundState::HOWL, (Direction)dir, base + houndPrefix + L"howl.png",
					0, 0, 7, 47, px, py, false, 0.03f);
			}
		}

		m_animator->SetState(m_state, this->transform->GetDirection());
	}
}

bool Boss_Hound::OnInteraction(GameObject* obj)
{
    return Entity::OnInteraction(obj);
}

void Boss_Hound::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator)
		return;

	// 1. 공통 애니메이션 상태 처리 (HIT, DEATH, ATTACK, HOWL 등)
	if (HandleCommonAnimationState((int)BossHoundState::HIT, (int)BossHoundState::DEATH, (int)BossHoundState::ATTACK))
		return;

	if (m_state == (int)BossHoundState::HOWL)
	{
		if (m_animator->IsAnimationDone()) ChangeState((int)BossHoundState::CHASE);
		return;
	}

	if (m_state == (int)BossHoundState::ATTACK_PRE)
	{
		if (m_animator->IsAnimationDone()) {
			ChangeState((int)BossHoundState::ATTACK);
		}
		return;
	}

	// 2. 메인 AI 로직
	if (m_state == (int)BossHoundState::CHASE)
	{
		if (m_distToPlayerSq <= (m_attackRange * m_attackRange)) {
			if (m_attackCooldownTimer <= 0.0f) ChangeState((int)BossHoundState::ATTACK_PRE);
			else ChangeState((int)BossHoundState::IDLE);
			return;
		}
	}
	else if (m_state == (int)BossHoundState::IDLE)
	{
		if (m_attackTarget && m_attackTarget->IsEnabled()) {
            // 공격 사거리 밖일 때만 울음 소리 상태로 전환
            if (m_distToPlayerSq > (m_attackRange * m_attackRange * 1.1f)) {
                ChangeState((int)BossHoundState::HOWL);
                return;
            }
            // 사거리 내에 있고 쿨다운이 끝났다면 공격 준비
            else if (m_attackCooldownTimer <= 0.0f) {
                ChangeState((int)BossHoundState::ATTACK_PRE);
                return;
            }
        }
	}
	
	// 배회 로직은 헬퍼 함수 사용
	if (m_attackTarget && m_attackTarget->IsEnabled()) {
		if (m_state != (int)BossHoundState::CHASE && m_state != (int)BossHoundState::ATTACK && m_state != (int)BossHoundState::ATTACK_PRE) {
			ChangeState((int)BossHoundState::CHASE);
		}
	} else {
		UpdateAI_Wander(deltaTime, (int)BossHoundState::RUN, (int)BossHoundState::IDLE);
	}

	// 맵 경계 체크
	if (m_state == (int)BossHoundState::CHASE || m_state == (int)BossHoundState::RUN) ClampPositionToMapBounds();
}

void Boss_Hound::UpdateMovement(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	// 애니메이션 재생 중(공격, 히트 상태)는 이동하지 않음
	if (m_state == (int)BossHoundState::ATTACK || m_state == (int)BossHoundState::ATTACK_PRE || 
		m_state == (int)BossHoundState::HIT || m_state == (int)BossHoundState::DEATH || m_state == (int)BossHoundState::HOWL)
		return;

	if (m_state == (int)BossHoundState::CHASE || m_state == (int)BossHoundState::RUN)
	{
		// 공격 사거리 내에 들어오면 즉시 멈춤
		if (m_distToPlayerSq <= (m_attackRange * m_attackRange))
		{
			m_animator->SetState((int)BossHoundState::IDLE, transform->GetDirection());
			return;
		}

		Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(newDir);
		m_animator->SetState((int)BossHoundState::RUN, transform->GetDirection());

		float speed = (m_state == (int)BossHoundState::CHASE) ? m_runSpeed : m_walkSpeed;
		float moveDist = speed * deltaTime;
		transform->SetPosition(transform->GetX() + m_dirToPlayer.X * moveDist, transform->GetY() + m_dirToPlayer.Y * moveDist);
	}
	else if (m_state == (int)BossHoundState::IDLE)
	{
		m_animator->SetState((int)BossHoundState::IDLE, transform->GetDirection());
	}

	// 맵 경계 위치 경계 체크
	if (m_state == (int)BossHoundState::CHASE || m_state == (int)BossHoundState::RUN) {
		ClampPositionToMapBounds();
	}
}

void Boss_Hound::Damaged(int damage)
{
	Entity::Damaged(damage);
	ChangeState((int)BossHoundState::HIT);
}

void Boss_Hound::OnAttackHit()
{
	if (m_state != (int)BossHoundState::ATTACK) return;
	
	// Monster 기본 클래스의 공격 처리 사용
	ProcessAttackHit(m_damage);
}

void Boss_Hound::OnAttackEnd()
{
	if (m_state != (int)BossHoundState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	ChangeState((int)BossHoundState::RUN);
}

void Boss_Hound::Die()
{
    ChangeState((int)BossHoundState::DEATH);
}


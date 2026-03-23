#include "99_Default/pch.h"
#include "Monster.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../02_GameObject/Component/Transform/Transform.h"
#include "../../../02_GameObject/Entity/Player/Player.h"
#include "../../../03_Animation/Animator.h"

Monster::Monster(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
	const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Combatant(id, x, y, pivotX, pivotY, dir, baseDir, imageName, true, true, colliderType),
	m_attackCooldownTimer(0.0f), m_walkSpeed(0.0f), m_runSpeed(0.0f),
	m_targetX(x), m_targetY(y), m_distToPlayerSq(1e10f), m_dirToPlayer(0.0f, 0.0f),
	m_aiTickTimer(0.0f),
	m_wanderRadius(200.0f), m_aggroRadius(300.0f), m_deaggroRadius(500.0f),
	m_idleTimer(0.0f), m_idleDuration(2.0f),
	m_aggroType(AggroType::ON_RANGE), m_hasBeenHit(false), m_bCanChase(true)
{
	// AI 연산 부하 분산을 위해 개체별로 0.1s ~ 0.2s 사이의 고유 틱 간격 부여
	m_aiTickInterval = 0.1f + static_cast<float>(rand()) / (static_cast<float>(RAND_MAX / 0.1f));
}

Monster::~Monster()
{
}

void Monster::Init()
{
	Combatant::Init();
}

void Monster::SetupAggro(AggroType type, float aggroRadius, float deaggroRadius)
{
	m_aggroType = type;
	m_aggroRadius = aggroRadius;
	m_deaggroRadius = deaggroRadius;
	m_hasBeenHit = false;

	// ALWAYS 타입인 경우 즉시 플레이어를 타겟으로 설정
	if (m_aggroType == AggroType::ALWAYS)
	{
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
	}
}

void Monster::Damaged(int damage)
{
	Entity::Damaged(damage);

	// ON_HIT_THEN_RANGE 타입: 피격 시 어그로 활성화
	if (m_aggroType == AggroType::ON_HIT_THEN_RANGE && !m_hasBeenHit)
	{
		m_hasBeenHit = true;
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
	}
}

void Monster::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (m_isDead)
	{
		return;
	}

	const bool hadAggroTarget = (m_attackTarget && m_attackTarget->IsEnabled());

	UpdateAI(deltaTime);

	// --- 1. [매 프레임] 정보 수집 (이동 계산에 필요하므로 매 프레임 수행) ---
	Player* player = ObjectManager::GetInstance()->GetPlayer();
	if (player && player->IsEnabled())
	{
		Transform* playerTr = player->GetComponent<Transform>();
		if (playerTr && transform)
		{
			float dx = playerTr->GetX() - transform->GetX();
			float dy = playerTr->GetY() - transform->GetY();
			m_distToPlayerSq = dx * dx + dy * dy;

			if (m_distToPlayerSq > 0.0001f)
			{
				float invDist = 1.0f / sqrtf(m_distToPlayerSq);
				m_dirToPlayer.X = dx * invDist;
				m_dirToPlayer.Y = dy * invDist;
			}
		}
	}
	else
	{
		m_distToPlayerSq = 1e10f;
		m_dirToPlayer = { 0.0f, 0.0f };
		if (m_attackTarget == player) m_attackTarget = nullptr;
	}

	// --- 2. [AI Tick] 일정 간격으로만 무거운 로직 수행 (최적화) ---
	m_aiTickTimer += deltaTime;
	if (m_aiTickTimer >= m_aiTickInterval)
	{
		// 2-1. 어그로 타겟 체크 (매 프레임 할 필요 없는 판단 로직)
		if (!m_attackTarget || !m_attackTarget->IsEnabled())
		{
			m_attackTarget = nullptr;
			if (player && player->IsEnabled())
			{
				switch (m_aggroType)
				{
				case AggroType::ON_RANGE:
					if (m_distToPlayerSq <= (m_aggroRadius * m_aggroRadius)) m_attackTarget = player;
					break;
				case AggroType::ALWAYS:
					m_attackTarget = player;
					break;
				case AggroType::ON_HIT_THEN_RANGE:
					if (m_hasBeenHit && m_distToPlayerSq <= (m_aggroRadius * m_aggroRadius)) m_attackTarget = player;
					break;
				}
			}
		}
		else if (m_aggroType != AggroType::ALWAYS)
		{
			if (m_distToPlayerSq > (m_deaggroRadius * m_deaggroRadius)) m_attackTarget = nullptr;
		}

		m_aiTickTimer = 0.0f;
	}

	const bool hasAggroTarget = (m_attackTarget && m_attackTarget->IsEnabled());
	if (hadAggroTarget && !hasAggroTarget)
	{
		ResetAggroSession();
	}

	if (m_attackCooldownTimer > 0.0f)
		m_attackCooldownTimer -= deltaTime;

	UpdateMovement(deltaTime); 
}

void Monster::OnDeathEnd()
{
	ObjectManager::GetInstance()->RemoveGameObject(this);
}

void Monster::ResolveWanderCenter(float& outX, float& outY) const
{
	if (transform)
	{
		outX = transform->GetX();
		outY = transform->GetY();
		return;
	}

	outX = 0.0f;
	outY = 0.0f;
}

bool Monster::CheckCounterAttack()
{
	if (!m_bUseSuperArmor) return false;

	if (m_attackCooldownTimer <= 0.0f && m_attackTarget && m_attackTarget->IsEnabled())
	{
		m_attackCooldownTimer = m_attackCooldown;
		TriggerAttackState();
		if (IsInAttackState())
		{
			m_bHitDuringAttack = true;
			return true;
		}
	}
	return false;
}

void Monster::MoveTowardPlayer(float deltaTime, float speed, int runAnimState, int idleState)
{
	if (!transform || !m_animator) return;

	// 공격 사거리 내에 들어오면 즉시 멈춤
	if (m_distToPlayerSq <= (m_attackRange * m_attackRange))
	{
		m_animator->SetState(idleState, transform->GetDirection());
		return;
	}

	// 방향 전환 (더 많이 차이나는 축 기준)
	Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
	transform->SetDirection(newDir);
	m_animator->SetState(runAnimState, transform->GetDirection());

	// 이동
	float moveDist = speed * deltaTime;
	transform->SetPosition(transform->GetX() + m_dirToPlayer.X * moveDist, transform->GetY() + m_dirToPlayer.Y * moveDist);

	// 맵 경계 체크
	ClampPositionToMapBounds();
}

void Monster::MoveTowardLocation(float deltaTime, float speed, int walkAnimState, int idleState)
{
	if (!transform || !m_animator) return;

	float wdx = m_targetX - transform->GetX();
	float wdy = m_targetY - transform->GetY();
	float wdistSq = wdx * wdx + wdy * wdy;

	// 도착 체크 (2px 이내)
	if (wdistSq < 4.0f) {
		ChangeState(idleState);
		return;
	}

	float wdist = sqrtf(wdistSq);
	Direction wDir = (std::abs(wdx) > std::abs(wdy)) ? (wdx > 0.0f ? DIR_RIGHT : DIR_LEFT) : (wdy > 0.0f ? DIR_DOWN : DIR_UP);
	transform->SetDirection(wDir);
	m_animator->SetState(walkAnimState, transform->GetDirection());

	float moveStep = speed * deltaTime;
	transform->SetPosition(transform->GetX() + (wdx / wdist) * moveStep, transform->GetY() + (wdy / wdist) * moveStep);

	// 맵 경계 체크
	ClampPositionToMapBounds();
}

void Monster::CheckAttackTransition(float range, int attackState, int idleState)
{
	// 사거리 계산 및 판별
	if (m_distToPlayerSq <= (range * range)) {
		// 쿨타임이 끝났다면 공격 상태로 전환
		if (m_attackCooldownTimer <= 0.0f) {
			m_attackCooldownTimer = m_attackCooldown; // 공격 시작 시 쿨타임 설정
			ChangeState(attackState);
		}
		else {
			// 사거리 내에 있지만 쿨타임 중이라면 대기(IDLE) 상태 유지
			ChangeState(idleState);
		}
	}
}

void Monster::UpdateAI_AlwaysChase(float deltaTime, int runState, int attackState, int idleState)
{
	if (m_state == runState)
	{
		CheckAttackTransition(m_attackRange, attackState, idleState);
	}
	else if (m_state == idleState)
	{
		// 공격 사거리 내에 있고 쿨다운이 끝났다면 즉시 공격 상태로 전환
		if (m_distToPlayerSq <= (m_attackRange * m_attackRange)) {
			if (m_attackCooldownTimer <= 0.0f) {
				ChangeState(attackState);
				return;
			}
			// 쿨다운 중이면 그대로 대기 (AlwaysChase는 보통 공격적인 타입)
		}

		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration)
		{
			// 공격 사거리 밖에 있을 때만 다시 추격(RUN) 상태로 전환
			// m_bCanChase 플래그 확인 추가
			if (m_bCanChase && m_distToPlayerSq > (m_attackRange * m_attackRange)) {
				ChangeState(runState);
				m_idleTimer = 0.0f;
			}
		}
	}
}

void Monster::UpdateAI_Wander(float deltaTime, int walkState, int idleState)
{
	if (m_state != idleState) return;

	float centerX = 0.0f;
	float centerY = 0.0f;
	ResolveWanderCenter(centerX, centerY);

	m_idleTimer += deltaTime;
	if (m_idleTimer >= m_idleDuration) {
		float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
		float dist = (rand() / (float)RAND_MAX) * m_wanderRadius;
		m_targetX = centerX + cosf(angle) * dist;
		m_targetY = centerY + sinf(angle) * dist;

		ChangeState(walkState);
		m_idleTimer = 0.0f;
	}
}

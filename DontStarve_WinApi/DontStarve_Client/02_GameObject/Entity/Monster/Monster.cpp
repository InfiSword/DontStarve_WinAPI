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

	// 1. 타겟 또는 플레이어와의 거리/방향 계산 (어그로 판정용)
	Player* player = ObjectManager::GetInstance()->GetPlayer();
	GameObject* targetToTrack = (m_attackTarget && m_attackTarget->IsEnabled()) ? m_attackTarget : player;

	if (targetToTrack && targetToTrack->IsEnabled()) {
		Transform* targetTr = targetToTrack->GetComponent<Transform>();
		if (targetTr && transform) {
			float dx = targetTr->GetX() - transform->GetX();
			float dy = targetTr->GetY() - transform->GetY();
			m_distToPlayerSq = dx * dx + dy * dy;
			if (m_distToPlayerSq > 0.0001f) {
				float invDist = 1.0f / sqrtf(m_distToPlayerSq);
				m_dirToPlayer.X = dx * invDist;
				m_dirToPlayer.Y = dy * invDist;
			}
		}
	}
	else {
		m_distToPlayerSq = 1e10f;
		m_dirToPlayer = { 0.0f, 0.0f };
	}

	// 2. 어그로 관리 (해제 및 시작)
	if (m_attackTarget && m_attackTarget->IsEnabled()) {
		// 어그로 해제 체크 (deaggroRadius가 설정된 경우)
		if (m_deaggroRadius > 0.0f && m_distToPlayerSq > (m_deaggroRadius * m_deaggroRadius)) {
			m_attackTarget = nullptr;
		}
	}
	else if (player && player->IsEnabled()) {
		// 어그로 시작 체크
		if (m_aggroType == AggroType::ALWAYS) {
			m_attackTarget = player;
		}
		else if (m_aggroType == AggroType::ON_RANGE ||
			(m_aggroType == AggroType::ON_HIT_THEN_RANGE && m_hasBeenHit)) {
			if (m_aggroRadius > 0.0f && m_distToPlayerSq <= (m_aggroRadius * m_aggroRadius)) {
				m_attackTarget = player;
			}
		}
	}

	const bool hasAggroTarget = (m_attackTarget && m_attackTarget->IsEnabled());
	if (hadAggroTarget && !hasAggroTarget)
	{
		ResetAggroSession();
	}

	UpdateAI(deltaTime);

	if (m_attackCooldownTimer > 0.0f)
		m_attackCooldownTimer -= deltaTime;

	UpdateMovement(deltaTime);
}

void Monster::UpdateAI(float deltaTime)
{
	if (m_isDead) return;

	int nextState = m_state;

	switch (m_state)
	{
	case (int)CombatantState::IDLE:
		nextState = UpdateIdle(deltaTime);
		break;
	case (int)CombatantState::WALK:
		nextState = UpdateWalk(deltaTime);
		break;
	case (int)CombatantState::CHASE:
		nextState = UpdateChase(deltaTime);
		break;
	case (int)CombatantState::ATTACK:
		nextState = UpdateAttack(deltaTime);
		break;
	case (int)CombatantState::HIT:
		nextState = UpdateHit(deltaTime);
		break;
	}

	ChangeState(nextState);
}

void Monster::UpdateMovement(float deltaTime)
{
	if (m_isDead) return;

	switch (m_state)
	{
	case (int)CombatantState::WALK:
		MoveTowardLocation(deltaTime, m_walkSpeed);
		break;
	case (int)CombatantState::CHASE:
		MoveTowardPlayer(deltaTime, m_runSpeed);
		break;
	}
}

int Monster::UpdateIdle(float deltaTime)
{
	if (m_attackTarget && m_attackTarget->IsEnabled())
	{
		if (m_distToPlayerSq > (m_attackRange * m_attackRange))
		{
			return (int)CombatantState::CHASE;
		}
		else if (m_attackCooldownTimer <= 0.0f)
		{
			return (int)CombatantState::ATTACK;
		}
	}
	else
	{
		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration)
		{
			float centerX, centerY;
			ResolveWanderCenter(centerX, centerY);
			float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
			float dist = (rand() / (float)RAND_MAX) * m_wanderRadius;
			m_targetX = centerX + cosf(angle) * dist;
			m_targetY = centerY + sinf(angle) * dist;
			m_idleTimer = 0.0f;
			return (int)CombatantState::WALK;
		}
	}
	return (int)CombatantState::IDLE;
}

int Monster::UpdateWalk(float deltaTime)
{
	if (m_attackTarget && m_attackTarget->IsEnabled())
	{
		return (int)CombatantState::CHASE;
	}

	if (!transform) return (int)CombatantState::IDLE;

	float wdx = m_targetX - transform->GetX();
	float wdy = m_targetY - transform->GetY();
	if (wdx * wdx + wdy * wdy < 4.0f)
	{
		m_idleTimer = 0.0f;
		return (int)CombatantState::IDLE;
	}

	return (int)CombatantState::WALK;
}

int Monster::UpdateChase(float deltaTime)
{
	if (!m_attackTarget || !m_attackTarget->IsEnabled() || !m_bCanChase)
	{
		m_idleTimer = 0.0f;
		return (int)CombatantState::IDLE;
	}

	if (m_distToPlayerSq <= (m_attackRange * m_attackRange))
	{
		if (m_attackCooldownTimer <= 0.0f)
		{
			return (int)CombatantState::ATTACK;
		}
		else
		{
			return (int)CombatantState::IDLE;
		}
	}

	return (int)CombatantState::CHASE;
}

int Monster::UpdateAttack(float deltaTime)
{
	return (int)CombatantState::ATTACK;
}

int Monster::UpdateHit(float deltaTime)
{
	return (int)CombatantState::HIT;
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

void Monster::MoveTowardPlayer(float deltaTime, float speed)
{
	if (!transform || !m_animator) return;

	// 실시간 방향 업데이트 (플레이어와의 상대적 위치 기준)
	Direction newDir = DIR_DOWN;
	if (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) {
		newDir = (m_dirToPlayer.X > 0.0f) ? DIR_RIGHT : DIR_LEFT;
	}
	else {
		newDir = (m_dirToPlayer.Y > 0.0f) ? DIR_DOWN : DIR_UP;
	}

	transform->SetDirection(newDir);
	ChangeState(m_state);

	// 이동
	float moveDist = speed * deltaTime;
	transform->SetPosition(transform->GetX() + m_dirToPlayer.X * moveDist, transform->GetY() + m_dirToPlayer.Y * moveDist);

	// 맵 경계 체크
	ClampPositionToMapBounds();
}

void Monster::MoveTowardLocation(float deltaTime, float speed)
{
	if (!transform || !m_animator) return;

	float wdx = m_targetX - transform->GetX();
	float wdy = m_targetY - transform->GetY();
	float wdistSq = wdx * wdx + wdy * wdy;

	if (wdistSq < 0.0001f) return;

	float wdist = sqrtf(wdistSq);

	// 실시간 방향 업데이트 (목표 지점 기준)
	Direction wDir = DIR_DOWN;
	if (std::abs(wdx) > std::abs(wdy)) {
		wDir = (wdx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
	}
	else {
		wDir = (wdy > 0.0f) ? DIR_DOWN : DIR_UP;
	}
	transform->SetDirection(wDir);
	ChangeState(m_state);

	float moveStep = speed * deltaTime;
	transform->SetPosition(transform->GetX() + (wdx / wdist) * moveStep, transform->GetY() + (wdy / wdist) * moveStep);

	// 맵 경계 체크
	ClampPositionToMapBounds();
}
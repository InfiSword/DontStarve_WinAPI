#include "99_Default/pch.h"
#include "Monster.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../02_GameObject/Component/Transform/Transform.h"
#include "../../../02_GameObject/Component/Collider/BoxCollider.h"
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
      m_aggroType(AggroType::ON_RANGE), m_hasBeenHit(false)
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
	Entity::Update(deltaTime); // 애니메이션 업데이트
	UpdateAI(deltaTime);       // 서브클래스에서 DEATH 완료 후 제거 등의 처리를 수행할 수 있도록 호출

	if (m_isDead) return;

	// --- 1. [매 프레임] 플레이어와의 정보 계산 (제곱 거리 사용) ---
    Player* player = ObjectManager::GetInstance()->GetPlayer();
    if (player)
    {
        Transform* playerTr = player->GetComponent<Transform>();
        if (playerTr && transform)
        {
            float dx = playerTr->GetX() - transform->GetX();
            float dy = playerTr->GetY() - transform->GetY();
            m_distToPlayerSq = dx * dx + dy * dy; // sqrtf 제거 (최적화)

            // 방향 벡터는 이동에 필요하므로 매 프레임 정규화 (최소한의 sqrt 허용)
            if (m_distToPlayerSq > 0.0001f)
            {
                float invDist = 1.0f / sqrtf(m_distToPlayerSq);
                m_dirToPlayer.X = dx * invDist;
                m_dirToPlayer.Y = dy * invDist;
            }
        }
    }

    // --- 1.5 [어그로 타입별 자동 어그로 체크] ---
    if (!m_attackTarget || !m_attackTarget->IsEnabled())
    {
        switch (m_aggroType)
        {
        case AggroType::ON_RANGE:
            // 범위 내 진입 시 어그로 획득
            if (m_distToPlayerSq <= (m_aggroRadius * m_aggroRadius))
            {
                m_attackTarget = player;
            }
            break;

        case AggroType::ALWAYS:
            // 항상 플레이어 추격
            m_attackTarget = player;
            break;

        case AggroType::ON_HIT_THEN_RANGE:
            // 한 번 맞은 적이 있다면 범위 체크 시작
            if (m_hasBeenHit && m_distToPlayerSq <= (m_aggroRadius * m_aggroRadius))
            {
                m_attackTarget = player;
            }
            break;
        }
    }
    else
    {
        // 어그로 타겟이 있을 때 deaggro 체크 (ALWAYS 타입은 제외)
        if (m_aggroType != AggroType::ALWAYS)
        {
            if (m_distToPlayerSq > (m_deaggroRadius * m_deaggroRadius))
            {
                m_attackTarget = nullptr;
                // ON_HIT_THEN_RANGE 타입은 어그로 해제 시 피격 상태 초기화하지 않음
                // (한 번 적대적이 되면 계속 범위 체크를 유지)
            }
        }
    }

    // --- 2. [AI Tick] 일정 간격으로만 AI 상태 결정 수행 ---
    m_aiTickTimer += deltaTime;
    if (m_aiTickTimer >= m_aiTickInterval)
    {
        UpdateAI(m_aiTickTimer); // 누적된 시간 전달
        m_aiTickTimer = 0.0f;
    }

    // --- 3. [매 프레임] 이동 및 쿨타임, 애니메이션 처리 ---
    if (m_attackCooldownTimer > 0.0f)
        m_attackCooldownTimer -= deltaTime;

    UpdateMovement(deltaTime); // 부드러운 이동
    Entity::Update(deltaTime); // 애니메이션 업데이트
}

void Monster::UpdateAI(float deltaTime)
{
    // Default implementation - child classes should override
}

void Monster::UpdateMovement(float deltaTime)
{
    // Default implementation - child classes should override
}

void Monster::OnAttackHit()
{
    // Default implementation - child classes should override
}

bool Monster::HandleCommonAnimationState(int hitState, int deathState, int attackState)
{
    if (m_state == hitState || m_state == deathState || (attackState != -1 && m_state == attackState))
    {
        if (m_animator && m_animator->IsAnimationDone())
        {
            if (m_state == deathState) {
                ObjectManager::GetInstance()->RemoveGameObject(this);
                return true;
            }

            if (attackState != -1 && m_state == attackState) {
                OnAttackEnd();
                // 공격이 완전히 끝난 시점에 쿨다운 시작 (공격 애니메이션 도중 쿨다운이 깎이는 것 방지)
                m_attackCooldownTimer = m_attackCooldown;
            }
            
            // 기본 상태로 복귀 (자식 클래스에서 UpdateAI 시 다시 결정됨)
            ChangeState((int)0); // 보통 0은 IDLE
        }
        return true;
    }
    return false;
}

void Monster::CheckAttackTransition(float range, int attackState, int idleState)
{
    if (m_distToPlayerSq <= (range * range)) {
        if (m_attackCooldownTimer <= 0.0f) {
            ChangeState(attackState);
            // m_attackCooldownTimer 설정은 HandleCommonAnimationState의 OnAttackEnd 이후로 이동
        }
        else {
            // 공격 사거리 내에 있지만 쿨다운 중일 때는 억지로 IDLE로 바꾸지 않고 
            // 현재 상태(보통 CHASE/RUN)를 유지하여 플레이어를 계속 따라붙게 함
            // (IDLE로 바꾸면 제자리에서 멍하니 서있게 됨)
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
            if (m_distToPlayerSq > (m_attackRange * m_attackRange)) {
                ChangeState(runState);
                m_idleTimer = 0.0f;
            }
        }
    }
}

void Monster::UpdateAI_RangeChase(float deltaTime, int idleState, int walkState, int chaseState, int attackState, int tauntState)
{
    if (m_state == chaseState)
    {
        if (!m_attackTarget || !m_attackTarget->IsEnabled()) {
            ChangeState(idleState);
            m_idleTimer = 0.0f;
            return;
        }

        CheckAttackTransition(m_attackRange, attackState, idleState);
    }
    else if (m_state == idleState)
    {
        if (m_attackTarget && m_attackTarget->IsEnabled()) {
            // 공격 사거리 밖에 있을 때만 다시 추격(CHASE) 상태로 전환 (약간의 버퍼 1.1f 사용)
            if (m_distToPlayerSq > (m_attackRange * m_attackRange * 1.1f)) {
                if (tauntState != -1) ChangeState(tauntState);
                else ChangeState(chaseState);
                return;
            }
            
            // 사거리 내에 있고 쿨다운이 끝났다면 공격
            if (m_attackCooldownTimer <= 0.0f) {
                ChangeState(attackState);
            }
            return;
        }
        UpdateAI_Wander(deltaTime, walkState, idleState);
    }
    else if (m_state == walkState)
    {
        if (m_attackTarget && m_attackTarget->IsEnabled()) {
            if (tauntState != -1) ChangeState(tauntState);
            else ChangeState(chaseState);
            return;
        }
    }
}

void Monster::UpdateAI_Wander(float deltaTime, int walkState, int idleState)
{
    if (m_state != idleState) return;

    m_idleTimer += deltaTime;
    if (m_idleTimer >= m_idleDuration) {
        float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
        float dist = (rand() / (float)RAND_MAX) * m_wanderRadius;
        m_targetX = transform->GetX() + cosf(angle) * dist;
        m_targetY = transform->GetY() + sinf(angle) * dist;

        // 맵 경계 체크 (Define.h 상수의 가시성 확인 필요, 여기서는 일반 로직만 작성)
        ChangeState(walkState);
        m_idleTimer = 0.0f;
    }
}


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
}

bool Boss_Hound::OnInteraction(GameObject* obj)
{
    return Entity::OnInteraction(obj);
}

void Boss_Hound::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator)
		return;

	// 1. 공통 애니메이션 상태 처리 (HIT, DEATH, ATTACK)
	if (HandleCommonAnimationState((int)BossHoundState::HIT, (int)BossHoundState::DEATH, (int)BossHoundState::ATTACK))
		return;

	// 2. 메인 AI 로직 (AlwaysChase 타입 헬퍼 사용)
	UpdateAI_AlwaysChase(deltaTime, 
		(int)BossHoundState::RUN, (int)BossHoundState::ATTACK, (int)BossHoundState::IDLE);
}

void Boss_Hound::UpdateMovement(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;
	if (m_state != (int)BossHoundState::RUN) return;

	// 플레이어와 너무 가까우면 이동을 멈춰 흔들림(jitter) 방지
	if (m_distToPlayerSq < (m_attackRange * m_attackRange * 0.9f))
	{
		m_animator->SetState((int)BossHoundState::IDLE, transform->GetDirection());
		return;
	}

	Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
	transform->SetDirection(newDir);
	m_animator->SetState((int)BossHoundState::RUN, transform->GetDirection());

	float moveDist = m_runSpeed * deltaTime;
	transform->SetPosition(transform->GetX() + m_dirToPlayer.X * moveDist, transform->GetY() + m_dirToPlayer.Y * moveDist);
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


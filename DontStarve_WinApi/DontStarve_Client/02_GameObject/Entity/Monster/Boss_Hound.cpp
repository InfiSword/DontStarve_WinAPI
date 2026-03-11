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

	if (m_state == (int)BossHoundState::HIT || m_state == (int)BossHoundState::DEATH)
	{
		if (m_animator->IsAnimationDone())
		{
			if (m_state == (int)BossHoundState::DEATH) {
				ObjectManager::GetInstance()->RemoveGameObject(this);
				return;
			}
			ChangeState((int)BossHoundState::IDLE);
		}
		return;
	}

	// Monster::Update에서 어그로를 자동으로 처리함(ALWAYS 타입)
	// 어그로가 켜져있을 때 항상 추격함

	if (m_state == (int)BossHoundState::RUN)
	{
		if (m_distToPlayerSq <= (m_attackRange * m_attackRange)) {
			if (m_attackCooldownTimer <= 0.0f) {
				ChangeState((int)BossHoundState::ATTACK);
				m_attackCooldownTimer = m_attackCooldown;
			}
			else {
				ChangeState((int)BossHoundState::IDLE);
			}
		}
	}
	else if (m_state == (int)BossHoundState::IDLE)
	{
		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration)
		{
			ChangeState((int)BossHoundState::RUN);
			m_idleTimer = 0.0f;
		}
	}
}

void Boss_Hound::UpdateMovement(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;
	if (m_state != (int)BossHoundState::RUN) return;

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


#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "Boss_Hound.h"

const float Boss_Hound::ATTACK_RANGE = 100.0f;
const float Boss_Hound::ATTACK_COOLDOWN = 2.0f;

Boss_Hound::Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, baseDir, imageName, colliderType)
	, m_wanderRadius(300.0f)
	, m_aggroRadius(400.0f)
	, m_deaggroRadius(600.0f)
	, m_idleTimer(0.0f)
	, m_idleDuration(2.0f)
	, m_attackCollider(nullptr)
{
	m_hp = 300;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;

	m_walkSpeed = 100.0f;
	m_runSpeed = 250.0f;
}

Boss_Hound::~Boss_Hound() {}

void Boss_Hound::Init()
{
	Monster::Init();
	
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

	// 어그로 체크
	if (!m_aggroTarget && m_distToPlayerSq <= (m_aggroRadius * m_aggroRadius))
	{
		m_aggroTarget = ObjectManager::GetInstance()->GetPlayer();
		ChangeState((int)BossHoundState::RUN);
	}

	if (m_aggroTarget && m_distToPlayerSq > (m_deaggroRadius * m_deaggroRadius))
	{
		m_aggroTarget = nullptr;
		ChangeState((int)BossHoundState::IDLE);
	}

	if (m_state == (int)BossHoundState::RUN)
	{
		if (m_distToPlayerSq <= (ATTACK_RANGE * ATTACK_RANGE)) {
			if (m_attackCooldownTimer <= 0.0f) {
				ChangeState((int)BossHoundState::ATTACK);
				m_attackCooldownTimer = ATTACK_COOLDOWN;
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

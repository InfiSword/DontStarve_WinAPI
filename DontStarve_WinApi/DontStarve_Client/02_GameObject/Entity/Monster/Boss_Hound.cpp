#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "Boss_Hound.h"

Boss_Hound::Boss_Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Entity(id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName, true, true)
	, m_wanderRadius(300.0f)
	, m_aggroRadius(400.0f)
	, m_deaggroRadius(600.0f)
	, m_walkSpeed(100.0f)
	, m_runSpeed(250.0f)
	, m_attackCooldownTimer(0.0f)
	, m_idleTimer(0.0f)
	, m_idleDuration(2.0f)
	, m_targetX(x)
	, m_targetY(y)
	, m_aggroTarget(nullptr)
	, m_attackCollider(nullptr)
{
	m_hp = 300;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
}

Boss_Hound::~Boss_Hound() {}

void Boss_Hound::Init()
{
	Entity::Init();
	
	m_state = (int)BossHoundState::IDLE;
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	
	if (!this->transform) {
		this->transform = GetComponent<Transform>();
	}
	
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

void Boss_Hound::Update(float deltaTime)
{
    Entity::Update(deltaTime);
}

void Boss_Hound::Damaged(int damage)
{
	m_hp -= damage;
	m_state = (int)BossHoundState::HIT;
	
	if (m_hp <= 0) {
        m_hp = 0;
		m_state = (int)BossHoundState::DEATH;
        m_isDead = true;
	}
}

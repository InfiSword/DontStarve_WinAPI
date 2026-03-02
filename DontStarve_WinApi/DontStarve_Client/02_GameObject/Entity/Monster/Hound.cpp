#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../../01_Manager/SceneManager/SceneManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../Player/Player.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"
#include "Hound.h"

const float Hound::ATTACK_RANGE = 70.0f;
const float Hound::ATTACK_COOLDOWN = 1.2f;

static const int HOUND_ATTACK_HIT_FRAME = 4;
static const int HOUND_ATTACK_BOX_W = 80, HOUND_ATTACK_BOX_H = 50;
static const int HOUND_ATTACK_DOWN[]  = { -40,   0, HOUND_ATTACK_BOX_W, HOUND_ATTACK_BOX_H };
static const int HOUND_ATTACK_UP[]    = { -40, -50, HOUND_ATTACK_BOX_W, HOUND_ATTACK_BOX_H };
static const int HOUND_ATTACK_LEFT[]  = { -80, -25, HOUND_ATTACK_BOX_W, HOUND_ATTACK_BOX_H };
static const int HOUND_ATTACK_RIGHT[] = {   0, -25, HOUND_ATTACK_BOX_W, HOUND_ATTACK_BOX_H };

Hound::Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Entity(id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName)
	, m_wanderRadius(200.0f)
	, m_aggroRadius(350.0f)
	, m_deaggroRadius(500.0f)
	, m_walkSpeed(80.0f)
	, m_runSpeed(200.0f)
	, m_attackCooldownTimer(0.0f)
	, m_idleTimer(0.0f)
	, m_idleDuration(2.0f)
	, m_targetX(x)
	, m_targetY(y)
	, m_aggroTarget(nullptr)
	, m_attackCollider(nullptr)
{
	m_hp = 90;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
}

Hound::~Hound() {}

void Hound::Init()
{
	Entity::Init();
	
	m_state = (int)HoundState::IDLE;
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_attackCooldownTimer = 0.0f;

	if (!this->transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Hound: Transform component not found!\n");
			return;
		}
	}

	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}
	
	OutputDebugStringW((L"Hound: Init 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(m_id);

		if (objData) {
			std::wstring base = objData->baseDir + L"\\";
			std::wstring prefix = L"Hound";
			if (m_id == GOID_MONSTER_REDHOUNDDOG) prefix = L"RedHound";
			else if (m_id == GOID_MONSTER_ICEHOUNDDOG) prefix = L"IceHound";

			float px = transform->GetPivotX();
			float py = transform->GetPivotY();
			
			m_animator->RegisterAnimation((int)HoundState::IDLE, DIR_DOWN, base + prefix + L"_hound_idle_down.png",
				0, 0, 6, 6, px, py, true, 0.03f);
			m_animator->RegisterAnimation((int)HoundState::IDLE, DIR_UP, base + prefix + L"_hound_idle_up.png",
				0, 0, 6, 6, px, py, true, 0.03f);
			std::wstring idleSidePath = base + prefix + L"_hound_idle_side.png";
			m_animator->RegisterAnimation((int)HoundState::IDLE, DIR_LEFT, idleSidePath,
				0, 0, 6, 6, px, py, true, 0.03f, false);
			m_animator->RegisterAnimation((int)HoundState::IDLE, DIR_RIGHT, idleSidePath,
				0, 0, 6, 6, px, py, true, 0.03f);

			m_animator->RegisterAnimation((int)HoundState::RUN, DIR_DOWN, base + prefix + L"_hound_run_loop_down.png",
				0, 0, 6, 6, px, py, true, 0.03f);
			m_animator->RegisterAnimation((int)HoundState::RUN, DIR_UP, base + prefix + L"_hound_run_loop_up.png",
				0, 0, 6, 6, px, py, true, 0.03f);
			std::wstring walkSidePath = base + prefix + L"_hound_run_loop_side.png";
			m_animator->RegisterAnimation((int)HoundState::RUN, DIR_LEFT, walkSidePath,
				0, 0, 6, 6, px, py, true, 0.03f, false);
			m_animator->RegisterAnimation((int)HoundState::RUN, DIR_RIGHT, walkSidePath,
				0, 0, 6, 6, px, py, true, 0.03f);

			m_animator->RegisterAnimation((int)HoundState::ATTACK_PRE, DIR_DOWN, base + prefix + L"_hound_atk_pre_down.png",
				0, 0, 4, 4, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)HoundState::ATTACK_PRE, DIR_UP, base + prefix + L"_hound_atk_pre_up.png",
				0, 0, 4, 4, px, py, false, 0.03f);
			std::wstring atkPreSidePath = base + prefix + L"_hound_atk_pre_side.png";
			m_animator->RegisterAnimation((int)HoundState::ATTACK_PRE, DIR_LEFT, atkPreSidePath,
				0, 0, 4, 4, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)HoundState::ATTACK_PRE, DIR_RIGHT, atkPreSidePath,
				0, 0, 4, 4, px, py, false, 0.03f);

			m_animator->RegisterAnimation((int)HoundState::ATTACK, DIR_DOWN, base + prefix + L"_hound_atk_down.png",
				0, 0, 8, 8, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)HoundState::ATTACK, DIR_UP, base + prefix + L"_hound_atk_up.png",
				0, 0, 8, 8, px, py, false, 0.03f);
			std::wstring atkSidePath = base + prefix + L"_hound_atk_side.png";
			m_animator->RegisterAnimation((int)HoundState::ATTACK, DIR_LEFT, atkSidePath,
				0, 0, 8, 8, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)HoundState::ATTACK, DIR_RIGHT, atkSidePath,
				0, 0, 8, 8, px, py, false, 0.03f);

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)HoundState::HIT, (Direction)dir, base + prefix + L"_hound_hit_side.png",
					0, 0, 4, 4, px, py, false, 0.03f);
			}

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)HoundState::DEATH, (Direction)dir, base + prefix + L"_hound_death.png",
					0, 0, 10, 10, px, py, false, 0.03f);
			}

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)HoundState::HOWL, (Direction)dir, base + prefix + L"_hound_howl.png",
					0, 0, 6, 36, px, py, false, 0.03f);
			}
		}

		m_animator->SetState(m_state, this->transform->GetDirection());
	}

	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		m_attackCollider->SetObjectCollider(HOUND_ATTACK_DOWN[0], HOUND_ATTACK_DOWN[1], HOUND_ATTACK_DOWN[2], HOUND_ATTACK_DOWN[3]);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Hound::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (!IsEnabled() || !transform || !m_animator)
		return;

	if (m_attackCooldownTimer > 0.0f) {
		m_attackCooldownTimer -= deltaTime;
	}

	if (m_state == (int)HoundState::HIT)
	{
		m_animator->SetState((int)HoundState::HIT, transform->GetDirection());
		if (m_animator->IsAnimationDone())
		{
			if (m_aggroTarget && m_aggroTarget->IsEnabled())
			{
				m_state = (int)HoundState::CHASE;
				m_attackCooldownTimer = 0.0f;
			}
			else
			{
				m_state = (int)HoundState::IDLE;
			}
		}
		return;
	}

	if (m_state == (int)HoundState::HOWL)
	{
		m_animator->SetState((int)HoundState::HOWL, transform->GetDirection());
		if (m_animator->IsAnimationDone())
		{
			m_state = (int)HoundState::CHASE;
		}
		return;
	}

	if (m_state == (int)HoundState::DEATH)
	{
		m_animator->SetState((int)HoundState::DEATH, transform->GetDirection());
		
		if (m_animator->IsAnimationDone())
		{
			ObjectManager::GetInstance()->RemoveGameObject(this);
		}
		return;
	}

	GameObject* player = ObjectManager::GetInstance()->GetPlayer();
	if (player && player->IsEnabled())
	{
		Transform* playerTransform = player->GetComponent<Transform>();
		if (playerTransform)
		{
			float dx = playerTransform->GetX() - transform->GetX();
			float dy = playerTransform->GetY() - transform->GetY();
			float distToPlayer = sqrtf(dx * dx + dy * dy);

			if (!m_aggroTarget && distToPlayer <= m_aggroRadius)
			{
				m_aggroTarget = player;
				m_state = (int)HoundState::HOWL;

				Direction newDir;
				if (std::abs(dx) > std::abs(dy))
					newDir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
				else
					newDir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
				transform->SetDirection(newDir);
			}

			if (m_aggroTarget && distToPlayer > m_deaggroRadius)
			{
				m_aggroTarget = nullptr;
				m_state = (int)HoundState::IDLE;
				m_idleTimer = 0.0f;
				m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
				m_attackCooldownTimer = 0.0f;
			}
		}
	}

	if (m_state == (int)HoundState::CHASE)
	{
		if (!m_aggroTarget || !m_aggroTarget->IsEnabled()) {
			m_aggroTarget = nullptr;
			m_state = (int)HoundState::IDLE;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
			m_attackCooldownTimer = 0.0f;
			return;
		}

		Transform* targetTr = m_aggroTarget->GetComponent<Transform>();
		if (!targetTr) {
			m_aggroTarget = nullptr;
			m_state = (int)HoundState::IDLE;
			return;
		}

		float tx = targetTr->GetX();
		float ty = targetTr->GetY();
		float cx = transform->GetX();
		float cy = transform->GetY();
		float dx = tx - cx;
		float dy = ty - cy;
		float distance = sqrtf(dx * dx + dy * dy);

		Direction newDir;
		if (distance < 0.0001f) 
			newDir = transform->GetDirection();
		else if (std::abs(dx) > std::abs(dy)) 
			newDir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
		else 
			newDir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;

		if (distance <= ATTACK_RANGE && m_attackCooldownTimer <= 0.0f) {
			transform->SetDirection(newDir);
			m_state = (int)HoundState::ATTACK_PRE;
			m_animator->SetState((int)HoundState::ATTACK_PRE, transform->GetDirection());
			return;
		}

		if (distance <= ATTACK_RANGE && m_attackCooldownTimer > 0.0f) {
			transform->SetDirection(newDir);
			m_animator->SetState((int)HoundState::IDLE, transform->GetDirection());
			return;
		}

		if (transform->GetDirection() != newDir) 
			transform->SetDirection(newDir);
		m_animator->SetState((int)HoundState::RUN, transform->GetDirection());

		float moveDist = m_runSpeed * deltaTime;
		float step = (std::min)(moveDist, distance);
		if (distance > 0.0001f) {
			float nx = cx + (dx / distance) * step;
			float ny = cy + (dy / distance) * step;
			transform->SetPosition(nx, ny);
		}
		return;
	}

	if (m_state == (int)HoundState::ATTACK_PRE)
	{
		m_animator->SetState((int)HoundState::ATTACK_PRE, transform->GetDirection());
		if (m_animator->IsAnimationDone())
		{
			m_state = (int)HoundState::ATTACK;
			m_animator->SetState((int)HoundState::ATTACK, transform->GetDirection());
			m_attackCooldownTimer = ATTACK_COOLDOWN;
		}
		return;
	}

	if (m_state == (int)HoundState::ATTACK)
	{
		m_animator->SetState((int)HoundState::ATTACK, transform->GetDirection());
		if (m_attackCollider && transform) {
			Direction dir = transform->GetDirection();
			if (dir == DIR_DOWN) m_attackCollider->SetObjectCollider(HOUND_ATTACK_DOWN[0], HOUND_ATTACK_DOWN[1], HOUND_ATTACK_DOWN[2], HOUND_ATTACK_DOWN[3]);
			else if (dir == DIR_UP) m_attackCollider->SetObjectCollider(HOUND_ATTACK_UP[0], HOUND_ATTACK_UP[1], HOUND_ATTACK_UP[2], HOUND_ATTACK_UP[3]);
			else if (dir == DIR_LEFT) m_attackCollider->SetObjectCollider(HOUND_ATTACK_LEFT[0], HOUND_ATTACK_LEFT[1], HOUND_ATTACK_LEFT[2], HOUND_ATTACK_LEFT[3]);
			else m_attackCollider->SetObjectCollider(HOUND_ATTACK_RIGHT[0], HOUND_ATTACK_RIGHT[1], HOUND_ATTACK_RIGHT[2], HOUND_ATTACK_RIGHT[3]);

			int frameIdx = m_animator->GetCurrentFrameIndex();
			if (frameIdx == HOUND_ATTACK_HIT_FRAME) {
				m_attackCollider->SetColliderEnabled(true);
				if (m_aggroTarget && m_aggroTarget->IsEnabled()) {
					Transform* pt = m_aggroTarget->GetComponent<Transform>();
					if (pt && m_attackCollider->ContainsPoint(pt->GetX(), pt->GetY())) {
						Entity* playerEntity = dynamic_cast<Entity*>(m_aggroTarget);
						if (playerEntity) playerEntity->Damaged(20);
					}
				}
			}
			else
				m_attackCollider->SetColliderEnabled(false);
		}
		
		if (m_animator->IsAnimationDone()) {
			if (m_attackCollider) 
				m_attackCollider->SetColliderEnabled(false);
			m_state = (int)HoundState::CHASE;
		}
		return;
	}

	if (m_state == (int)HoundState::IDLE)
	{
		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration)
		{
			float centerX = transform->GetX();
			float centerY = transform->GetY();

			float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
			float dist = (rand() / (float)RAND_MAX) * m_wanderRadius;
			m_targetX = centerX + cosf(angle) * dist;
			m_targetY = centerY + sinf(angle) * dist;

			m_state = (int)HoundState::RUN;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
		}
		else
		{
			m_animator->SetState((int)HoundState::IDLE, transform->GetDirection());
		}
		return;
	}

	if (m_state == (int)HoundState::RUN)
	{
		float cx = transform->GetX();
		float cy = transform->GetY();
		float dx = m_targetX - cx;
		float dy = m_targetY - cy;
		float distance = sqrtf(dx * dx + dy * dy);

		Direction newDir;
		if (distance < 0.0001f)
			newDir = transform->GetDirection();
		else if (std::abs(dx) > std::abs(dy))
			newDir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
		else
			newDir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
		
		if (transform->GetDirection() != newDir)
			transform->SetDirection(newDir);
		m_animator->SetState((int)HoundState::RUN, transform->GetDirection());

		const float arrivalEpsilon = 2.0f;
		float moveDist = m_walkSpeed * deltaTime;
		bool arrived = (distance < arrivalEpsilon) || (distance <= moveDist);

		if (arrived)
		{
			transform->SetPosition(m_targetX, m_targetY);
			m_state = (int)HoundState::IDLE;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
		}
		else
		{
			float step = (std::min)(moveDist, distance);
			float nx = cx + (dx / distance) * step;
			float ny = cy + (dy / distance) * step;
			transform->SetPosition(nx, ny);
		}
	}
}

bool Hound::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Hound::Damaged(int damage)
{
	m_hp -= damage;
	m_state = (int)HoundState::HIT;

	if (m_hp <= 0) {
		m_hp = 0;
		m_state = (int)HoundState::DEATH;
		m_isDead = true;
		SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
		GameProgressManager::GetInstance()->OnMonsterKilled(GetID(), currentScene);
	}

	if (!IsDead() && IsEnabled()) {
		m_aggroTarget = ObjectManager::GetInstance()->GetPlayer();
		m_attackCooldownTimer = 0.0f;
	}
}

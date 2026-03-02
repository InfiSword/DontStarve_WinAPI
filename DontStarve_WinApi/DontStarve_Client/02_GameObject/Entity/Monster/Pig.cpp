#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/SceneManager/SceneManager.h"
#include "../../../01_Manager/SceneManager/GameScene.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"
#include "Pig.h"

const float Pig::ATTACK_RANGE = 70.0f;
const float Pig::ATTACK_COOLDOWN = 1.5f;
static const int PIG_ATTACK_HIT_FRAME = 15;
static const int PIG_ATTACK_BOX_W = 70, PIG_ATTACK_BOX_H = 50;
static const int PIG_ATTACK_DOWN[]  = { -35,    0, PIG_ATTACK_BOX_W, PIG_ATTACK_BOX_H };
static const int PIG_ATTACK_UP[]    = { -35,  -50, PIG_ATTACK_BOX_W, PIG_ATTACK_BOX_H };
static const int PIG_ATTACK_LEFT[]  = { -70,  -25, PIG_ATTACK_BOX_W, PIG_ATTACK_BOX_H };
static const int PIG_ATTACK_RIGHT[] = {   0,  -25, PIG_ATTACK_BOX_W, PIG_ATTACK_BOX_H };

Pig::Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
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
	m_hp = 100;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
}

Pig::~Pig() {}

void Pig::Init()
{
	Entity::Init();

	m_state = (int)PigState::IDLE;
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_attackCooldownTimer = 0.0f;
	
	if (!this->transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Pig: Transform component not found!\n");
			return;
		}
	}
	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}
	
	OutputDebugStringW((L"Pig: Init 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_PIG);
		if (m_id == GOID_MONSTER_PIG && objData) {
			std::wstring base = objData->baseDir + L"\\";
			
			std::wstring baseAction = base + L"Action\\";
			m_animator->RegisterAnimation((int)PigState::IDLE, DIR_DOWN, baseAction + L"pig_pigman_idle_loop_down.png",
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			m_animator->RegisterAnimation((int)PigState::IDLE, DIR_UP, baseAction + L"pig_pigman_idle_loop_up.png",
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			std::wstring idleSidePath = baseAction + L"pig_pigman_idle_loop_side.png";
			m_animator->RegisterAnimation((int)PigState::IDLE, DIR_LEFT, idleSidePath,
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f, false);
			m_animator->RegisterAnimation((int)PigState::IDLE, DIR_RIGHT, idleSidePath,
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);

			m_animator->RegisterAnimation((int)PigState::WALK, DIR_DOWN, baseAction + L"pig_pigman_walk_loop_down.png",
				0, 0, 4, 41, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			m_animator->RegisterAnimation((int)PigState::WALK, DIR_UP, baseAction + L"pig_pigman_walk_loop_up.png",
				0, 0, 4, 41, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			std::wstring walkSidePath = baseAction + L"pig_pigman_walk_loop_side.png";
			m_animator->RegisterAnimation((int)PigState::WALK, DIR_LEFT, walkSidePath,
				0, 0, 4, 41, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f, false);
			m_animator->RegisterAnimation((int)PigState::WALK, DIR_RIGHT, walkSidePath,
				0, 0, 4, 41, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);

			m_animator->RegisterAnimation((int)PigState::RUN, DIR_DOWN, baseAction + L"pig_pigman_run_loop_down.png",
				225, 199, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			m_animator->RegisterAnimation((int)PigState::RUN, DIR_UP, baseAction + L"pig_pigman_run_loop_up.png",
				225, 195, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
            std::wstring runSidePath = baseAction + L"pig_pigman_run_loop_side.png";
			m_animator->RegisterAnimation((int)PigState::RUN, DIR_LEFT, runSidePath,
				222, 200, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f, false);
			m_animator->RegisterAnimation((int)PigState::RUN, DIR_RIGHT, runSidePath,
				222, 200, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);

			std::wstring baseAttack = base + L"Attack\\";
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_DOWN, baseAttack + L"down_pigman_atk_down.png",
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_UP, baseAttack + L"up_pigman_atk_up.png",
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			std::wstring atkSidePath = baseAttack + L"side_pigman_atk_side.png";
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_LEFT, atkSidePath,
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f, false);
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_RIGHT, atkSidePath,
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);

			m_animator->RegisterAnimation((int)PigState::HIT, DIR_DOWN, base + L"Hit\\Hit_pigman_hit.png",
				232, 245, 4, 29, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			m_animator->RegisterAnimation((int)PigState::DEATH, DIR_DOWN, base + L"Death\\Death_pigman_death.png",
				227, 243, 4, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
		}

		m_animator->SetState(m_state, this->transform->GetDirection());
	}

	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		m_attackCollider->SetObjectCollider(PIG_ATTACK_DOWN[0], PIG_ATTACK_DOWN[1], PIG_ATTACK_DOWN[2], PIG_ATTACK_DOWN[3]);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Pig::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (!IsEnabled() || !transform || !m_animator)
		return;

	if (m_attackCooldownTimer > 0.0f) {
		m_attackCooldownTimer -= deltaTime;
	}

	if (m_state == (int)PigState::HIT)
	{
		m_animator->SetState((int)PigState::HIT, transform->GetDirection());
		if (m_animator->IsAnimationDone())
		{
			if (m_aggroTarget && m_aggroTarget->IsEnabled())
			{
				m_state = (int)PigState::CHASE;
				m_attackCooldownTimer = 0.0f;
			}
			else
			{
				m_state = (int)PigState::IDLE;
			}
		}
		return;
	}

	if (m_state == (int)PigState::DEATH)
	{
		m_animator->SetState((int)PigState::DEATH, transform->GetDirection());
		
		if (m_animator->IsAnimationDone())
		{
			ObjectManager::GetInstance()->RemoveGameObject(this);
		}
		return;
	}

	if (m_state == (int)PigState::CHASE)
	{
		if (!m_aggroTarget || !m_aggroTarget->IsEnabled()) {
			m_aggroTarget = nullptr;
			m_state = (int)PigState::IDLE;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
			m_attackCooldownTimer = 0.0f;
			return;
		}
		Transform* targetTr = m_aggroTarget->GetComponent<Transform>();
		if (!targetTr) {
			m_aggroTarget = nullptr;
			m_state = (int)PigState::IDLE;
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
			m_state = (int)PigState::ATTACK;
			m_animator->SetState((int)PigState::ATTACK, transform->GetDirection());
			m_attackCooldownTimer = ATTACK_COOLDOWN;
			return;
		}

		if (distance <= ATTACK_RANGE && m_attackCooldownTimer > 0.0f) {
			transform->SetDirection(newDir);
			m_animator->SetState((int)PigState::IDLE, transform->GetDirection());
			return;
		}

		if (transform->GetDirection() != newDir) 
			transform->SetDirection(newDir);
		m_animator->SetState((int)PigState::RUN, transform->GetDirection());

		float moveDist = m_runSpeed * deltaTime;
		float step = (std::min)(moveDist, distance);
		if (distance > 0.0001f) {
			float nx = cx + (dx / distance) * step;
			float ny = cy + (dy / distance) * step;
			transform->SetPosition(nx, ny);
		}
		return;
	}

	if (m_state == (int)PigState::ATTACK)
	{
		m_animator->SetState((int)PigState::ATTACK, transform->GetDirection());
		if (m_attackCollider && transform) {
			Direction dir = transform->GetDirection();
			if (dir == DIR_DOWN) m_attackCollider->SetObjectCollider(PIG_ATTACK_DOWN[0], PIG_ATTACK_DOWN[1], PIG_ATTACK_DOWN[2], PIG_ATTACK_DOWN[3]);
			else if (dir == DIR_UP) m_attackCollider->SetObjectCollider(PIG_ATTACK_UP[0], PIG_ATTACK_UP[1], PIG_ATTACK_UP[2], PIG_ATTACK_UP[3]);
			else if (dir == DIR_LEFT) m_attackCollider->SetObjectCollider(PIG_ATTACK_LEFT[0], PIG_ATTACK_LEFT[1], PIG_ATTACK_LEFT[2], PIG_ATTACK_LEFT[3]);
			else m_attackCollider->SetObjectCollider(PIG_ATTACK_RIGHT[0], PIG_ATTACK_RIGHT[1], PIG_ATTACK_RIGHT[2], PIG_ATTACK_RIGHT[3]);

			int frameIdx = m_animator->GetCurrentFrameIndex();
			if (frameIdx == PIG_ATTACK_HIT_FRAME) {
				m_attackCollider->SetColliderEnabled(true);
				if (m_aggroTarget && m_aggroTarget->IsEnabled()) {
					Transform* pt = m_aggroTarget->GetComponent<Transform>();
					if (pt && m_attackCollider->ContainsPoint(pt->GetX(), pt->GetY())) {
						Entity* playerEntity = dynamic_cast<Entity*>(m_aggroTarget);
						if (playerEntity) playerEntity->Damaged(10);
					}
				}
			}
			else
				m_attackCollider->SetColliderEnabled(false);
		}
		
		if (m_animator->IsAnimationDone()) {
			if (m_attackCollider) 
				m_attackCollider->SetColliderEnabled(false);
			m_state = (int)PigState::CHASE;
		}
		return;
	}

	if (m_state == (int)PigState::IDLE)
	{
		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration)
		{
			float cx = transform->GetX();
			float cy = transform->GetY();
			float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
			float dist = (rand() / (float)RAND_MAX) * m_wanderRadius;
			m_targetX = cx + cosf(angle) * dist;
			m_targetY = cy + sinf(angle) * dist;

			m_state = (int)PigState::WALK;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
		}
		else
		{
			m_animator->SetState((int)PigState::IDLE, transform->GetDirection());
		}
		return;
	}

	if (m_state == (int)PigState::WALK)
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
		m_animator->SetState((int)PigState::WALK, transform->GetDirection());

		const float arrivalEpsilon = 2.0f;
		float moveDist = m_walkSpeed * deltaTime;
		bool arrived = (distance < arrivalEpsilon) || (distance <= moveDist);

		if (arrived)
		{
			transform->SetPosition(m_targetX, m_targetY);
			m_state = (int)PigState::IDLE;
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

bool Pig::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Pig::Damaged(int damage)
{
	m_hp -= damage;
	m_state = (int)PigState::HIT;

	if (m_hp <= 0) {
		m_hp = 0;
		m_state = (int)PigState::DEATH;
		m_isDead = true;
		SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
		GameProgressManager::GetInstance()->OnMonsterKilled(GetID(), currentScene);
	}

	if (!IsDead() && IsEnabled()) {
		m_aggroTarget = ObjectManager::GetInstance()->GetPlayer();
		m_attackCooldownTimer = 0.0f;
	}
}

void Pig::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	// 행동 반경 원 (보라색)
	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());
	float r = GetActionRadius();
	Gdiplus::RectF circleRect(
		screenCenter.X - r,
		screenCenter.Y - r,
		r * 2.0f,
		r * 2.0f
	);
	renderManager->AddDrawEllipseCommand(
		circleRect,
		Gdiplus::Color(200, 100, 200, 255),
		2.0f,
		LAYER_DEBUG_OVERLAY,
		9998.0f
	);
}

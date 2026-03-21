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

Pig::Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
	const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
{
	m_hp = 100;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
	m_walkSpeed = 80.0f;
	m_runSpeed = 200.0f;
	m_attackRange = 70.0f;
	m_attackCooldown = 1.5f;
	m_attackHitFrame = 28;
	m_damage = 15;
	m_attackBoxWidth = 70;
	m_attackBoxHeight = 50;
}

Pig::~Pig() {}

void Pig::Init()
{
	Monster::Init();
	SetupAggro(AggroType::ON_HIT_THEN_RANGE, 250.0f, 600.0f);
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)PigState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;

	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}

	if (!m_animator) m_animator = AddComponent<Animator>();
	if (m_animator)
	{
		ResourceManager* pRM = ResourceManager::GetInstance();
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_PIG);
		if (objData) {
			std::wstring base = objData->baseDir + L"\\";
			std::wstring baseAction = base + L"Action\\";
			
			m_animator->RegisterAnimation((int)PigState::IDLE, DIR_DOWN, baseAction + L"pig_pigman_idle_loop_down.png", 0, 0, 4, 33, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);
			m_animator->RegisterAnimation((int)PigState::IDLE, DIR_UP, baseAction + L"pig_pigman_idle_loop_up.png", 0, 0, 4, 33, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);
			std::wstring idleSidePath = baseAction + L"pig_pigman_idle_loop_side.png";
			m_animator->RegisterAnimation((int)PigState::IDLE, DIR_LEFT, idleSidePath, 0, 0, 4, 33, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f, false);
			m_animator->RegisterAnimation((int)PigState::IDLE, DIR_RIGHT, idleSidePath, 0, 0, 4, 33, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);

			m_animator->RegisterAnimation((int)PigState::WALK, DIR_DOWN, baseAction + L"pig_pigman_walk_loop_down.png", 0, 0, 4, 41, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);
			m_animator->RegisterAnimation((int)PigState::WALK, DIR_UP, baseAction + L"pig_pigman_walk_loop_up.png", 0, 0, 4, 41, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);
			std::wstring walkSidePath = baseAction + L"pig_pigman_walk_loop_side.png";
			m_animator->RegisterAnimation((int)PigState::WALK, DIR_LEFT, walkSidePath, 0, 0, 4, 41, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f, false);
			m_animator->RegisterAnimation((int)PigState::WALK, DIR_RIGHT, walkSidePath, 0, 0, 4, 41, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);

			m_animator->RegisterAnimation((int)PigState::RUN, DIR_DOWN, baseAction + L"pig_pigman_run_loop_down.png", 225, 198, 4, 33, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);
			m_animator->RegisterAnimation((int)PigState::RUN, DIR_UP, baseAction + L"pig_pigman_run_loop_up.png", 225, 195, 4, 33, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);
			std::wstring runSidePath = baseAction + L"pig_pigman_run_loop_side.png";
			m_animator->RegisterAnimation((int)PigState::RUN, DIR_LEFT, runSidePath, 222, 200, 4, 33, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f, false);
			m_animator->RegisterAnimation((int)PigState::RUN, DIR_RIGHT, runSidePath, 222, 200, 4, 33, transform->GetPivotX(), transform->GetPivotY(), true, 0.025f);

			std::wstring baseAttack = base + L"Attack\\";
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_DOWN, baseAttack + L"down_pigman_atk_down.png", 0, 0, 4, 66, transform->GetPivotX(), transform->GetPivotY(), false, 0.025f);
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_UP, baseAttack + L"up_pigman_atk_up.png", 0, 0, 4, 66, transform->GetPivotX(), transform->GetPivotY(), false, 0.025f);
			std::wstring atkSidePath = baseAttack + L"side_pigman_atk_side.png";
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_LEFT, atkSidePath, 0, 0, 4, 66, transform->GetPivotX(), transform->GetPivotY(), false, 0.025f, false);
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_RIGHT, atkSidePath, 0, 0, 4, 66, transform->GetPivotX(), transform->GetPivotY(), false, 0.025f);

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				AnimationClip* clip = m_animator->GetAnimationClip((int)PigState::ATTACK, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
					clip->AddEventFrame(65, L"attack_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"attack_hit") this->OnAttackHit();
						else if (eventName == L"attack_end") this->OnAttackEnd();
						});
				}
			}

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)PigState::HIT, (Direction)dir, base + L"Hit\\Hit_pigman_hit.png", 232, 245, 4, 29, transform->GetPivotX(), transform->GetPivotY(), false, 0.03f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)PigState::HIT, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(28, L"hit_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"hit_end") this->OnHitEnd();
						});
				}
			}
			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)PigState::DEATH, (Direction)dir, base + L"Death\\Death_pigman_death.png", 227, 243, 4, 64, transform->GetPivotX(), transform->GetPivotY(), false, 0.03f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)PigState::DEATH, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(63, L"death_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"death_end") this->OnDeathEnd();
						});
				}
			}
		}
		m_animator->SetState(m_state, this->transform->GetDirection());
	}

	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		UpdateAttackBoxByDirection(DIR_DOWN);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Pig::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	switch ((PigState)m_state)
	{
	case PigState::CHASE:
		if (!m_attackTarget || !m_attackTarget->IsEnabled()) { 
			ChangeState((int)PigState::IDLE); 
			m_idleTimer = 0.0f; 
		}
		else {
			// 개선된 CheckAttackTransition 사용: 사거리 내 진입 시 공격 또는 IDLE로 자동 전환
			CheckAttackTransition(m_attackRange, (int)PigState::ATTACK, (int)PigState::IDLE);
		}
		break;
	case PigState::IDLE:
		if (m_attackTarget && m_attackTarget->IsEnabled()) {
			// 사거리 밖으로 나가면 다시 추격
			if (m_distToPlayerSq > (m_attackRange * m_attackRange * 1.1f)) {
				ChangeState((int)PigState::CHASE);
			}
			// 사거리 내에 있다면 공격 전환 시도
			else {
				CheckAttackTransition(m_attackRange, (int)PigState::ATTACK, (int)PigState::IDLE);
			}
		}
		else UpdateAI_Wander(deltaTime, (int)PigState::WALK, (int)PigState::IDLE);
		break;
	case PigState::WALK:
		if (m_attackTarget && m_attackTarget->IsEnabled()) ChangeState((int)PigState::CHASE);
		break;
	}

	if (m_state == (int)PigState::CHASE || m_state == (int)PigState::WALK)
		ClampPositionToMapBounds();
}

void Pig::UpdateMovement(float deltaTime)
{
	if (!IsEnabled()) return;

	switch ((PigState)m_state)
	{
	case PigState::CHASE: MoveTowardPlayer(deltaTime, m_runSpeed, (int)PigState::RUN, (int)PigState::IDLE); break;
	case PigState::WALK: MoveTowardLocation(deltaTime, m_walkSpeed, (int)PigState::WALK, (int)PigState::IDLE); break;
	case PigState::IDLE: 
		// IDLE 상태에서도 플레이어를 바라보게 업데이트
		if (m_attackTarget && m_attackTarget->IsEnabled()) {
			Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
			transform->SetDirection(newDir);
		}
		m_animator->SetState((int)PigState::IDLE, transform->GetDirection()); 
		break;
	}
}

void Pig::Damaged(int damage)
{
	Entity::Damaged(damage);
	if (!IsDead()) {
		ChangeState((int)PigState::HIT);
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
	}
}

void Pig::ChangeState(int newState)
{
	Monster::ChangeState(newState);
}

void Pig::OnAttackHit() { if (m_state == (int)PigState::ATTACK) ProcessAttackHit(m_damage); }

void Pig::OnAttackEnd()
{
	if (m_state != (int)PigState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	
	// 공격 종료 후에도 사거리 내에 있다면 IDLE로 전환하여 쿨타임을 기다림
	if (m_attackTarget && m_attackTarget->IsEnabled() && m_distToPlayerSq <= (m_attackRange * m_attackRange * 1.1f)) {
		ChangeState((int)PigState::IDLE);
	} else {
		ChangeState((int)PigState::CHASE);
	}
}

void Pig::OnHitEnd()
{
    if (m_state != (int)PigState::HIT) return;
    ChangeState((int)PigState::IDLE);
}

void Pig::Die() { ChangeState((int)PigState::DEATH); }

bool Pig::OnInteraction(GameObject* obj) { return Entity::OnInteraction(obj); }

void Pig::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());
	float rWander = m_wanderRadius;
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - rWander, screenCenter.Y - rWander, rWander * 2.0f, rWander * 2.0f), Gdiplus::Color(100, 200, 100, 255), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);

	if (m_state == (int)PigState::ATTACK && m_attackCollider) {
		UpdateAttackBoxByDirection(transform->GetDirection());
		RECT worldRect = m_attackCollider->GetWorldBoundingBox();
		Gdiplus::PointF topLeft = cameraManager->WorldToScreen((float)worldRect.left, (float)worldRect.top);
		Gdiplus::PointF bottomRight = cameraManager->WorldToScreen((float)worldRect.right, (float)worldRect.bottom);
		renderManager->AddDrawRectCommand(Gdiplus::RectF(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y), Gdiplus::Color(255, 0, 0), 2.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
	}
}

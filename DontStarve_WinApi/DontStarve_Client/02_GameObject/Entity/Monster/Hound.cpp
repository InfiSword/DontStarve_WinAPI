#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"
#include "Hound.h"

Hound::Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
             const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
	, m_bHasHowled(false)
{
	m_hp = 90;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
	m_walkSpeed = 80.0f;
	m_runSpeed = 200.0f;
	m_attackRange = 70.0f;
	m_attackCooldown = 1.2f;
	m_attackHitFrame = 4;
	m_damage = 20;
	m_attackBoxWidth = 80;
	m_attackBoxHeight = 50;
}

Hound::~Hound() {}

void Hound::Init()
{
	Monster::Init();
	SetupAggro(AggroType::ALWAYS, 0.0f, 0.0f);
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)HoundState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_bHasHowled = false;

	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}

	if (!m_animator) m_animator = AddComponent<Animator>();
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(m_id);
		if (objData) {
			std::wstring base = objData->baseDir + L"\\";
			float px = transform->GetPivotX();
			float py = transform->GetPivotY();
			
			m_animator->RegisterAnimation((int)HoundState::IDLE, DIR_DOWN, base + L"Hound_hound_idle_down.png", 0, 0, 7, 20, px, py, true, 0.02f);
			m_animator->RegisterAnimation((int)HoundState::IDLE, DIR_UP, base + L"Hound_hound_idle_up.png", 0, 0, 7, 20, px, py, true, 0.02f);
			std::wstring idleSidePath = base + L"Hound_hound_idle_side.png";
			m_animator->RegisterAnimation((int)HoundState::IDLE, DIR_LEFT, idleSidePath, 0, 0, 7, 20, px, py, true, 0.02f, false);
			m_animator->RegisterAnimation((int)HoundState::IDLE, DIR_RIGHT, idleSidePath, 0, 0, 7, 20, px, py, true, 0.02f);

			m_animator->RegisterAnimation((int)HoundState::RUN, DIR_DOWN, base + L"Hound_hound_run_loop_down.png", 0, 0, 7, 16, px, py, true, 0.02f);
			m_animator->RegisterAnimation((int)HoundState::RUN, DIR_UP, base + L"Hound_hound_run_loop_up.png", 0, 0, 7, 16, px, py, true, 0.02f);
			std::wstring walkSidePath = base + L"Hound_hound_run_loop_side.png";
			m_animator->RegisterAnimation((int)HoundState::RUN, DIR_LEFT, walkSidePath, 0, 0, 7, 16, px, py, true, 0.02f, false);
			m_animator->RegisterAnimation((int)HoundState::RUN, DIR_RIGHT, walkSidePath, 0, 0, 7, 16, px, py, true, 0.02f);

			m_animator->RegisterAnimation((int)HoundState::ATTACK_PRE, DIR_DOWN, base + L"Hound_hound_atk_pre_down.png", 0, 0, 7, 29, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)HoundState::ATTACK_PRE, DIR_UP, base + L"Hound_hound_atk_pre_up.png", 0, 0, 7, 29, px, py, false, 0.03f);
			std::wstring atkPreSidePath = base + L"Hound_hound_atk_pre_side.png";
			m_animator->RegisterAnimation((int)HoundState::ATTACK_PRE, DIR_LEFT, atkPreSidePath, 0, 0, 7, 29, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)HoundState::ATTACK_PRE, DIR_RIGHT, atkPreSidePath, 0, 0, 7, 29, px, py, false, 0.03f);

			m_animator->RegisterAnimation((int)HoundState::ATTACK, DIR_DOWN, base + L"Hound_hound_atk_down.png", 0, 0, 7, 18, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)HoundState::ATTACK, DIR_UP, base + L"Hound_hound_atk_up.png", 0, 0, 7, 18, px, py, false, 0.03f);
			std::wstring atkSidePath = base + L"Hound_hound_atk_side.png";
			m_animator->RegisterAnimation((int)HoundState::ATTACK, DIR_LEFT, atkSidePath, 0, 0, 7, 18, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)HoundState::ATTACK, DIR_RIGHT, atkSidePath, 0, 0, 7, 18, px, py, false, 0.03f);

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				AnimationClip* clip = m_animator->GetAnimationClip((int)HoundState::ATTACK, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
					clip->AddEventFrame(17, L"attack_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"attack_hit") this->OnAttackHit();
						else if (eventName == L"attack_end") this->OnAttackEnd();
					});
				}
			}

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)HoundState::HIT, (Direction)dir, base + L"Hound_hound_hit_side.png", 0, 0, 7, 27, px, py, false, 0.03f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)HoundState::HIT, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(26, L"hit_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"hit_end") this->OnHitEnd();
					});
				}
			}

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)HoundState::DEATH, (Direction)dir, base + L"Hound_hound_death.png", 0, 0, 7, 52, px, py, false, 0.03f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)HoundState::DEATH, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(51, L"death_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"death_end") this->OnDeathEnd();
					});
				}
			}

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) 
				m_animator->RegisterAnimation((int)HoundState::HOWL, (Direction)dir, base + L"Hound_hound_howl.png", 0, 0, 7, 47, px, py, false, 0.03f);
		}
		m_animator->SetState(m_state, this->transform->GetDirection());
	}

	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		UpdateAttackBoxByDirection(DIR_DOWN);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Hound::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	switch ((HoundState)m_state)
	{
	case HoundState::HOWL:
		if (m_animator->IsAnimationDone()) ChangeState((int)HoundState::CHASE);
		break;
	case HoundState::ATTACK_PRE:
		if (m_animator->IsAnimationDone()) ChangeState((int)HoundState::ATTACK);
		break;
	case HoundState::ATTACK:
		if (m_animator->IsAnimationDone()) ChangeState((int)HoundState::CHASE);
		break;
	case HoundState::CHASE:
		CheckAttackTransition(m_attackRange, (int)HoundState::ATTACK_PRE, (int)HoundState::IDLE);
		break;
	case HoundState::IDLE:
		if (m_attackTarget && m_attackTarget->IsEnabled()) {
			if (m_distToPlayerSq > (m_attackRange * m_attackRange * 1.1f)) {
				if (!m_bHasHowled) { ChangeState((int)HoundState::HOWL); m_bHasHowled = true; }
				else ChangeState((int)HoundState::CHASE);
			}
			else if (m_attackCooldownTimer <= 0.0f) ChangeState((int)HoundState::ATTACK_PRE);
		}
		else
			UpdateAI_Wander(deltaTime, (int)HoundState::RUN, (int)HoundState::IDLE);
		break;
	case HoundState::RUN:
		if (m_attackTarget && m_attackTarget->IsEnabled()) ChangeState((int)HoundState::CHASE);
		break;
	}

	if (m_state == (int)HoundState::CHASE || m_state == (int)HoundState::RUN) ClampPositionToMapBounds();
}

void Hound::UpdateMovement(float deltaTime)
{
	if (!IsEnabled()) return;

	switch ((HoundState)m_state)
	{
	case HoundState::CHASE: 
		MoveTowardPlayer(deltaTime, m_runSpeed, (int)HoundState::RUN, (int)HoundState::IDLE); 
		break;
	case HoundState::RUN: 
		MoveTowardLocation(deltaTime, m_walkSpeed, (int)HoundState::RUN, (int)HoundState::IDLE); 
		break;
	case HoundState::IDLE: 
		m_animator->SetState((int)HoundState::IDLE, transform->GetDirection()); 
		break;
	}
}

void Hound::Damaged(int damage)
{
	Entity::Damaged(damage);
	if (!IsDead()) {
		ChangeState((int)HoundState::HIT);
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
	}
}

void Hound::OnAttackHit() 
{ 
	if (m_state == (int)HoundState::ATTACK) ProcessAttackHit(m_damage); 
}

void Hound::OnAttackEnd()
{
	if (m_state != (int)HoundState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	ChangeState((int)HoundState::CHASE);
}

void Hound::OnHitEnd()
{
	if (m_state != (int)HoundState::HIT) return;
	ChangeState((int)HoundState::IDLE);
}

void Hound::Die() { ChangeState((int)HoundState::DEATH); }

bool Hound::OnInteraction(GameObject* obj) { return Entity::OnInteraction(obj); }

void Hound::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());
	float rWander = m_wanderRadius;
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - rWander, screenCenter.Y - rWander, rWander * 2.0f, rWander * 2.0f), Gdiplus::Color(100, 200, 100, 255), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - m_aggroRadius, screenCenter.Y - m_aggroRadius, m_aggroRadius * 2.0f, m_aggroRadius * 2.0f), Gdiplus::Color(255, 255, 0), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);

	if (m_state == (int)HoundState::ATTACK && m_attackCollider) {
		UpdateAttackBoxByDirection(transform->GetDirection());
		RECT worldRect = m_attackCollider->GetWorldBoundingBox();
		Gdiplus::PointF topLeft = cameraManager->WorldToScreen((float)worldRect.left, (float)worldRect.top);
		Gdiplus::PointF bottomRight = cameraManager->WorldToScreen((float)worldRect.right, (float)worldRect.bottom);
		renderManager->AddDrawRectCommand(Gdiplus::RectF(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y), Gdiplus::Color(255, 0, 0), 2.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
	}
}

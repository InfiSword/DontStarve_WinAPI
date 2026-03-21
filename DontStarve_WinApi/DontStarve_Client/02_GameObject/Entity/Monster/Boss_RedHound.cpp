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
#include "Boss_RedHound.h"

Boss_RedHound::Boss_RedHound(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
                       const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
	, m_bHasHowled(false)
{
	m_hp = 300;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
	m_walkSpeed = 100.0f;
	m_runSpeed = 250.0f;
	m_attackRange = 100.0f;
	m_attackCooldown = 2.0f;
	m_attackHitFrame = 4;
	m_damage = 30;
	m_attackBoxWidth = 100;
	m_attackBoxHeight = 60;
	m_wanderRadius = 300.0f;
	m_aggroRadius = 400.0f;
	m_deaggroRadius = 600.0f;
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f;
}

Boss_RedHound::~Boss_RedHound() {}

void Boss_RedHound::Init()
{
	Monster::Init();
	SetupAggro(AggroType::ALWAYS, 0.0f, 0.0f);
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)BossRedHoundState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_bHasHowled = false;
	m_bCanChase = false; // 초기에는 추격 불가
	
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
			std::wstring prefix = L"RedHound_";
			std::wstring houndPrefix = prefix + L"hound_";
			float px = transform->GetPivotX();
			float py = transform->GetPivotY();

			m_animator->RegisterAnimation((int)BossRedHoundState::IDLE, DIR_DOWN, base + houndPrefix + L"idle_down.png", 0, 0, 7, 20, px, py, true, 0.02f);
			m_animator->RegisterAnimation((int)BossRedHoundState::IDLE, DIR_UP, base + houndPrefix + L"idle_up.png", 0, 0, 7, 20, px, py, true, 0.02f);
			std::wstring idleSidePath = base + houndPrefix + L"idle_side.png";
			m_animator->RegisterAnimation((int)BossRedHoundState::IDLE, DIR_LEFT, idleSidePath, 0, 0, 7, 20, px, py, true, 0.02f, false);
			m_animator->RegisterAnimation((int)BossRedHoundState::IDLE, DIR_RIGHT, idleSidePath, 0, 0, 7, 20, px, py, true, 0.02f);

			for (int state = (int)BossRedHoundState::RUN; state <= (int)BossRedHoundState::CHASE; ++state) {
				if (state != (int)BossRedHoundState::RUN && state != (int)BossRedHoundState::CHASE) continue;
				m_animator->RegisterAnimation(state, DIR_DOWN, base + houndPrefix + L"run_loop_down.png", 0, 0, 7, 16, px, py, true, 0.02f);
				m_animator->RegisterAnimation(state, DIR_UP, base + houndPrefix + L"run_loop_up.png", 0, 0, 7, 16, px, py, true, 0.02f);
				std::wstring walkSidePath = base + houndPrefix + L"run_loop_side.png";
				m_animator->RegisterAnimation(state, DIR_LEFT, walkSidePath, 0, 0, 7, 16, px, py, true, 0.02f, false);
				m_animator->RegisterAnimation(state, DIR_RIGHT, walkSidePath, 0, 0, 7, 16, px, py, true, 0.02f);
			}

			m_animator->RegisterAnimation((int)BossRedHoundState::ATTACK_PRE, DIR_DOWN, base + houndPrefix + L"atk_pre_down.png", 0, 0, 7, 29, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)BossRedHoundState::ATTACK_PRE, DIR_UP, base + houndPrefix + L"atk_pre_up.png", 0, 0, 7, 29, px, py, false, 0.03f);
			std::wstring atkPreSidePath = base + houndPrefix + L"atk_pre_side.png";
			m_animator->RegisterAnimation((int)BossRedHoundState::ATTACK_PRE, DIR_LEFT, atkPreSidePath, 0, 0, 7, 29, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)BossRedHoundState::ATTACK_PRE, DIR_RIGHT, atkPreSidePath, 0, 0, 7, 29, px, py, false, 0.03f);

			m_animator->RegisterAnimation((int)BossRedHoundState::ATTACK, DIR_DOWN, base + houndPrefix + L"atk_down.png", 0, 0, 7, 18, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)BossRedHoundState::ATTACK, DIR_UP, base + houndPrefix + L"atk_up.png", 0, 0, 7, 18, px, py, false, 0.03f);
			std::wstring atkSidePath = base + houndPrefix + L"atk_side.png";
			m_animator->RegisterAnimation((int)BossRedHoundState::ATTACK, DIR_LEFT, atkSidePath, 0, 0, 7, 18, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)BossRedHoundState::ATTACK, DIR_RIGHT, atkSidePath, 0, 0, 7, 18, px, py, false, 0.03f);

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossRedHoundState::ATTACK, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
					clip->AddEventFrame(17, L"attack_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"attack_hit") this->OnAttackHit();
						else if (eventName == L"attack_end") this->OnAttackEnd();
					});
				}
			}

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)BossRedHoundState::HIT, (Direction)dir, base + houndPrefix + L"hit_side.png", 0, 0, 7, 27, px, py, false, 0.03f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossRedHoundState::HIT, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(26, L"hit_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"hit_end") this->OnHitEnd();
					});
				}
			}

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)BossRedHoundState::DEATH, (Direction)dir, base + houndPrefix + L"death.png", 0, 0, 7, 52, px, py, false, 0.03f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossRedHoundState::DEATH, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(51, L"death_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"death_end") this->OnDeathEnd();
					});
				}
			}

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)BossRedHoundState::HOWL, (Direction)dir, base + houndPrefix + L"howl.png", 0, 0, 7, 47, px, py, false, 0.03f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossRedHoundState::HOWL, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(46, L"howl_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"howl_end") {
							m_bHasHowled = true;
							if (m_bCanChase) ChangeState((int)BossRedHoundState::CHASE);
							else ChangeState((int)BossRedHoundState::IDLE);
						}
					});
				}
			}
		}
		m_animator->SetState(m_state, this->transform->GetDirection());
	}
}

bool Boss_RedHound::OnInteraction(GameObject* obj) { return Entity::OnInteraction(obj); }

void Boss_RedHound::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	switch ((BossRedHoundState)m_state)
	{
	case BossRedHoundState::HOWL:
		break;
	case BossRedHoundState::ATTACK_PRE:
		if (m_animator->IsAnimationDone()) 
			ChangeState((int)BossRedHoundState::ATTACK);
		break;
	case BossRedHoundState::CHASE:
		if (!m_bCanChase) { ChangeState((int)BossRedHoundState::IDLE); break; }
		CheckAttackTransition(m_attackRange, (int)BossRedHoundState::ATTACK_PRE, (int)BossRedHoundState::IDLE);
		break;
	case BossRedHoundState::IDLE:
		if (m_attackTarget && m_attackTarget->IsEnabled()) {
			// 처음 발견 시 하울링
			if (!m_bHasHowled) ChangeState((int)BossRedHoundState::HOWL);
			else if (m_bCanChase) {
				if (m_distToPlayerSq > (m_attackRange * m_attackRange * 1.1f)) ChangeState((int)BossRedHoundState::CHASE);
				else if (m_attackCooldownTimer <= 0.0f) ChangeState((int)BossRedHoundState::ATTACK_PRE);
			}
		}
		else UpdateAI_Wander(deltaTime, (int)BossRedHoundState::RUN, (int)BossRedHoundState::IDLE);
		break;
	case BossRedHoundState::RUN:
		if (m_attackTarget && m_attackTarget->IsEnabled() && m_bCanChase) 
			ChangeState((int)BossRedHoundState::CHASE);
		break;
	}

	if (m_state == (int)BossRedHoundState::CHASE || m_state == (int)BossRedHoundState::RUN) ClampPositionToMapBounds();
}

void Boss_RedHound::UpdateMovement(float deltaTime)
{
	if (!IsEnabled()) return;

	switch ((BossRedHoundState)m_state)
	{
	case BossRedHoundState::CHASE: MoveTowardPlayer(deltaTime, m_runSpeed, (int)BossRedHoundState::RUN, (int)BossRedHoundState::IDLE); break;
	case BossRedHoundState::RUN: MoveTowardLocation(deltaTime, m_walkSpeed, (int)BossRedHoundState::RUN, (int)BossRedHoundState::IDLE); break;
	case BossRedHoundState::IDLE: m_animator->SetState((int)BossRedHoundState::IDLE, transform->GetDirection()); break;
	}
}

void Boss_RedHound::Damaged(int damage)
{
	Entity::Damaged(damage);
	if (!IsDead()) ChangeState((int)BossRedHoundState::HIT);
}

void Boss_RedHound::OnAttackHit()
{
	if (m_state == (int)BossRedHoundState::ATTACK)
		ProcessAttackHit(m_damage);
}

void Boss_RedHound::OnAttackEnd()
{
	if (m_state != (int)BossRedHoundState::ATTACK) return;
	ChangeState((int)BossRedHoundState::RUN);
}

void Boss_RedHound::OnHitEnd()
{
	if (m_state != (int)BossRedHoundState::HIT) return;
	ChangeState((int)BossRedHoundState::IDLE);
}

void Boss_RedHound::Die() { ChangeState((int)BossRedHoundState::DEATH); }

void Boss_RedHound::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());
	float rWander = m_wanderRadius;
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - rWander, screenCenter.Y - rWander, rWander * 2.0f, rWander * 2.0f), Gdiplus::Color(100, 200, 100, 255), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - m_aggroRadius, screenCenter.Y - m_aggroRadius, m_aggroRadius * 2.0f, m_aggroRadius * 2.0f), Gdiplus::Color(255, 255, 0), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);

	if (m_state == (int)BossRedHoundState::ATTACK && m_attackCollider) {
		UpdateAttackBoxByDirection(transform->GetDirection());
		RECT worldRect = m_attackCollider->GetWorldBoundingBox();
		Gdiplus::PointF topLeft = cameraManager->WorldToScreen((float)worldRect.left, (float)worldRect.top);
		Gdiplus::PointF bottomRight = cameraManager->WorldToScreen((float)worldRect.right, (float)worldRect.bottom);
		renderManager->AddDrawRectCommand(Gdiplus::RectF(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y), Gdiplus::Color(255, 0, 0), 2.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
	}
}

#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"
#include "../../Building/SpiderEgg.h"
#include "Spider.h"

Spider::Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
               const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
	, m_homeEgg(nullptr)
	, m_spawnRadius(200.0f)
	, m_bHasTaunted(false)
{
	m_type = GO_TYPE_MONSTER;
	m_walkSpeed = 60.0f;
	m_runSpeed = 150.0f;
	m_attackRange = 50.0f;
	m_attackCooldown = 1.2f;
	m_attackHitFrame = 45;
	m_damage = 15;
	m_attackBoxWidth = 60;
	m_attackBoxHeight = 40;
}

Spider::~Spider() {}

void Spider::Init()
{
	Monster::Init();
	SetupAggro(AggroType::ON_RANGE, 300.0f, 500.0f);
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)SpiderState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_bHasTaunted = false;

	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}

	if (!m_animator) m_animator = AddComponent<Animator>();
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		if (m_id == GOID_MONSTER_SPIDER || m_id == GOID_MONSTER_WARRIOR_SPIDER)
		{
			bool isWarrior = (m_id == GOID_MONSTER_WARRIOR_SPIDER);
			m_hp = isWarrior ? 200 : 80;
			m_maxHp = m_hp;
			m_walkSpeed = isWarrior ? 80.0f : 60.0f;
			m_runSpeed = isWarrior ? 180.0f : 150.0f;

			const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(m_id);
			if (objData) {
				std::wstring base = objData->baseDir + L"\\";
				std::wstring prefix = isWarrior ? L"Warrior_spider_" : L"Spider_spider_";
				
				for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) m_animator->RegisterAnimation((int)SpiderState::IDLE, (Direction)dir, base + prefix + L"idle_01.png", 0, 0, 1, 1, transform->GetPivotX(), transform->GetPivotY(), true, 0.05f);
				
				m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_DOWN, base + prefix + L"walk_loop_down.png", 0, 0, 7, 35, transform->GetPivotX(), transform->GetPivotY(), true, 0.02f);
				m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_UP, base + prefix + L"walk_loop_up.png", 0, 0, 7, 35, transform->GetPivotX(), transform->GetPivotY(), true, 0.02f);
				std::wstring walkSidePath = base + prefix + L"walk_loop_side.png";
				m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_LEFT, walkSidePath, 0, 0, 7, 35, transform->GetPivotX(), transform->GetPivotY(), true, 0.02f, false);
				m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_RIGHT, walkSidePath, 0, 0, 7, 35, transform->GetPivotX(), transform->GetPivotY(), true, 0.02f);

				m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_DOWN, base + prefix + L"atk_down.png", 0, 0, 7, 71, transform->GetPivotX(), transform->GetPivotY(), false, 0.02f);
				m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_UP, base + prefix + L"atk_up.png", 0, 0, 7, 71, transform->GetPivotX(), transform->GetPivotY(), false, 0.02f);
				std::wstring attackSidePath = base + prefix + L"atk_side.png";
				m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_LEFT, attackSidePath, 0, 0, 7, 71, transform->GetPivotX(), transform->GetPivotY(), false, 0.02f, false);
				m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_RIGHT, attackSidePath, 0, 0, 7, 71, transform->GetPivotX(), transform->GetPivotY(), false, 0.02f);

				for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
					AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderState::ATTACK, (Direction)dir);
					if (clip) {
						clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
						clip->AddEventFrame(70, L"attack_end");
						clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
							if (eventName == L"attack_hit") this->OnAttackHit();
							else if (eventName == L"attack_end") this->OnAttackEnd();
							});
					}
				}

				for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
					m_animator->RegisterAnimation((int)SpiderState::HIT, (Direction)dir, base + prefix + L"hit.png", 0, 0, 7, 34, transform->GetPivotX(), transform->GetPivotY(), false, 0.02f);
					AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderState::HIT, (Direction)dir);
					if (clip) {
						clip->AddEventFrame(33, L"hit_end");
						clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
							if (eventName == L"hit_end") this->OnHitEnd();
							});
					}
				}

				for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
					m_animator->RegisterAnimation((int)SpiderState::DEATH, (Direction)dir, base + prefix + L"death.png", 0, 0, 7, 56, transform->GetPivotX(), transform->GetPivotY(), false, 0.02f);
					AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderState::DEATH, (Direction)dir);
					if (clip) {
						clip->AddEventFrame(55, L"death_end");
						clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
							if (eventName == L"death_end") this->OnDeathEnd();
							});
					}
				}

				for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
					m_animator->RegisterAnimation((int)SpiderState::TAUNT, (Direction)dir, base + prefix + L"taunt.png", 0, 0, 7, 64, transform->GetPivotX(), transform->GetPivotY(), false, 0.01f);
					AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderState::TAUNT, (Direction)dir);
					if (clip) {
						clip->AddEventFrame(63, L"taunt_end");
						clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
							if (eventName == L"taunt_end") this->ChangeState((int)SpiderState::CHASE);
							});
					}
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

void Spider::SetHomeEgg(SpiderEgg* egg, float spawnRadius) { m_homeEgg = egg; m_spawnRadius = spawnRadius; }

void Spider::SetAggroTarget(GameObject* target)
{
	if (target && target->IsEnabled())
	{
		m_attackTarget = target;
		
		if (!m_bHasTaunted)  
		{
			ChangeState((int)SpiderState::TAUNT); 
			m_bHasTaunted = true; 
		}

		Transform* targetTr = target->GetComponent<Transform>();
		if (targetTr && transform) {
			float dx = targetTr->GetX() - transform->GetX();
			float dy = targetTr->GetY() - transform->GetY();
			Direction newDir = (std::abs(dx) > std::abs(dy)) ? (dx > 0.0f ? DIR_RIGHT : DIR_LEFT) : (dy > 0.0f ? DIR_DOWN : DIR_UP);
			transform->SetDirection(newDir);
		}
	}
}

void Spider::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	if (!m_attackTarget || !m_attackTarget->IsEnabled()) m_bHasTaunted = false;

	switch ((SpiderState)m_state)
	{
	case SpiderState::TAUNT:
		// Transition handled by callback
		break;
	case SpiderState::CHASE:
		if (!m_attackTarget || !m_attackTarget->IsEnabled()) { ChangeState((int)SpiderState::IDLE); m_idleTimer = 0.0f; }
		else CheckAttackTransition(m_attackRange, (int)SpiderState::ATTACK, (int)SpiderState::IDLE);
		break;
	case SpiderState::IDLE:
		if (m_attackTarget && m_attackTarget->IsEnabled() && m_bCanChase) {
			if (m_distToPlayerSq > (m_attackRange * m_attackRange * 1.1f)) {
				if (!m_bHasTaunted) { ChangeState((int)SpiderState::TAUNT); m_bHasTaunted = true; }
				else ChangeState((int)SpiderState::CHASE);
			}
			else if (m_attackCooldownTimer <= 0.0f) ChangeState((int)SpiderState::ATTACK);
		}
		else {
			if (m_idleTimer == 0.0f && m_homeEgg && m_homeEgg->IsEnabled()) {
				float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
				float dist = (rand() / (float)RAND_MAX) * m_spawnRadius;
				Transform* eggTr = m_homeEgg->GetComponent<Transform>();
				m_targetX = eggTr->GetX() + cosf(angle) * dist;
				m_targetY = eggTr->GetY() + sinf(angle) * dist;
			}
			UpdateAI_Wander(deltaTime, (int)SpiderState::WALK, (int)SpiderState::IDLE);
		}
		break;
	case SpiderState::WALK:
		if (m_attackTarget && m_attackTarget->IsEnabled()) {
			if (!m_bHasTaunted) { ChangeState((int)SpiderState::TAUNT); m_bHasTaunted = true; }
			else ChangeState((int)SpiderState::CHASE);
		}
		break;
	}

	if (m_state == (int)SpiderState::CHASE || m_state == (int)SpiderState::WALK) ClampPositionToMapBounds();
}

void Spider::UpdateMovement(float deltaTime)
{
	if (!IsEnabled()) return;

	switch ((SpiderState)m_state)
	{
	case SpiderState::CHASE: MoveTowardPlayer(deltaTime, m_runSpeed, (int)SpiderState::WALK, (int)SpiderState::IDLE); break;
	case SpiderState::WALK: MoveTowardLocation(deltaTime, m_walkSpeed, (int)SpiderState::WALK, (int)SpiderState::IDLE); break;
	case SpiderState::IDLE: m_animator->SetState((int)SpiderState::IDLE, transform->GetDirection()); break;
	}
}

void Spider::Damaged(int damage)
{
	Entity::Damaged(damage);
	if (!IsDead()) {
		ChangeState((int)SpiderState::HIT);
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
	}
}

void Spider::OnAttackHit() { if (m_state == (int)SpiderState::ATTACK) ProcessAttackHit(m_damage); }

void Spider::OnAttackEnd()
{
	if (m_state != (int)SpiderState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	ChangeState((int)SpiderState::CHASE);
}

void Spider::OnHitEnd()
{
	if (m_state != (int)SpiderState::HIT) return;
	ChangeState((int)SpiderState::IDLE);
}

void Spider::Die() { ChangeState((int)SpiderState::DEATH); }

bool Spider::OnInteraction(GameObject* obj) { return Entity::OnInteraction(obj); }

void Spider::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - m_spawnRadius, screenCenter.Y - m_spawnRadius, m_spawnRadius * 2.0f, m_spawnRadius * 2.0f), Gdiplus::Color(100, 200, 100, 255), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - m_aggroRadius, screenCenter.Y - m_aggroRadius, m_aggroRadius * 2.0f, m_aggroRadius * 2.0f), Gdiplus::Color(255, 255, 0), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - m_deaggroRadius, screenCenter.Y - m_deaggroRadius, m_deaggroRadius * 2.0f, m_deaggroRadius * 2.0f), Gdiplus::Color(255, 165, 0), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);

	if (m_state == (int)SpiderState::ATTACK && m_attackCollider) {
		UpdateAttackBoxByDirection(transform->GetDirection());
		RECT worldRect = m_attackCollider->GetWorldBoundingBox();
		Gdiplus::PointF topLeft = cameraManager->WorldToScreen((float)worldRect.left, (float)worldRect.top);
		Gdiplus::PointF bottomRight = cameraManager->WorldToScreen((float)worldRect.right, (float)worldRect.bottom);
		renderManager->AddDrawRectCommand(Gdiplus::RectF(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y), Gdiplus::Color(255, 0, 0), 2.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
	}
}

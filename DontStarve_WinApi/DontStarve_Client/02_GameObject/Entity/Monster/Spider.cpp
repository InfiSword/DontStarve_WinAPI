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
	
	// 공격 관련 설정
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

	// 거리 일정 범위 진입 시 추격하는 타입
	SetupAggro(AggroType::ON_RANGE, 300.0f, 500.0f);

	// 공격 박스 설정 (방향별 오프셋 자동 계산)
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)SpiderState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_bHasTaunted = false;

	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}

	OutputDebugStringW((L"Spider: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();

		if (m_id == GOID_MONSTER_SPIDER)
		{
			m_hp = 80;
			m_maxHp = m_hp;
			m_walkSpeed = 60.0f;
			m_runSpeed = 150.0f;

			const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_SPIDER);
			if (!objData) return;
			std::wstring base = objData->baseDir + L"\\";

			// IDLE
			std::wstring idlePath = base + L"Spider_spider_idle_01.png";
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderState::IDLE, (Direction)dir, idlePath,
					0, 0, 1, 1, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.05f);
			}

			// WALK
			m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_DOWN, base + L"Spider_spider_walk_loop_down.png",
				0, 0, 7, 35, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_UP, base + L"Spider_spider_walk_loop_up.png",
				0, 0, 7, 35, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			std::wstring walkSidePath = base + L"Spider_spider_walk_loop_side.png";
			m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_LEFT, walkSidePath,
				0, 0, 7, 35, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f, false);
			m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_RIGHT, walkSidePath,
				0, 0, 7, 35, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);

			// ATTACK
			m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_DOWN, base + L"Spider_spider_atk_down.png",
				0, 0, 7, 71, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_UP, base + L"Spider_spider_atk_up.png",
				0, 0, 7, 71, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			std::wstring attackSidePath = base + L"Spider_spider_atk_side.png";
			m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_LEFT, attackSidePath,
				0, 0, 7, 71, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f, false);
			m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_RIGHT, attackSidePath,
				0, 0, 7, 71, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderState::ATTACK, (Direction)dir);
				clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
				clip->AddEventFrame(70, L"attack_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"attack_hit") this->OnAttackHit();
					else if (eventName == L"attack_end") this->OnAttackEnd();
					});
			}

			// HIT / DEATH
			m_animator->RegisterAnimation((int)SpiderState::HIT, DIR_DOWN, base + L"Spider_spider_hit.png",
				0, 0, 7, 34, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			m_animator->RegisterAnimation((int)SpiderState::DEATH, DIR_DOWN, base + L"Spider_spider_death.png",
				0, 0, 7, 56, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);

			// TAUNT
			std::wstring tauntPath = base + L"Spider_spider_taunt.png";
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderState::TAUNT, (Direction)dir, tauntPath,
					0, 0, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.01f);
			}
		}
		else if (m_id == GOID_MONSTER_WARRIOR_SPIDER) {
			m_hp = 200;
			m_maxHp = m_hp;
			m_walkSpeed = 80.0f;
			m_runSpeed = 180.0f;

			const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_WARRIOR_SPIDER);
			if (!objData) return;
			std::wstring base = objData->baseDir + L"\\";

			// IDLE
			std::wstring idlePath = base + L"Warrior_spider_idle_01.png";
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderState::IDLE, (Direction)dir, idlePath,
					0, 0, 1, 1, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			}

			// WALK
			m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_DOWN, base + L"Warrior_spider_walk_loop_down.png",
				0, 0, 7, 35, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_UP, base + L"Warrior_spider_walk_loop_up.png",
				0, 0, 7, 35, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);
			std::wstring walkSidePath = base + L"Warrior_spider_walk_loop_side.png";
			m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_LEFT, walkSidePath,
				0, 0, 7, 35, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f, false);
			m_animator->RegisterAnimation((int)SpiderState::WALK, DIR_RIGHT, walkSidePath,
				0, 0, 7, 35, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.02f);

			// ATTACK
			m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_DOWN, base + L"Warrior_spider_atk_down.png",
				0, 0, 7, 71, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_UP, base + L"Warrior_spider_atk_up.png",
				0, 0, 7, 71, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			std::wstring attackSidePath = base + L"Warrior_spider_atk_side.png";
			m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_LEFT, attackSidePath,
				0, 0, 7, 71, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f, false);
			m_animator->RegisterAnimation((int)SpiderState::ATTACK, DIR_RIGHT, attackSidePath,
				0, 0, 7, 71, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderState::ATTACK, (Direction)dir);
				clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
				clip->AddEventFrame(70, L"attack_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"attack_hit") this->OnAttackHit();
					else if (eventName == L"attack_end") this->OnAttackEnd();
					});
			}

			// HIT / DEATH
			m_animator->RegisterAnimation((int)SpiderState::HIT, DIR_DOWN, base + L"Warrior_spider_hit.png",
				0, 0, 7, 34, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			m_animator->RegisterAnimation((int)SpiderState::DEATH, DIR_DOWN, base + L"Warrior_spider_death.png",
				0, 0, 7, 56, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);

			// TAUNT
			std::wstring tauntPath = base + L"Warrior_spider_taunt.png";
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)SpiderState::TAUNT, (Direction)dir, tauntPath,
					0, 0, 7, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.01f);
			}
		}

		m_animator->SetState(m_state, this->transform->GetDirection());
	}

	// 공격 전용 콜라이더 (ObjectManager에서 설정한 몸통 콜라이더는 그대로 사용)
	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		UpdateAttackBoxByDirection(DIR_DOWN);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Spider::SetHomeEgg(SpiderEgg* egg, float spawnRadius)
{
	m_homeEgg = egg;
	m_spawnRadius = spawnRadius;
}

void Spider::SetAggroTarget(GameObject* target)
{
	if (target && target->IsEnabled())
	{
		m_attackTarget = target;
		// SetAggroTarget 호출 시에도 도발 여부 체크
		if (!m_bHasTaunted)
		{
			ChangeState((int)SpiderState::TAUNT);
			m_bHasTaunted = true;
		}

		Transform* targetTr = target->GetComponent<Transform>();
		if (targetTr && transform) {
			float dx = targetTr->GetX() - transform->GetX();
			float dy = targetTr->GetY() - transform->GetY();
			Direction newDir;
			if (std::abs(dx) > std::abs(dy))
				newDir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
			else
				newDir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
			transform->SetDirection(newDir);
		}
	}
}

void Spider::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator)
		return;

	// 어그로 타겟이 없어지면 도발 가능 상태로 리셋
	if (!m_attackTarget || !m_attackTarget->IsEnabled())
	{
		m_bHasTaunted = false;
	}

	// 1. 공통 애니메이션 상태 처리 (HIT, DEATH, ATTACK, TAUNT 등)
	if (HandleCommonAnimationState((int)SpiderState::HIT, (int)SpiderState::DEATH, (int)SpiderState::ATTACK))
		return;

	if (m_state == (int)SpiderState::TAUNT)
	{
		if (m_animator->IsAnimationDone()) ChangeState((int)SpiderState::CHASE);
		return;
	}

	// 2. 메인 AI 로직 (RangeChase 타입)
	// 이미 도발을 했다면 tauntState를 -1로 넘겨서 즉시 CHASE로 가게 하거나, 
	// 직접 체크하여 TAUNT 상태로 진입하게 합니다.
	int nextTauntState = m_bHasTaunted ? -1 : (int)SpiderState::TAUNT;
	
	UpdateAI_RangeChase(deltaTime, 
		(int)SpiderState::IDLE, (int)SpiderState::WALK, (int)SpiderState::CHASE, (int)SpiderState::ATTACK, 
		nextTauntState);

	// TAUNT 상태로 진입했다면 플래그 설정
	if (m_state == (int)SpiderState::TAUNT)
	{
		m_bHasTaunted = true;
	}

	// 3. 거미 고유의 배회 로직 (거미집 중심)
	if (m_state == (int)SpiderState::IDLE && m_idleTimer == 0.0f) 
	{
		if (m_homeEgg && m_homeEgg->IsEnabled() && m_state == (int)SpiderState::WALK) {
			Transform* eggTr = m_homeEgg->GetComponent<Transform>();
			float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
			float dist = (rand() / (float)RAND_MAX) * m_spawnRadius;
			m_targetX = eggTr->GetX() + cosf(angle) * dist;
			m_targetY = eggTr->GetY() + sinf(angle) * dist;
		}
	}
}

void Spider::UpdateMovement(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	// 애니메이션 재생 중(공격, 히트 상태)는 이동하지 않음
	if (m_state == (int)SpiderState::ATTACK || m_state == (int)SpiderState::HIT || m_state == (int)SpiderState::DEATH || m_state == (int)SpiderState::TAUNT)
		return;

	if (m_state == (int)SpiderState::CHASE)
	{
		// 공격 사거리 내에 들어오면 즉시 멈춤 (AI 틱 대기 중 이동 방지)
		if (m_distToPlayerSq <= (m_attackRange * m_attackRange))
		{
			m_animator->SetState((int)SpiderState::IDLE, transform->GetDirection());
			return;
		}

		Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(newDir);
		m_animator->SetState((int)SpiderState::WALK, transform->GetDirection()); 

		float moveDist = m_runSpeed * deltaTime;
		transform->SetPosition(transform->GetX() + m_dirToPlayer.X * moveDist, transform->GetY() + m_dirToPlayer.Y * moveDist);
	}
	else if (m_state == (int)SpiderState::WALK)
	{
		float wdx = m_targetX - transform->GetX();
		float wdy = m_targetY - transform->GetY();
		float wdistSq = wdx * wdx + wdy * wdy;

		if (wdistSq < 4.0f) { 
			ChangeState((int)SpiderState::IDLE);
			return;
		}

		float wdist = sqrtf(wdistSq);
		Direction wDir = (std::abs(wdx) > std::abs(wdy)) ? (wdx > 0.0f ? DIR_RIGHT : DIR_LEFT) : (wdy > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(wDir);
		m_animator->SetState((int)SpiderState::WALK, transform->GetDirection());

		float moveStep = m_walkSpeed * deltaTime;
		transform->SetPosition(transform->GetX() + (wdx / wdist) * moveStep, transform->GetY() + (wdy / wdist) * moveStep);
	}
	else if (m_state == (int)SpiderState::IDLE)
	{
		m_animator->SetState((int)SpiderState::IDLE, transform->GetDirection());
	}

	// 맵 경계 위치 경계 체크
	if (m_state == (int)SpiderState::CHASE || m_state == (int)SpiderState::WALK) {
		ClampPositionToMapBounds();
	}
}

bool Spider::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Spider::Damaged(int damage)
{
	Entity::Damaged(damage);

	if (IsDead()) {
		return;  // Die()는 Entity::Damaged()에서 호출됨
	}

	ChangeState((int)SpiderState::HIT);

	if (!IsDead() && IsEnabled()) {
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
	}
}

void Spider::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());

	float rWander = m_spawnRadius;
	renderManager->AddDrawEllipseCommand(
		Gdiplus::RectF(screenCenter.X - rWander, screenCenter.Y - rWander, rWander * 2.0f, rWander * 2.0f),
		Gdiplus::Color(100, 200, 100, 255),
		1.0f, LAYER_DEBUG_OVERLAY, 9998.0f
	);

	float rAggro = m_aggroRadius;
	renderManager->AddDrawEllipseCommand(
		Gdiplus::RectF(screenCenter.X - rAggro, screenCenter.Y - rAggro, rAggro * 2.0f, rAggro * 2.0f),
		Gdiplus::Color(255, 255, 0),
		1.0f, LAYER_DEBUG_OVERLAY, 9998.0f
	);

	float rDeaggro = m_deaggroRadius;
	renderManager->AddDrawEllipseCommand(
		Gdiplus::RectF(screenCenter.X - rDeaggro, screenCenter.Y - rDeaggro, rDeaggro * 2.0f, rDeaggro * 2.0f),
		Gdiplus::Color(255, 165, 0),
		1.0f, LAYER_DEBUG_OVERLAY, 9998.0f
	);

	if (m_state == (int)SpiderState::ATTACK && m_attackCollider) {
		UpdateAttackBoxByDirection(transform->GetDirection());

		RECT worldRect = m_attackCollider->GetWorldBoundingBox();
		Gdiplus::PointF topLeft = cameraManager->WorldToScreen((float)worldRect.left, (float)worldRect.top);
		Gdiplus::PointF bottomRight = cameraManager->WorldToScreen((float)worldRect.right, (float)worldRect.bottom);

		renderManager->AddDrawRectCommand(
			Gdiplus::RectF(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y),
			Gdiplus::Color(255, 0, 0),
			2.0f, LAYER_DEBUG_OVERLAY, 9999.0f
		);
	}
}

void Spider::OnAttackHit()
{
	if (m_state != (int)SpiderState::ATTACK) return;
	
	// Monster 기본 클래스의 공격 처리 사용
	ProcessAttackHit(m_damage);
}

void Spider::OnAttackEnd()
{
	if (m_state != (int)SpiderState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	ChangeState((int)SpiderState::CHASE);
}

void Spider::Die()
{
    ChangeState((int)SpiderState::DEATH);
}

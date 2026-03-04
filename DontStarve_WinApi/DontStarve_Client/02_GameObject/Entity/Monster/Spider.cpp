#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/SceneManager/SceneManager.h"
#include "../../../01_Manager/SceneManager/GameScene.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"
#include "../../Building/SpiderEgg.h"
#include "Spider.h"

const float Spider::ATTACK_RANGE = 50.0f;
const float Spider::ATTACK_COOLDOWN = 1.2f;

static const int SPIDER_ATTACK_HIT_FRAME = 45;  // 8프레임 중 4프레임에 데미지
static const int SPIDER_ATTACK_BOX_W = 60, SPIDER_ATTACK_BOX_H = 40;
static const int SPIDER_ATTACK_DOWN[] = { -30,   0, SPIDER_ATTACK_BOX_W, SPIDER_ATTACK_BOX_H };
static const int SPIDER_ATTACK_UP[] = { -30, -40, SPIDER_ATTACK_BOX_W, SPIDER_ATTACK_BOX_H };
static const int SPIDER_ATTACK_LEFT[] = { -60, -20, SPIDER_ATTACK_BOX_W, SPIDER_ATTACK_BOX_H };
static const int SPIDER_ATTACK_RIGHT[] = { 0, -20, SPIDER_ATTACK_BOX_W, SPIDER_ATTACK_BOX_H };

Spider::Spider(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Entity(id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName, true, true)
	, m_homeEgg(nullptr)
	, m_spawnRadius(200.0f)
	, m_aggroRadius(300.0f)
	, m_deaggroRadius(400.0f)
	, m_walkSpeed(60.0f)
	, m_runSpeed(150.0f)
	, m_attackCooldownTimer(0.0f)
	, m_idleTimer(0.0f)
	, m_idleDuration(2.0f)
	, m_targetX(x)
	, m_targetY(y)
	, m_aggroTarget(nullptr)
	, m_attackCollider(nullptr)
{
	m_type = GO_TYPE_MONSTER;
}

Spider::~Spider() {}

void Spider::Init()
{
	Entity::Init();

	m_state = (int)SpiderState::IDLE;
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_attackCooldownTimer = 0.0f;

	// Transform 컴포넌트 확인
	if (!this->transform) {
		this->transform = GetComponent<Transform>();
		if (!this->transform) {
			OutputDebugStringW(L"Spider: Transform component not found!\n");
			return;
		}
	}

	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}

	OutputDebugStringW((L"Spider: Init 성공 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

	// Animator 생성 및 애니메이션 등록
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
				if (clip) {
					clip->AddEventFrame(SPIDER_ATTACK_HIT_FRAME, L"attack_hit");
					clip->AddEventFrame(70, L"attack_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"attack_hit") this->OnAttackHit();
						else if (eventName == L"attack_end") this->OnAttackEnd();
						});
				}
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
				if (clip) {
					clip->AddEventFrame(SPIDER_ATTACK_HIT_FRAME, L"attack_hit"); // 전사 거미는 4번 프레임
					clip->AddEventFrame(70, L"attack_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"attack_hit") this->OnAttackHit();
						else if (eventName == L"attack_end") this->OnAttackEnd();
						});
				}
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

	// 공격 판정용 콜라이더
	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		m_attackCollider->SetObjectCollider(SPIDER_ATTACK_DOWN[0], SPIDER_ATTACK_DOWN[1], SPIDER_ATTACK_DOWN[2], SPIDER_ATTACK_DOWN[3]);
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
		m_aggroTarget = target;
		m_state = (int)SpiderState::TAUNT;

		// 방향 설정 (타겟 쪽으로)
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

void Spider::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (!IsEnabled() || !transform || !m_animator)
		return;

	// 공격 쿨타임 감소
	if (m_attackCooldownTimer > 0.0f) {
		m_attackCooldownTimer -= deltaTime;
	}

	// 애니메이션 기반 상태 처리 (HIT, TAUNT, DEATH, ATTACK)
	if (m_state == (int)SpiderState::HIT || m_state == (int)SpiderState::TAUNT ||
		m_state == (int)SpiderState::DEATH || m_state == (int)SpiderState::ATTACK)
	{
		m_animator->SetState(m_state, transform->GetDirection());

		if (m_animator->IsAnimationDone())
		{
			if (m_state == (int)SpiderState::DEATH) {
				ObjectManager::GetInstance()->RemoveGameObject(this);
				return;
			}

			if (m_state == (int)SpiderState::ATTACK) {
				OnAttackEnd(); // 내부에서 m_state를 CHASE로 변경함
			}
			else {
				// HIT나 TAUNT가 끝난 경우 타겟이 있으면 추격, 없으면 대기
				if (m_aggroTarget && m_aggroTarget->IsEnabled())
					m_state = (int)SpiderState::CHASE;
				else
					m_state = (int)SpiderState::IDLE;
			}
		}
		return; // 위 상태들일 때는 아래 이동/감지 로직을 수행하지 않음
	}

	// 3. 타겟(플레이어) 정보 미리 계산
	GameObject* player = ObjectManager::GetInstance()->GetPlayer();
	float distToPlayer = 99999.0f;
	float dx = 0.0f, dy = 0.0f;

	if (player && player->IsEnabled()) {
		dx = player->GetComponent<Transform>()->GetX() - transform->GetX();
		dy = player->GetComponent<Transform>()->GetY() - transform->GetY();
		distToPlayer = sqrtf(dx * dx + dy * dy);
	}

	// 메인 상태 머신 (CHASE, IDLE, WALK)
	if (m_state == (int)SpiderState::CHASE)
	{
		// 어그로 해제 조건 체크 (멀어지면 다시 IDLE로)
		if (!m_aggroTarget || !m_aggroTarget->IsEnabled() || distToPlayer > m_deaggroRadius) {
			m_aggroTarget = nullptr;
			m_state = (int)SpiderState::IDLE;
			m_idleTimer = 0.0f;
			return;
		}

		// 방향 설정
		Direction newDir = (std::abs(dx) > std::abs(dy)) ? (dx > 0.0f ? DIR_RIGHT : DIR_LEFT) : (dy > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(newDir);

		// 공격 가능 여부 체크
		if (distToPlayer <= ATTACK_RANGE) {
			if (m_attackCooldownTimer <= 0.0f) {
				m_state = (int)SpiderState::ATTACK;
				m_animator->SetState((int)SpiderState::ATTACK, transform->GetDirection());
				m_attackCooldownTimer = ATTACK_COOLDOWN;
				return;
			}
			else {
				// 공격 쿨타임 중에는 Pig처럼 IDLE 애니메이션 재생하며 대기
				m_animator->SetState((int)SpiderState::IDLE, transform->GetDirection());
				return;
			}
		}

		// 추격 이동
		m_animator->SetState((int)SpiderState::WALK, transform->GetDirection());
		float moveDist = m_runSpeed * deltaTime;
		float step = (std::min)(moveDist, distToPlayer);
		transform->SetPosition(transform->GetX() + (dx / distToPlayer) * step, transform->GetY() + (dy / distToPlayer) * step);
	}

	else if (m_state == (int)SpiderState::IDLE)
	{
		// IDLE일 때만 플레이어를 새로 감지함 (TAUNT로 전이)
		if (distToPlayer <= m_aggroRadius) {
			m_aggroTarget = player;
			m_state = (int)SpiderState::TAUNT;
			return;
		}

		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration) {
			// 배회 목표 설정 로직 (기존과 동일)
			float centerX = transform->GetX();
			float centerY = transform->GetY();
			if (m_homeEgg && m_homeEgg->IsEnabled()) {
				centerX = m_homeEgg->GetComponent<Transform>()->GetX();
				centerY = m_homeEgg->GetComponent<Transform>()->GetY();
			}
			float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
			float dist = (rand() / (float)RAND_MAX) * m_spawnRadius;
			m_targetX = centerX + cosf(angle) * dist;
			m_targetY = centerY + sinf(angle) * dist;

			m_state = (int)SpiderState::WALK;
			m_idleTimer = 0.0f;
		}
		else {
			m_animator->SetState((int)SpiderState::IDLE, transform->GetDirection());
		}
	}

	else if (m_state == (int)SpiderState::WALK)
	{
		// 배회 중에도 플레이어를 감지하면 TAUNT로 전환
		if (distToPlayer <= m_aggroRadius) {
			m_aggroTarget = player;
			m_state = (int)SpiderState::TAUNT;
			return;
		}

		float wdx = m_targetX - transform->GetX();
		float wdy = m_targetY - transform->GetY();
		float wdist = sqrtf(wdx * wdx + wdy * wdy);

		Direction wDir = (std::abs(wdx) > std::abs(wdy)) ? (wdx > 0.0f ? DIR_RIGHT : DIR_LEFT) : (wdy > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(wDir);
		m_animator->SetState((int)SpiderState::WALK, transform->GetDirection());

		float moveStep = m_walkSpeed * deltaTime;
		if (wdist < 2.0f || wdist <= moveStep) {
			transform->SetPosition(m_targetX, m_targetY);
			m_state = (int)SpiderState::IDLE;
		}
		else {
			transform->SetPosition(transform->GetX() + (wdx / wdist) * moveStep, transform->GetY() + (wdy / wdist) * moveStep);
		}
	}
}

bool Spider::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Spider::Damaged(int damage)
{
	m_hp -= damage;

	m_state = (int)SpiderState::HIT;

	if (m_hp <= 0) {
		m_hp = 0;
		m_state = (int)SpiderState::DEATH;
		m_isDead = true;
	}

	if (!IsDead() && IsEnabled()) {
		m_aggroTarget = ObjectManager::GetInstance()->GetPlayer();
		m_attackCooldownTimer = 0.0f;
	}
}

void Spider::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());

	// 행동 반경 원 (연두색) - 거미집 기준이면 위치가 다를 수 있지만 일단 현재 위치 기준
	float rWander = m_spawnRadius;
	renderManager->AddDrawEllipseCommand(
		Gdiplus::RectF(screenCenter.X - rWander, screenCenter.Y - rWander, rWander * 2.0f, rWander * 2.0f),
		Gdiplus::Color(100, 200, 100, 255),
		1.0f, LAYER_DEBUG_OVERLAY, 9998.0f
	);

	// 어그로 반경 (노란색)
	float rAggro = m_aggroRadius;
	renderManager->AddDrawEllipseCommand(
		Gdiplus::RectF(screenCenter.X - rAggro, screenCenter.Y - rAggro, rAggro * 2.0f, rAggro * 2.0f),
		Gdiplus::Color(255, 255, 0),
		1.0f, LAYER_DEBUG_OVERLAY, 9998.0f
	);

	// 어그로 해제 반경 (주황색)
	float rDeaggro = m_deaggroRadius;
	renderManager->AddDrawEllipseCommand(
		Gdiplus::RectF(screenCenter.X - rDeaggro, screenCenter.Y - rDeaggro, rDeaggro * 2.0f, rDeaggro * 2.0f),
		Gdiplus::Color(255, 165, 0),
		1.0f, LAYER_DEBUG_OVERLAY, 9998.0f
	);

	// 공격 반경 (빨간색 사각형) - 공격 상태일 때만 표시
	if (m_state == (int)SpiderState::ATTACK && m_attackCollider) {
		// 현재 방향에 맞게 콜라이더 위치 업데이트 (Update에서 이미 수행하지만 렌더링 시점에도 확실히 함)
		Direction dir = transform->GetDirection();
		if (dir == DIR_DOWN) m_attackCollider->SetObjectCollider(SPIDER_ATTACK_DOWN[0], SPIDER_ATTACK_DOWN[1], SPIDER_ATTACK_DOWN[2], SPIDER_ATTACK_DOWN[3]);
		else if (dir == DIR_UP) m_attackCollider->SetObjectCollider(SPIDER_ATTACK_UP[0], SPIDER_ATTACK_UP[1], SPIDER_ATTACK_UP[2], SPIDER_ATTACK_UP[3]);
		else if (dir == DIR_LEFT) m_attackCollider->SetObjectCollider(SPIDER_ATTACK_LEFT[0], SPIDER_ATTACK_LEFT[1], SPIDER_ATTACK_LEFT[2], SPIDER_ATTACK_LEFT[3]);
		else m_attackCollider->SetObjectCollider(SPIDER_ATTACK_RIGHT[0], SPIDER_ATTACK_RIGHT[1], SPIDER_ATTACK_RIGHT[2], SPIDER_ATTACK_RIGHT[3]);

		RECT worldRect = m_attackCollider->GetWorldBoundingBox();
		Gdiplus::PointF topLeft = cameraManager->WorldToScreen((float)worldRect.left, (float)worldRect.top);
		Gdiplus::PointF bottomRight = cameraManager->WorldToScreen((float)worldRect.right, (float)worldRect.bottom);

		renderManager->AddDrawCommand(
			Gdiplus::RectF(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y),
			Gdiplus::Color(255, 0, 0),
			2.0f, LAYER_DEBUG_OVERLAY, 9999.0f
		);
	}
}

void Spider::OnAttackHit()
{
	if (m_state != (int)SpiderState::ATTACK || !m_attackCollider || !transform) return;

	Direction dir = transform->GetDirection();
	if (dir == DIR_DOWN) m_attackCollider->SetObjectCollider(SPIDER_ATTACK_DOWN[0], SPIDER_ATTACK_DOWN[1], SPIDER_ATTACK_DOWN[2], SPIDER_ATTACK_DOWN[3]);
	else if (dir == DIR_UP) m_attackCollider->SetObjectCollider(SPIDER_ATTACK_UP[0], SPIDER_ATTACK_UP[1], SPIDER_ATTACK_UP[2], SPIDER_ATTACK_UP[3]);
	else if (dir == DIR_LEFT) m_attackCollider->SetObjectCollider(SPIDER_ATTACK_LEFT[0], SPIDER_ATTACK_LEFT[1], SPIDER_ATTACK_LEFT[2], SPIDER_ATTACK_LEFT[3]);
	else m_attackCollider->SetObjectCollider(SPIDER_ATTACK_RIGHT[0], SPIDER_ATTACK_RIGHT[1], SPIDER_ATTACK_RIGHT[2], SPIDER_ATTACK_RIGHT[3]);

	m_attackCollider->SetColliderEnabled(true);

	if (m_aggroTarget && m_aggroTarget->IsEnabled()) {
		Transform* pt = m_aggroTarget->GetComponent<Transform>();
		if (pt && m_attackCollider->ContainsPoint(pt->GetX(), pt->GetY())) {
			Entity* playerEntity = dynamic_cast<Entity*>(m_aggroTarget);
			if (playerEntity) playerEntity->Damaged(8);
		}
	}

	m_attackCollider->SetColliderEnabled(false);
}

void Spider::OnAttackEnd()
{
	if (m_state != (int)SpiderState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	m_state = (int)SpiderState::CHASE;
}

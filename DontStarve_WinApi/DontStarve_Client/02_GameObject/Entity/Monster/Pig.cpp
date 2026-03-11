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
	
	// 공격 관련 설정
	m_attackRange = 70.0f;
	m_attackCooldown = 1.5f;
	m_attackHitFrame = 28;
	m_damage = 18;
	m_attackBoxWidth = 70;
	m_attackBoxHeight = 50;
}

Pig::~Pig() {}

void Pig::Init()
{
	Monster::Init();

	// 어그로 처음 공격받으면 이후부터 범위 체크를 시작하는 타입
	// 처음에는 평화로운데 한 번 공격받으면 적대적으로 경계 상태가 됨
	SetupAggro(AggroType::ON_HIT_THEN_RANGE, 250.0f, 600.0f);

	// 공격 박스 설정 (방향별 오프셋 자동 계산)
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)PigState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_attackCooldownTimer = 0.0f;
	
	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}
	
	OutputDebugStringW((L"Pig: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

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
				0, 0, 4, 66, this->transform->GetPivotY(), this->transform->GetPivotY(), false, 0.02f);
			std::wstring atkSidePath = baseAttack + L"side_pigman_atk_side.png";
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_LEFT, atkSidePath,
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f, false);
			m_animator->RegisterAnimation((int)PigState::ATTACK, DIR_RIGHT, atkSidePath,
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);

			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
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

			m_animator->RegisterAnimation((int)PigState::HIT, DIR_DOWN, base + L"Hit\\Hit_pigman_hit.png",
				232, 245, 4, 29, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			
			// Death 애니메이션을 모든 방향에 등록
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)PigState::DEATH, (Direction)dir, base + L"Death\\Death_pigman_death.png",
					227, 243, 4, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.02f);
			}
		}

		m_animator->SetState(m_state, this->transform->GetDirection());
	}

	// 怨듦꺽 ?먯젙??肄쒕씪?대뜑 (ObjectManager?먯꽌 ?ㅼ젙??紐명넻 肄쒕씪?대뜑??洹몃?濣??ъ슜)
	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		UpdateAttackBoxByDirection(DIR_DOWN);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Pig::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator)
		return;

	// --- 1. 애니메이션 기반 상태 처리 (HIT, DEATH, ATTACK) ---
	if (m_state == (int)PigState::HIT || m_state == (int)PigState::DEATH || m_state == (int)PigState::ATTACK)
	{
		if (m_animator->IsAnimationDone())
		{
			if (m_state == (int)PigState::DEATH) {
				ObjectManager::GetInstance()->RemoveGameObject(this);
				return;
			}

			if (m_state == (int)PigState::ATTACK) {
				OnAttackEnd();
			}
			else if (m_state == (int)PigState::HIT) {
				if (m_attackTarget && m_attackTarget->IsEnabled())
					ChangeState((int)PigState::CHASE);
				else
					ChangeState((int)PigState::IDLE);
			}
		}
		else if (m_state == (int)PigState::DEATH) {
			// DEATH 상태에서는 애니메이션이 끝날 때까지 대기
			// 애니메이터 상태를 다시 한번 확실히 설정
			if (m_animator) {
				m_animator->SetState((int)PigState::DEATH, transform->GetDirection());
			}
		}
		return;
	}

	// --- 2. 메인 상태 머신 (상태 결정만 수행) ---
	if (m_state == (int)PigState::CHASE)
	{
		if (!m_attackTarget || !m_attackTarget->IsEnabled()) {
			m_attackTarget = nullptr;
			ChangeState((int)PigState::IDLE);
			m_idleTimer = 0.0f;
			return;
		}

		// 공격 사거리 체크 (제곱 거리 사용)
		if (m_distToPlayerSq <= (m_attackRange * m_attackRange)) {
			if (m_attackCooldownTimer <= 0.0f) {
				ChangeState((int)PigState::ATTACK);
				m_attackCooldownTimer = m_attackCooldown;
			}
			else {
				ChangeState((int)PigState::IDLE);
			}
		}
	}
	else if (m_state == (int)PigState::IDLE)
	{
		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration) {
			float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
			float dist = (rand() / (float)RAND_MAX) * m_wanderRadius;
			m_targetX = transform->GetX() + cosf(angle) * dist;
			m_targetY = transform->GetY() + sinf(angle) * dist;

			// 목표 위치가 맵 경계를 벗어나지 않도록 제한
			const float BUFFER = 50.0f;  // 경계에서 50픽셀 떨어지게
			const float mapMaxX = static_cast<float>(MAP_WIDTH * TILE_SIZE) - BUFFER;
			const float mapMaxY = static_cast<float>(MAP_HEIGHT * TILE_SIZE) - BUFFER;
			const float mapMinX = BUFFER;
			const float mapMinY = BUFFER;

			if (m_targetX < mapMinX) m_targetX = mapMinX;
			if (m_targetX > mapMaxX) m_targetX = mapMaxX;
			if (m_targetY < mapMinY) m_targetY = mapMinY;
			if (m_targetY > mapMaxY) m_targetY = mapMaxY;

			ChangeState((int)PigState::WALK);
			m_idleTimer = 0.0f;
		}
	}
}

void Pig::UpdateMovement(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	// 애니메이션 재생 중(공격, 히트 상태)는 이동하지 않음
	if (m_state == (int)PigState::ATTACK || m_state == (int)PigState::HIT || m_state == (int)PigState::DEATH)
		return;

	if (m_state == (int)PigState::CHASE)
	{
		Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(newDir);
		m_animator->SetState((int)PigState::RUN, transform->GetDirection());

		float moveDist = m_runSpeed * deltaTime;
		transform->SetPosition(transform->GetX() + m_dirToPlayer.X * moveDist, transform->GetY() + m_dirToPlayer.Y * moveDist);
	}
	else if (m_state == (int)PigState::WALK)
	{
		float wdx = m_targetX - transform->GetX();
		float wdy = m_targetY - transform->GetY();
		float wdistSq = wdx * wdx + wdy * wdy;

		if (wdistSq < 4.0f) { // 2.0f * 2.0f
			ChangeState((int)PigState::IDLE);
			return;
		}

		float wdist = sqrtf(wdistSq);
		Direction wDir = (std::abs(wdx) > std::abs(wdy)) ? (wdx > 0.0f ? DIR_RIGHT : DIR_LEFT) : (wdy > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(wDir);
		m_animator->SetState((int)PigState::WALK, transform->GetDirection());

		float moveStep = m_walkSpeed * deltaTime;
		transform->SetPosition(transform->GetX() + (wdx / wdist) * moveStep, transform->GetY() + (wdy / wdist) * moveStep);
	}
	else if (m_state == (int)PigState::IDLE)
	{
		m_animator->SetState((int)PigState::IDLE, transform->GetDirection());
	}

	// 맵 경계 위치 경계 체크 (CHASE, WALK 상태에서 맵 밖으로 나가지 않도록)
	if (m_state == (int)PigState::CHASE || m_state == (int)PigState::WALK) {
		ClampPositionToMapBounds();
	}
}

bool Pig::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Pig::Damaged(int damage)
{
	Entity::Damaged(damage);
	ChangeState((int)PigState::HIT);

	if (!IsDead() && IsEnabled()) {
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
		m_attackCooldownTimer = 0.0f;
	}
}

void Pig::OnAttackHit()
{
	if (m_state != (int)PigState::ATTACK) return;
	
	// Monster 기본 클래스의 공격 처리 사용
	ProcessAttackHit(m_damage);
}

void Pig::OnAttackEnd()
{
	if (m_state != (int)PigState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	ChangeState((int)PigState::CHASE);
}

void Pig::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());

	float rWander = m_wanderRadius;
	renderManager->AddDrawEllipseCommand(
		Gdiplus::RectF(screenCenter.X - rWander, screenCenter.Y - rWander, rWander * 2.0f, rWander * 2.0f),
		Gdiplus::Color(100, 200, 100, 255),
		1.0f, LAYER_DEBUG_OVERLAY, 9998.0f
	);

	if (m_state == (int)PigState::ATTACK && m_attackCollider) {
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

void Pig::Die()
{
	// Change to death state and set animator
	ChangeState((int)PigState::DEATH);
	
	// 명시적으로 애니메이터 상태 설정
	if (m_animator && transform) {
		m_animator->SetState((int)PigState::DEATH, transform->GetDirection());
	}
}


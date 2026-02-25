#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/SceneManager/SceneManager.h"
#include "../../../01_Manager/SceneManager/GameScene.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../Player/Player.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"
#include "Pig.h"
#include <cmath>
#include <cstdlib>
#include "../../../01_Manager/ObjectManager/ObjectManager.h"

const float Pig::ATTACK_RANGE = 70.0f;
// 공격 콜라이더: 방향별 로컬 박스 (offsetX, offsetY, width, height) — 플레이어와 동일한 전방 기준
static const int PIG_ATTACK_HIT_FRAME = 15;  // 66프레임 중 이 프레임에 데미지
static const int PIG_ATTACK_BOX_W = 70, PIG_ATTACK_BOX_H = 50;
static const int PIG_ATTACK_DOWN[]  = { -35,    0, PIG_ATTACK_BOX_W, PIG_ATTACK_BOX_H };
static const int PIG_ATTACK_UP[]    = { -35,  -50, PIG_ATTACK_BOX_W, PIG_ATTACK_BOX_H };
static const int PIG_ATTACK_LEFT[]  = { -70,  -25, PIG_ATTACK_BOX_W, PIG_ATTACK_BOX_H };
static const int PIG_ATTACK_RIGHT[] = {   0,  -25, PIG_ATTACK_BOX_W, PIG_ATTACK_BOX_H };

Pig::Pig(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Monster(id, x, y, pivotX, pivotY, baseDir, imageName)
	, m_actionRadius(400.0f)
	, m_targetX(x)
	, m_targetY(y)
	, m_idleTimer(0.0f)
	, m_idleDuration(2.0f)
	, m_walkSpeed(80.0f)
	, m_runSpeed(200.0f)
	, m_aggroTarget(nullptr)
	, m_attackCollider(nullptr)
{
	m_hp = 100;
	maxHp = m_hp;
}

Pig::~Pig() {}

void Pig::Init()
{
	Monster::Init();

	// body 콜라이더(클릭/상호작용용)는 ObjectManager::CreateGameObject에서 Init() 호출 직후
	// data->hasCollider일 때 맵/리소스 데이터(colliderOffsetX,Y, colliderWidth,Height)로 자동 추가됨.

	m_state = MONSTER_IDLE;
	m_idleTimer = 0.0f;
	// 첫 IDLE 지속 시간 2~5초 랜덤
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_targetX = transform ? transform->GetX() : 0.0f;
	m_targetY = transform ? transform->GetY() : 0.0f;
	
	// Transform 컴포넌트 확인
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

	// Animator 생성 및 애니메이션 등록 (AnimationDefinition 클래스 제거)
	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (m_animator) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_PIG);
		if (m_id == GOID_MONSTER_PIG && objData) {
			std::wstring base = objData->baseDir + L"\\";
			
			// IDLE
			std::wstring baseAction = base + L"Action\\";
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_DOWN, baseAction + L"pig_pigman_idle_loop_down.png",
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_UP, baseAction + L"pig_pigman_idle_loop_up.png",
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			std::wstring idleSidePath = baseAction + L"pig_pigman_idle_loop_side.png";
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_LEFT, idleSidePath,
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f, false);
			m_animator->RegisterAnimation((int)MONSTER_IDLE, DIR_RIGHT, idleSidePath,
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

			// WALK
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_DOWN, baseAction + L"pig_pigman_walk_loop_down.png",
				0, 0, 4, 41, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_UP, baseAction + L"pig_pigman_walk_loop_up.png",
				0, 0, 4, 41, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			std::wstring walkSidePath = baseAction + L"pig_pigman_walk_loop_side.png";
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_LEFT, walkSidePath,
				0, 0, 4, 41, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f, false);
			m_animator->RegisterAnimation((int)MONSTER_WALK, DIR_RIGHT, walkSidePath,
				0, 0, 4, 41, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

			// RUN
			m_animator->RegisterAnimation((int)MONSTER_RUN, DIR_DOWN, baseAction + L"pig_pigman_run_loop_down.png",
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_RUN, DIR_UP, baseAction + L"pig_pigman_run_loop_up.png",
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);
            std::wstring runSidePath = baseAction + L"pig_pigman_run_loop_side.png";
			m_animator->RegisterAnimation((int)MONSTER_RUN, DIR_LEFT, runSidePath,
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f, false);
			m_animator->RegisterAnimation((int)MONSTER_RUN, DIR_RIGHT, runSidePath,
				0, 0, 4, 33, this->transform->GetPivotX(), this->transform->GetPivotY(), true, 0.03f);

			// ATTACK
			std::wstring baseAttack = base + L"Attack\\";
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_DOWN, baseAttack + L"down_pigman_atk_down.png",
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_UP, baseAttack + L"up_pigman_atk_up.png",
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			std::wstring atkSidePath = baseAttack + L"side_pigman_atk_side.png";
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_LEFT, atkSidePath,
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f, false);
			m_animator->RegisterAnimation((int)MONSTER_ATTACK, DIR_RIGHT, atkSidePath,
				0, 0, 4, 66, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);

			// HIT / DEATH
			m_animator->RegisterAnimation((int)MONSTER_HIT, DIR_DOWN, base + L"Hit\\Hit_pigman_hit.png",
				0, 0, 4, 29, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
			m_animator->RegisterAnimation((int)MONSTER_DEATH, DIR_DOWN, base + L"Death\\Death_pigman_death.png",
				0, 0, 4, 64, this->transform->GetPivotX(), this->transform->GetPivotY(), false, 0.03f);
		}

		m_animator->SetState((int)m_state, this->transform->GetDirection());
	}

	// 공격 판정용 콜라이더 (MONSTER_ATTACK 시 방향별 전방, hit 프레임에만 활성)
	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		m_attackCollider->SetObjectCollider(PIG_ATTACK_DOWN[0], PIG_ATTACK_DOWN[1], PIG_ATTACK_DOWN[2], PIG_ATTACK_DOWN[3]);
		m_attackCollider->SetColliderEnabled(false);
		m_attackCollider->SetInteractionCollider(false);
	}

	// Animator 초기화 확인
	Animator* animator = GetComponent<Animator>();
	if (animator) {
		// Animator 초기화 확인
		const SpriteSheet* spriteSheet = animator->GetSpriteSheet();
		if (spriteSheet) {
			OutputDebugStringW((L"Pig: Animator 초기화 성공 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 로드됨\n").c_str());
		} else {
			OutputDebugStringW((L"Pig: Animator 초기화 실패 - ID: " + std::to_wstring(m_id) + L", SpriteSheet 없음\n").c_str());
		}
	} else {
		OutputDebugStringW((L"Pig: Animator 생성 실패 - ID: " + std::to_wstring(m_id) + L"\n").c_str());
	}
}

void Pig::Update(float deltaTime)
{
	Monster::Update(deltaTime);

	if (!IsEnabled() || !transform || !m_animator)
		return;

	// HIT 상태: 피격 애니메이션 재생 후 종료되면 추격 또는 대기 상태로 전환
	if (m_state == MONSTER_HIT)
	{
		m_animator->SetState((int)MONSTER_HIT, transform->GetDirection());
		if (m_animator->IsAnimationDone())
		{
			// 피격 후 플레이어를 인식했다면 추격, 아니면 다시 IDLE
			if (m_aggroTarget && m_aggroTarget->IsEnabled())
			{
				m_state = MONSTER_CHASE;
			}
			else
			{
				m_state = MONSTER_IDLE;
			}
		}
		return;
	}

	// DEATH 상태: 사망 애니메이션만 유지 (비활성화는 Monster::Damaged에서 처리)
	if (m_state == MONSTER_DEATH)
	{
		m_animator->SetState((int)MONSTER_DEATH, transform->GetDirection());
		return;
	}

	float minX = 0.0f, minY = 0.0f, maxX = 0.0f, maxY = 0.0f;
	bool hasBounds = false;
	if (SceneManager::GetInstance()->GetCurrentSceneType() == SCENE_GAME_FARMING_AREA)
	{
		GameScene* gameScene = static_cast<GameScene*>(SceneManager::GetInstance()->GetCurrentScene());
		if (gameScene && gameScene->HasWalkableBounds())
		{
			gameScene->GetWalkableBounds(minX, minY, maxX, maxY);
			hasBounds = true;
		}
	}

	auto clampToWalkable = [&](float& x, float& y) {
		if (!hasBounds) return;
		x = (std::max)(minX, (std::min)(maxX, x));
		y = (std::max)(minY, (std::min)(maxY, y));
	};

	// MONSTER_CHASE: 피격 후 플레이어를 Run 속도로 추적
	if (m_state == MONSTER_CHASE)
	{
		if (!m_aggroTarget || !m_aggroTarget->IsEnabled()) {
			m_aggroTarget = nullptr;
			m_state = MONSTER_IDLE;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
			return;
		}
		Transform* targetTr = m_aggroTarget->GetComponent<Transform>();
		if (!targetTr) {
			m_aggroTarget = nullptr;
			m_state = MONSTER_IDLE;
			return;
		}
		float tx = targetTr->GetX();
		float ty = targetTr->GetY();
		float cx = transform->GetX();
		float cy = transform->GetY();
		float dx = tx - cx;
		float dy = ty - cy;
		float distance = sqrtf(dx * dx + dy * dy);

		if (distance <= ATTACK_RANGE) {
			Direction newDir;
			if (distance < 0.0001f) newDir = transform->GetDirection();
			else if (std::abs(dx) > std::abs(dy)) newDir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
			else newDir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
			transform->SetDirection(newDir);
			m_state = MONSTER_ATTACK;
			m_animator->SetState((int)MONSTER_ATTACK, transform->GetDirection());
			return;
		}

		Direction newDir;
		if (std::abs(dx) > std::abs(dy)) newDir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
		else newDir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
		if (transform->GetDirection() != newDir) transform->SetDirection(newDir);
		m_animator->SetState((int)MONSTER_RUN, transform->GetDirection());

		float moveDist = m_runSpeed * deltaTime;
		float step = (std::min)(moveDist, distance);
		if (distance > 0.0001f) {
			float nx = cx + (dx / distance) * step;
			float ny = cy + (dy / distance) * step;
			clampToWalkable(nx, ny);
			transform->SetPosition(nx, ny);
		}
		return;
	}

	// MONSTER_ATTACK: 방향별 공격 콜라이더 갱신, hit 프레임에만 활성화 후 플레이어와 겹치면 데미지, 애니 종료 시 CHASE
	if (m_state == MONSTER_ATTACK)
	{
		m_animator->SetState((int)MONSTER_ATTACK, transform->GetDirection());
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
			if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
			m_state = MONSTER_CHASE;
		}
		return;
	}

	if (m_state == MONSTER_IDLE)
	{
		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration)
		{
			// 현재 위치를 중심으로 반경 안 무작위 목표 선택
			float cx = transform->GetX();
			float cy = transform->GetY();
			float angle = (rand() / (float)RAND_MAX) * 6.283185307f;  // 0 ~ 2*PI
			float dist = (rand() / (float)RAND_MAX) * m_actionRadius;  // 0 ~ radius
			m_targetX = cx + cosf(angle) * dist;
			m_targetY = cy + sinf(angle) * dist;
			clampToWalkable(m_targetX, m_targetY);

			m_state = MONSTER_WALK;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;  // 다음 IDLE 2~5초
		}
		else
		{
			m_animator->SetState((int)MONSTER_IDLE, transform->GetDirection());
		}
		return;
	}

	if (m_state == MONSTER_WALK)
	{
		float cx = transform->GetX();
		float cy = transform->GetY();
		float dx = m_targetX - cx;
		float dy = m_targetY - cy;
		float distance = sqrtf(dx * dx + dy * dy);

		// 방향 설정 (Player와 동일: 가로 우선)
		Direction newDir;
		if (distance < 0.0001f)
			newDir = transform->GetDirection();
		else if (std::abs(dx) > std::abs(dy))
			newDir = (dx > 0.0f) ? DIR_RIGHT : DIR_LEFT;
		else
			newDir = (dy > 0.0f) ? DIR_DOWN : DIR_UP;
		if (transform->GetDirection() != newDir)
			transform->SetDirection(newDir);
		m_animator->SetState((int)MONSTER_WALK, transform->GetDirection());

		const float arrivalEpsilon = 2.0f;
		float moveDist = m_walkSpeed * deltaTime;
		bool arrived = (distance < arrivalEpsilon) || (distance <= moveDist);

		if (arrived)
		{
			transform->SetPosition(m_targetX, m_targetY);
			float px = m_targetX, py = m_targetY;
			clampToWalkable(px, py);
			transform->SetPosition(px, py);

			m_state = MONSTER_IDLE;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
		}
		else
		{
			float step = (std::min)(moveDist, distance);
			float nx = cx + (dx / distance) * step;
			float ny = cy + (dy / distance) * step;
			clampToWalkable(nx, ny);
			transform->SetPosition(nx, ny);
		}
	}
}

bool Pig::OnInteraction(GameObject* obj)
{
	return Monster::OnInteraction(obj);
}

void Pig::Damaged(int damage)
{
	Monster::Damaged(damage);
	if (!IsDead() && IsEnabled()) {
		m_aggroTarget = ObjectManager::GetInstance()->GetPlayer();
	}
}

void Pig::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	// 몬스터 body 콜라이더 박스 표시 (클릭/상호작용 영역, 청록색)
	// Pig의 body 콜라이더 Gizmo는 ColliderManager::RenderGizmos()에서
	// 각 BoxCollider::RenderGizmo() 호출을 통해 공통 패턴으로 그려집니다.

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

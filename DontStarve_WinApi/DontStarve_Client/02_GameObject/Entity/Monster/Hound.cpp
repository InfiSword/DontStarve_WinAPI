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

Hound::Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, baseDir, imageName, colliderType)
{
	m_hp = 90;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;

	m_walkSpeed = 80.0f;
	m_runSpeed = 200.0f;
	
	// Hound는 더 넓은 어그로 범위
	m_aggroRadius = 350.0f;
}

Hound::~Hound() {}

void Hound::Init()
{
	Monster::Init();
	
	ChangeState((int)HoundState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_attackCooldownTimer = 0.0f;

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

			// 애니메이션 이벤트 등록 
			for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
				AnimationClip* clip = m_animator->GetAnimationClip((int)HoundState::ATTACK, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(HOUND_ATTACK_HIT_FRAME, L"attack_hit");
					clip->AddEventFrame(7, L"attack_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"attack_hit") this->OnAttackHit();
						else if (eventName == L"attack_end") this->OnAttackEnd();
					});
				}
			}

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

	// 공격 판정용 콜라이더 (ObjectManager에서 설정된 몸통 콜라이더는 그대로 사용)
	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		m_attackCollider->SetObjectCollider(HOUND_ATTACK_DOWN[0], HOUND_ATTACK_DOWN[1], HOUND_ATTACK_DOWN[2], HOUND_ATTACK_DOWN[3]);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Hound::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator)
		return;

	if (m_state == (int)HoundState::HIT)
	{
		if (m_animator->IsAnimationDone())
		{
			if (m_aggroTarget && m_aggroTarget->IsEnabled())
			{
				ChangeState((int)HoundState::CHASE);
				m_attackCooldownTimer = 0.0f;
			}
			else
			{
				ChangeState((int)HoundState::IDLE);
			}
		}
		return;
	}

	if (m_state == (int)HoundState::HOWL)
	{
		if (m_animator->IsAnimationDone())
		{
			ChangeState((int)HoundState::CHASE);
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

	// 어그로 체크 및 해제 (Monster 부모 클래스의 m_distToPlayer 활용)
	if (!m_aggroTarget && m_distToPlayerSq <= (m_aggroRadius * m_aggroRadius))
	{
		m_aggroTarget = ObjectManager::GetInstance()->GetPlayer();
		ChangeState((int)HoundState::HOWL);

		Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(newDir);
	}

	if (m_aggroTarget && m_distToPlayerSq > (m_deaggroRadius * m_deaggroRadius))
	{
		m_aggroTarget = nullptr;
		ChangeState((int)HoundState::IDLE);
		m_idleTimer = 0.0f;
		m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
		m_attackCooldownTimer = 0.0f;
	}

	if (m_state == (int)HoundState::CHASE)
	{
		if (!m_aggroTarget || !m_aggroTarget->IsEnabled()) {
			m_aggroTarget = nullptr;
			ChangeState((int)HoundState::IDLE);
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
			m_attackCooldownTimer = 0.0f;
			return;
		}

		Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);

		if (m_distToPlayerSq <= (ATTACK_RANGE * ATTACK_RANGE) && m_attackCooldownTimer <= 0.0f) {
			transform->SetDirection(newDir);
			ChangeState((int)HoundState::ATTACK_PRE);
			return;
		}

		if (m_distToPlayerSq <= (ATTACK_RANGE * ATTACK_RANGE) && m_attackCooldownTimer > 0.0f) {
			transform->SetDirection(newDir);
			ChangeState((int)HoundState::IDLE);
			return;
		}

		if (transform->GetDirection() != newDir) 
			transform->SetDirection(newDir);
		m_animator->SetState((int)HoundState::RUN, transform->GetDirection());

		float distToPlayer = sqrtf(m_distToPlayerSq);
		float moveDist = m_runSpeed * deltaTime;
		float step = (std::min)(moveDist, distToPlayer);
		transform->SetPosition(transform->GetX() + m_dirToPlayer.X * step, transform->GetY() + m_dirToPlayer.Y * step);
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
		return;
	}

	if (m_state == (int)HoundState::IDLE)
	{
		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration)
		{
			float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
			float dist = (rand() / (float)RAND_MAX) * m_wanderRadius;
			m_targetX = transform->GetX() + cosf(angle) * dist;
			m_targetY = transform->GetY() + sinf(angle) * dist;

			// 목표 위치가 맵 경계를 벗어나지 않도록 제한
			const float BUFFER = 50.0f;
			const float mapMaxX = static_cast<float>(MAP_WIDTH * TILE_SIZE) - BUFFER;
			const float mapMaxY = static_cast<float>(MAP_HEIGHT * TILE_SIZE) - BUFFER;
			const float mapMinX = BUFFER;
			const float mapMinY = BUFFER;

			if (m_targetX < mapMinX) m_targetX = mapMinX;
			if (m_targetX > mapMaxX) m_targetX = mapMaxX;
			if (m_targetY < mapMinY) m_targetY = mapMinY;
			if (m_targetY > mapMaxY) m_targetY = mapMaxY;

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
		float wdx = m_targetX - transform->GetX();
		float wdy = m_targetY - transform->GetY();
		float wdist = sqrtf(wdx * wdx + wdy * wdy);

		Direction newDir = (std::abs(wdx) > std::abs(wdy)) ? (wdx > 0.0f ? DIR_RIGHT : DIR_LEFT) : (wdy > 0.0f ? DIR_DOWN : DIR_UP);
		
		if (transform->GetDirection() != newDir)
			transform->SetDirection(newDir);
		m_animator->SetState((int)HoundState::RUN, transform->GetDirection());

		float moveDist = m_walkSpeed * deltaTime;
		if (wdist < 2.0f || wdist <= moveDist)
		{
			transform->SetPosition(m_targetX, m_targetY);
			m_state = (int)HoundState::IDLE;
			m_idleTimer = 0.0f;
			m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
		}
		else
		{
			float step = (std::min)(moveDist, wdist);
			transform->SetPosition(transform->GetX() + (wdx / wdist) * step, transform->GetY() + (wdy / wdist) * step);
		}
	}

	// 매 프레임 위치 경계 체크 (CHASE, RUN 상태에서 맵 밖으로 나가지 않도록)
	if (m_state == (int)HoundState::CHASE || m_state == (int)HoundState::RUN) {
		ClampPositionToMapBounds();
	}
}

bool Hound::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Hound::Damaged(int damage)
{
	Entity::Damaged(damage);
	ChangeState((int)HoundState::HIT);

	if (!IsDead() && IsEnabled()) {
		m_aggroTarget = ObjectManager::GetInstance()->GetPlayer();
		m_attackCooldownTimer = 0.0f;
	}
}

void Hound::OnAttackHit()
{
	if (m_state != (int)HoundState::ATTACK || !m_attackCollider || !transform) return;

	Direction dir = transform->GetDirection();
	if (dir == DIR_DOWN) m_attackCollider->SetObjectCollider(HOUND_ATTACK_DOWN[0], HOUND_ATTACK_DOWN[1], HOUND_ATTACK_DOWN[2], HOUND_ATTACK_DOWN[3]);
	else if (dir == DIR_UP) m_attackCollider->SetObjectCollider(HOUND_ATTACK_UP[0], HOUND_ATTACK_UP[1], HOUND_ATTACK_UP[2], HOUND_ATTACK_UP[3]);
	else if (dir == DIR_LEFT) m_attackCollider->SetObjectCollider(HOUND_ATTACK_LEFT[0], HOUND_ATTACK_LEFT[1], HOUND_ATTACK_LEFT[2], HOUND_ATTACK_LEFT[3]);
	else m_attackCollider->SetObjectCollider(HOUND_ATTACK_RIGHT[0], HOUND_ATTACK_RIGHT[1], HOUND_ATTACK_RIGHT[2], HOUND_ATTACK_RIGHT[3]);

	m_attackCollider->SetColliderEnabled(true);

	if (m_aggroTarget && m_aggroTarget->IsEnabled()) {
		Transform* pt = m_aggroTarget->GetComponent<Transform>();
		if (pt && m_attackCollider->ContainsPoint(pt->GetX(), pt->GetY())) {
			m_aggroTarget->Damaged(20);
		}
	}

	m_attackCollider->SetColliderEnabled(false);
}

void Hound::OnAttackEnd()
{
	if (m_state != (int)HoundState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	ChangeState((int)HoundState::CHASE);
}

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

Hound::Hound(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
             const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
{
	m_hp = 90;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;

	m_walkSpeed = 80.0f;
	m_runSpeed = 200.0f;
	
	// 공격 관련 설정
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
	
	// 하운드는 항상 플레이어를 추격하는 타입(공격적인 몬스터)
	SetupAggro(AggroType::ALWAYS, 0.0f, 0.0f);

	// 공격 박스 설정 (방향별 오프셋 자동 계산)
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)HoundState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_attackCooldownTimer = 0.0f;

	if (this->transform) {
		m_targetX = this->transform->GetX();
		m_targetY = this->transform->GetY();
	}
	
	OutputDebugStringW((L"Hound: Init 완료 - ID: " + std::to_wstring(m_id) + L"\n").c_str());

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
					clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
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

	// 공격 전용 콜라이더 (ObjectManager에서 설정한 몸통 콜라이더는 그대로 사용)
	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		UpdateAttackBoxByDirection(DIR_DOWN);
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
			if (m_attackTarget && m_attackTarget->IsEnabled())
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

	// ALWAYS 타입이므로 어그로 해제 체크 불필요
	if (m_state == (int)HoundState::CHASE)
	{
		Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);

		if (m_distToPlayerSq <= (m_attackRange * m_attackRange) && m_attackCooldownTimer <= 0.0f) {
			transform->SetDirection(newDir);
			ChangeState((int)HoundState::ATTACK_PRE);
			return;
		}

		if (m_distToPlayerSq <= (m_attackRange * m_attackRange) && m_attackCooldownTimer > 0.0f) {
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
			m_attackCooldownTimer = m_attackCooldown;
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
		// ALWAYS 타입이므로 배회하지 않고 바로 추격
		// 어그로가 켜져있으면 HOWL -> CHASE로 전환
		if (m_attackTarget && m_attackTarget->IsEnabled())
		{
			ChangeState((int)HoundState::HOWL);
			
			Direction newDir = (std::abs(m_dirToPlayer.X) > std::abs(m_dirToPlayer.Y)) ? (m_dirToPlayer.X > 0.0f ? DIR_RIGHT : DIR_LEFT) : (m_dirToPlayer.Y > 0.0f ? DIR_DOWN : DIR_UP);
			transform->SetDirection(newDir);
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

	// 맵 경계 위치 경계 체크 (CHASE 상태에서 맵 밖으로 나가지 않도록)
	if (m_state == (int)HoundState::CHASE) {
		ClampPositionToMapBounds();
	}
}

void Hound::Damaged(int damage)
{
	Entity::Damaged(damage);
	ChangeState((int)HoundState::HIT);

	if (!IsDead() && IsEnabled()) {
		m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
		m_attackCooldownTimer = 0.0f;
	}
}

void Hound::OnAttackHit()
{
	if (m_state != (int)HoundState::ATTACK) return;
	
	// Monster 기본 클래스의 공격 처리 사용
	ProcessAttackHit(m_damage);
}

void Hound::OnAttackEnd()
{
	if (m_state != (int)HoundState::ATTACK) return;
	if (m_attackCollider) m_attackCollider->SetColliderEnabled(false);
	ChangeState((int)HoundState::CHASE);
}

void Hound::Die()
{
    ChangeState((int)HoundState::DEATH);
    // Monster::Update will call Entity::Update to advance the death animation
}

bool Hound::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}



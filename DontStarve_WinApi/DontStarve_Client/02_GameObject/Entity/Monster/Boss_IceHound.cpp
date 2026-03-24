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
#include "Boss_IceHound.h"

Boss_IceHound::Boss_IceHound(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
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

Boss_IceHound::~Boss_IceHound() {}

void Boss_IceHound::Init()
{
	Monster::Init();
	m_bUseSuperArmor = true;
	SetupAggro(AggroType::ALWAYS, 0.0f, 0.0f);
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)BossIceHoundState::IDLE);
	m_idleTimer = 0.0f;
	m_idleDuration = 2.0f + (rand() / (float)RAND_MAX) * 3.0f;
	m_bHasHowled = false;
	m_bCanChase = false; // 초기에는 추격 불가 (시네마틱 대기)

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
			std::wstring prefix = L"IceHound_";
			std::wstring houndPrefix = prefix + L"hound_";
			float px = transform->GetPivotX();
			float py = transform->GetPivotY();

			m_animator->RegisterAnimation((int)BossIceHoundState::IDLE, DIR_DOWN, base + houndPrefix + L"idle_down.png", 0, 0, 7, 20, px, py, true, 0.03f);
			m_animator->RegisterAnimation((int)BossIceHoundState::IDLE, DIR_UP, base + houndPrefix + L"idle_up.png", 0, 0, 7, 20, px, py, true, 0.03f);
			std::wstring idleSidePath = base + houndPrefix + L"idle_side.png";
			m_animator->RegisterAnimation((int)BossIceHoundState::IDLE, DIR_LEFT, idleSidePath, 0, 0, 7, 20, px, py, true, 0.03f, false);
			m_animator->RegisterAnimation((int)BossIceHoundState::IDLE, DIR_RIGHT, idleSidePath, 0, 0, 7, 20, px, py, true, 0.03f);

			for (int state = (int)BossIceHoundState::WALK; state <= (int)BossIceHoundState::CHASE; ++state) {
				if (state != (int)BossIceHoundState::WALK && state != (int)BossIceHoundState::CHASE) continue;
				m_animator->RegisterAnimation(state, DIR_DOWN, base + houndPrefix + L"run_loop_down.png", 0, 0, 7, 16, px, py, true, 0.03f);
				m_animator->RegisterAnimation(state, DIR_UP, base + houndPrefix + L"run_loop_up.png", 0, 0, 7, 16, px, py, true, 0.03f);
				std::wstring walkSidePath = base + houndPrefix + L"run_loop_side.png";
				m_animator->RegisterAnimation(state, DIR_LEFT, walkSidePath, 0, 0, 7, 16, px, py, true, 0.03f, false);
				m_animator->RegisterAnimation(state, DIR_RIGHT, walkSidePath, 0, 0, 7, 16, px, py, true, 0.03f);
			}

			m_animator->RegisterAnimation((int)BossIceHoundState::ATTACK_PRE, DIR_DOWN, base + houndPrefix + L"atk_pre_down.png", 0, 0, 7, 29, px, py, false, 0.02f);
			m_animator->RegisterAnimation((int)BossIceHoundState::ATTACK_PRE, DIR_UP, base + houndPrefix + L"atk_pre_up.png", 0, 0, 7, 29, px, py, false, 0.02f);
			std::wstring atkPreSidePath = base + houndPrefix + L"atk_pre_side.png";
			m_animator->RegisterAnimation((int)BossIceHoundState::ATTACK_PRE, DIR_LEFT, atkPreSidePath, 0, 0, 7, 29, px, py, false, 0.02f, false);
			m_animator->RegisterAnimation((int)BossIceHoundState::ATTACK_PRE, DIR_RIGHT, atkPreSidePath, 0, 0, 7, 29, px, py, false, 0.02f);

			m_animator->RegisterAnimation((int)BossIceHoundState::ATTACK, DIR_DOWN, base + houndPrefix + L"atk_down.png", 0, 0, 7, 18, px, py, false, 0.03f);
			m_animator->RegisterAnimation((int)BossIceHoundState::ATTACK, DIR_UP, base + houndPrefix + L"atk_up.png", 0, 0, 7, 18, px, py, false, 0.03f);
			std::wstring atkSidePath = base + houndPrefix + L"atk_side.png";
			m_animator->RegisterAnimation((int)BossIceHoundState::ATTACK, DIR_LEFT, atkSidePath, 0, 0, 7, 18, px, py, false, 0.03f, false);
			m_animator->RegisterAnimation((int)BossIceHoundState::ATTACK, DIR_RIGHT, atkSidePath, 0, 0, 7, 18, px, py, false, 0.03f);

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossIceHoundState::ATTACK, (Direction)dir);
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
				m_animator->RegisterAnimation((int)BossIceHoundState::HIT, (Direction)dir, base + houndPrefix + L"hit_side.png", 0, 0, 7, 27, px, py, false, 0.02f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossIceHoundState::HIT, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(26, L"hit_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"hit_end") this->OnHitEnd();
						});
				}
			}

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)BossIceHoundState::DEATH, (Direction)dir, base + houndPrefix + L"death.png", 0, 0, 7, 52, px, py, false, 0.02f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossIceHoundState::DEATH, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(51, L"death_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"death_end") this->OnDeathEnd();
						});
				}
			}

			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				m_animator->RegisterAnimation((int)BossIceHoundState::HOWL, (Direction)dir, base + houndPrefix + L"howl.png", 0, 0, 7, 47, px, py, false, 0.03f);
				AnimationClip* clip = m_animator->GetAnimationClip((int)BossIceHoundState::HOWL, (Direction)dir);
				if (clip) {
					clip->AddEventFrame(46, L"howl_end");
					clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
						if (eventName == L"howl_end") {
							m_bHasHowled = true;
							if (m_bCanChase) ChangeState((int)BossIceHoundState::CHASE);
							else ChangeState((int)BossIceHoundState::IDLE);
						}
						});
				}
			}
		}

		ChangeState(m_state);
	}
}

void Boss_IceHound::RenderDebugOverlay()
{
	Combatant::RenderDebugOverlay();
}

bool Boss_IceHound::OnInteraction(GameObject* obj) { return Entity::OnInteraction(obj); }

void Boss_IceHound::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	if (m_state == (int)BossIceHoundState::HOWL)
	{
		// HOWL은 애니메이션 이벤트(howl_end)에서 상태 전이가 이루어짐
		return;
	}
	if (m_state == (int)BossIceHoundState::ATTACK_PRE)
	{
		if (m_animator->IsAnimationDone()) 
			ChangeState((int)BossIceHoundState::ATTACK);
		return;
	}

	Monster::UpdateAI(deltaTime);

	if (m_state == (int)BossIceHoundState::CHASE || m_state == (int)BossIceHoundState::WALK) ClampPositionToMapBounds();
}

void Boss_IceHound::UpdateMovement(float deltaTime)
{
	Monster::UpdateMovement(deltaTime);
}

int Boss_IceHound::UpdateIdle(float deltaTime)
{
	int nextState = Monster::UpdateIdle(deltaTime);

	if (nextState == (int)BossIceHoundState::CHASE && !m_bHasHowled)
	{
		return (int)BossIceHoundState::HOWL;
	}

	if (nextState == (int)BossIceHoundState::ATTACK)
	{
		return (int)BossIceHoundState::ATTACK_PRE;
	}

	return nextState;
}

int Boss_IceHound::UpdateWalk(float deltaTime)
{
	int nextState = Monster::UpdateWalk(deltaTime);

	if (nextState == (int)BossIceHoundState::CHASE && !m_bHasHowled)
	{
		return (int)BossIceHoundState::HOWL;
	}

	return nextState;
}

int Boss_IceHound::UpdateChase(float deltaTime)
{
	int nextState = Monster::UpdateChase(deltaTime);

	if (nextState == (int)BossIceHoundState::ATTACK)
	{
		return (int)BossIceHoundState::ATTACK_PRE;
	}

	return nextState;
}

void Boss_IceHound::Damaged(int damage)
{
	Monster::Damaged(damage);
	if (IsDead()) return;

	if (CheckSuperArmorHit()) return;

	ChangeState((int)BossIceHoundState::HIT);
}

void Boss_IceHound::OnAttackHit()
{
	if (m_state == (int)BossIceHoundState::ATTACK)
		ProcessAttackHit(m_damage);
}

void Boss_IceHound::OnAttackEnd()
{
	if (m_state != (int)BossIceHoundState::ATTACK) return;

	HandleAttackEndSuperArmor();

	ChangeState((int)BossIceHoundState::CHASE);
}

void Boss_IceHound::OnHitEnd()
{
	if (m_state != (int)BossIceHoundState::HIT) return;
	ChangeState((int)BossIceHoundState::IDLE);
}

void Boss_IceHound::Die() { ChangeState((int)BossIceHoundState::DEATH); }
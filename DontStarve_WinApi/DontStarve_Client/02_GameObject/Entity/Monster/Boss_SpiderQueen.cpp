#include "99_Default/pch.h"
#include "../../Component/Transform/Transform.h"
#include "Boss_SpiderQueen.h"
#include "../Player/Player.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../../01_Manager/SceneManager/SceneManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Collider/BoxCollider.h"

const float Boss_SpiderQueen::ATTACK_RANGE = 70.0f;
const float Boss_SpiderQueen::ATTACK_COOLDOWN = 1.5f;
static const int PIG_ATTACK_HIT_FRAME = 28;
static const int BOSS_SPIDERQUEEN_ATTACK_BOX_W = 70, BOSS_SPIDERQUEEN_ATTACK_BOX_H = 50;
static const int PIG_ATTACK_DOWN[] = { -35,    0, BOSS_SPIDERQUEEN_ATTACK_BOX_W, BOSS_SPIDERQUEEN_ATTACK_BOX_H };
static const int PIG_ATTACK_UP[] = { -35,  -50, BOSS_SPIDERQUEEN_ATTACK_BOX_W, BOSS_SPIDERQUEEN_ATTACK_BOX_H };
static const int PIG_ATTACK_LEFT[] = { -70,  -25, BOSS_SPIDERQUEEN_ATTACK_BOX_W, BOSS_SPIDERQUEEN_ATTACK_BOX_H };
static const int PIG_ATTACK_RIGHT[] = { 0,  -25, BOSS_SPIDERQUEEN_ATTACK_BOX_W, BOSS_SPIDERQUEEN_ATTACK_BOX_H };

Boss_SpiderQueen::Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& baseDir, const std::wstring& imageName)
	: Entity(id, x, y, pivotX, pivotY, DIR_DOWN, baseDir, imageName, true, true)
	, m_bossPhase(1)
	, m_specialAttackCooldown(0.0f)
	, m_walkSpeed(50.0f), 
	m_attackCooldownTimer(0.0f), 
	m_idleTimer(0.0f), 
	m_idleDuration(2.0f), 
	m_targetX(x), 
	m_targetY(y), 
	m_aggroTarget(nullptr), 
	m_attackCollider(nullptr)
{
	m_hp = 1000;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
}

Boss_SpiderQueen::~Boss_SpiderQueen() {}

void Boss_SpiderQueen::Init()
{
	Entity::Init();

	m_state = (int)SpiderQueenState::IDLE;
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;


	if (!m_animator)
	{
		m_animator = AddComponent<Animator>();
	}

	ResourceManager* pRM = ResourceManager::GetInstance();
	const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_QUEEN_SPIDER);
	if (objData) {
		std::wstring base = objData->baseDir + L"\\";
		float px = transform->GetPivotX();
		float py = transform->GetPivotY();

		std::wstring idlePath = base + L"Queen_spider_queen_idle_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::IDLE, (Direction)dir, idlePath,
				0, 0, 4, 50, px, py, true, 0.02f);
		}

		std::wstring walkPath = base + L"Walk_spider_queen_walk_loop_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::CHASE, (Direction)dir, walkPath,
				0, 0, 7, 65, px, py, true, 0.02f);
		}

		std::wstring attackPath = base + L"Queen_spider_queen_atk_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::ATTACK, (Direction)dir, attackPath,
				0, 0, 7, 53, px, py, false, 0.02f);
		}

		m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_DOWN, base + L"Queen_spider_queen_hit_side.png",
			0, 0, 7, 29, px, py, false, 0.02f);
		m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_UP, base + L"Queen_spider_queen_hit_side.png",
			0, 0, 7, 29, px, py, false, 0.02f);
		m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_LEFT, base + L"Queen_spider_queen_hit_side.png",
			0, 0, 7, 29, px, py, false, 0.02f);
		m_animator->RegisterAnimation((int)SpiderQueenState::HIT, DIR_RIGHT, base + L"Queen_spider_queen_hit_side.png",
			0, 0, 7, 29, px, py, false, 0.02f);

		for (int dir = DIR_DOWN; dir <= DIR_RIGHT; dir++) {
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::ATTACK, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(PIG_ATTACK_HIT_FRAME, L"attack_hit");
				clip->AddEventFrame(65, L"attack_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"attack_hit") this->OnAttackHit();
					else if (eventName == L"attack_end") this->OnAttackEnd();
					});
			}
		}

		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::DEATH, (Direction)dir, base + L"Queen_spider_queen_death.png",
				0, 0, 7, 45, px, py, false, 0.03f);
		}

		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::TAUNT, (Direction)dir, base + L"Queen_spider_queen_taunt.png",
				0, 0, 7, 50, px, py, false, 0.03f);
		}
	}

	m_animator->SetState(m_state, this->transform->GetDirection());
}

void Boss_SpiderQueen::Update(float deltaTime)
{
	Entity::Update(deltaTime);

	if (!IsEnabled() || !transform || !m_animator)
		return;

	// 1. 공통 쿨타임 감소
	if (m_attackCooldownTimer > 0.0f) {
		m_attackCooldownTimer -= deltaTime;
	}

	// 2. 애니메이션 기반 상태 처리 (HIT, DEATH, ATTACK)
	if (m_state == (int)SpiderQueenState::HIT || m_state == (int)SpiderQueenState::DEATH || m_state == (int)SpiderQueenState::ATTACK)
	{
		m_animator->SetState(m_state, transform->GetDirection());

		if (m_animator->IsAnimationDone())
		{
			if (m_state == (int)SpiderQueenState::DEATH) {
				ObjectManager::GetInstance()->RemoveGameObject(this);
				return;
			}

			if (m_state == (int)SpiderQueenState::ATTACK) {
				OnAttackEnd();
			}
			else if (m_state == (int)SpiderQueenState::HIT) {
				// 맞아서 HIT 상태가 끝난 후, 타겟(나를 때린 놈)이 있으면 추격 시작
				if (m_aggroTarget && m_aggroTarget->IsEnabled())
					m_state = (int)SpiderQueenState::CHASE;
				else
					m_state = (int)SpiderQueenState::IDLE;
			}
		}
		return;
	}

	// 3. 타겟(나를 때린 플레이어) 정보 계산
	float distToPlayer = 99999.0f;
	float dx = 0.0f, dy = 0.0f;

	if (m_aggroTarget && m_aggroTarget->IsEnabled()) {
		dx = m_aggroTarget->GetComponent<Transform>()->GetX() - transform->GetX();
		dy = m_aggroTarget->GetComponent<Transform>()->GetY() - transform->GetY();
		distToPlayer = sqrtf(dx * dx + dy * dy);
	}

	// 4. 메인 상태 머신 (CHASE, IDLE, WALK)

	// [CHASE 상태] - 플레이어에게 맞아서 타겟이 생겼을 때만 진입됨
	if (m_state == (int)SpiderQueenState::CHASE)
	{
		if (!m_aggroTarget || !m_aggroTarget->IsEnabled()) {
			m_aggroTarget = nullptr;
			m_state = (int)SpiderQueenState::IDLE;
			m_idleTimer = 0.0f;
			return;
		}

		Direction newDir = (std::abs(dx) > std::abs(dy)) ? (dx > 0.0f ? DIR_RIGHT : DIR_LEFT) : (dy > 0.0f ? DIR_DOWN : DIR_UP);
		transform->SetDirection(newDir);

		if (distToPlayer <= ATTACK_RANGE) {
			if (m_attackCooldownTimer <= 0.0f) {
				m_state = (int)SpiderQueenState::ATTACK;
				m_animator->SetState((int)SpiderQueenState::ATTACK, transform->GetDirection());
				m_attackCooldownTimer = ATTACK_COOLDOWN;
				return;
			}
			else {
				m_animator->SetState((int)SpiderQueenState::IDLE, transform->GetDirection());
				return;
			}
		}

		m_animator->SetState((int)SpiderQueenState::CHASE, transform->GetDirection());
		float moveDist = m_walkSpeed * deltaTime;
		float step = (std::min)(moveDist, distToPlayer);
		transform->SetPosition(transform->GetX() + (dx / distToPlayer) * step, transform->GetY() + (dy / distToPlayer) * step);
	}

	else if (m_state == (int)SpiderQueenState::IDLE)
	{
		m_idleTimer += deltaTime;
		if (m_idleTimer >= m_idleDuration) 
		{

			m_state = (int)SpiderQueenState::CHASE;
			m_idleTimer = 0.0f;
		}
		else {
			m_animator->SetState((int)SpiderQueenState::IDLE, transform->GetDirection());
		}
	}

}

void Boss_SpiderQueen::OnAttackHit() {
	// 공격이 적중하는 프레임에 호출되는 콜백
	// 이곳에서 플레이어와의 충돌 판정 및 데미지 처리를 수행
}

void Boss_SpiderQueen::OnAttackEnd() {
	// 공격 애니메이션이 끝나는 프레임에 호출되는 콜백
	// 이곳에서 공격이 끝난 후의 상태 전환이나 쿨다운 초기화 등을 수행
}	

bool Boss_SpiderQueen::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void Boss_SpiderQueen::Damaged(int damage)
{
	m_hp -= damage;
	m_state = (int)SpiderQueenState::HIT;

	if (m_hp <= m_maxHp * 0.5f && m_bossPhase == 1) {
		m_bossPhase = 2;
		OutputDebugStringW(L"Boss_SpiderQueen: 보스 페이즈가 2단계로 전환!\n");
	}

	if (m_hp <= 0) {
		m_hp = 0;
		m_state = (int)SpiderQueenState::DEATH;
		m_isDead = true;
		SceneType currentScene = SceneManager::GetInstance()->GetCurrentSceneType();
		GameProgressManager::GetInstance()->OnMonsterKilled(GetID(), currentScene);
		OutputDebugStringW(L"Boss_SpiderQueen: 보스가 처치되었습니다!\n");
	}
}

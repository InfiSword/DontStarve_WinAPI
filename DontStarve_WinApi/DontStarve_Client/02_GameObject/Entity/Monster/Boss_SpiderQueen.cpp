#include "99_Default/pch.h"
#include "Boss_SpiderQueen.h"
#include "Spider.h"
#include "../Player/Player.h"
#include "../../../01_Manager/DataManager/DataManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../Building/SpiderEgg.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"

Boss_SpiderQueen::Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
	const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Spider(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
	, m_bossPhase(1)
	, m_specialAttackCooldown(0.0f)
	, m_comboAttackCooldown(5.0f)
	, m_comboCount(0)
	, m_poopCooldown(15.0f)
	, m_poopCount(0)
	, m_idleTimer(0.0f)
	, m_idleDuration(2.0f)
	, m_hasTriggeredCocoon(false)
	, m_cocoonTimer(0.0f)
	, m_healTickTimer(0.0f)
	, m_isHealing(false)
	, m_healFxAnimator(nullptr)
	, m_spawnOutFxAnimator(nullptr)
	, m_spawnOnHitCooldown(0.0f)
{
	m_hp = 2500;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
	m_walkSpeed = 50.0f;
	m_runSpeed = 120.0f;
	m_attackRange = 130.0f;
	m_attackCooldown = 1.5f;
	m_attackHitFrame = 40;
	m_damage = 25;
	m_attackBoxWidth = 110;
	m_attackBoxHeight = 80;
}

Boss_SpiderQueen::~Boss_SpiderQueen() {}

void Boss_SpiderQueen::Init()
{
	Monster::Init();
	m_bUseSuperArmor = true;
	SetupAggro(AggroType::ALWAYS, 0.0f, 0.0f);
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)SpiderQueenState::IDLE);
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;

	if (!m_animator) m_animator = AddComponent<Animator>(spriteRenderer);

	// FX Animator 추가
	if (!m_healFxAnimator) m_healFxAnimator = AddComponent<Animator>(spriteRenderer);
	if (!m_spawnOutFxAnimator) m_spawnOutFxAnimator = AddComponent<Animator>(spriteRenderer);

	DataManager* pRM = DataManager::GetInstance();
	const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_QUEEN_SPIDER);
	if (objData) {
		std::wstring base = objData->baseDir + L"\\";
		float px = objData->pivotX;
		float py = objData->pivotY;

		std::wstring idlePath = base + L"Queen_spider_queen_idle_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) m_animator->RegisterAnimation((int)SpiderQueenState::IDLE, (Direction)dir, idlePath, 0, 0, 4, 50, px, py, true, 0.02f);

		std::wstring walkPath = base + L"Walk_spider_queen_walk_loop_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) m_animator->RegisterAnimation((int)SpiderQueenState::CHASE, (Direction)dir, walkPath, 0, 0, 7, 65, px, py, true, 0.02f);

		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) m_animator->RegisterAnimation((int)SpiderQueenState::WALK, (Direction)dir, walkPath, 0, 0, 7, 65, px, py, true, 0.03f);

		std::wstring attackPath = base + L"Queen_spider_queen_atk_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::ATTACK, (Direction)dir, attackPath, 0, 0, 7, 53, px, py, false, 0.02f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::ATTACK, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(m_attackHitFrame, L"attack_hit");
				clip->AddEventFrame(52, L"attack_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"attack_hit") this->OnAttackHit();
					else if (eventName == L"attack_end") this->OnAttackEnd();
					});
			}
		}

		// 3콤보 공격 애니메이션 (기존 Attack 리소스 재사용)
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::COMBO_ATTACK, (Direction)dir, attackPath, 0, 0, 7, 53, px, py, false, 0.015f); // 콤보는 약간 더 빠르게
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::COMBO_ATTACK, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(m_attackHitFrame, L"combo_hit");
				clip->AddEventFrame(52, L"combo_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"combo_hit") this->OnComboAttackHit();
					else if (eventName == L"combo_end") this->OnComboAttackEnd();
					});
			}
		}

		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::HIT, (Direction)dir, base + L"Queen_spider_queen_hit_side.png", 0, 0, 7, 29, px, py, false, 0.02f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::HIT, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(28, L"hit_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"hit_end") this->OnHitEnd();
					});
			}
		}

		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::DEATH, (Direction)dir, base + L"Queen_spider_queen_death.png", 0, 0, 7, 45, px, py, false, 0.03f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::DEATH, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(44, L"death_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"death_end") this->OnDeathEnd();
					});
			}
		}

		// ENTER (BIRTH) 애니메이션
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::BIRTH, (Direction)dir, base + L"Queen_spider_queen_enter.png", 0, 0, 7, 58, px, py, false, 0.02f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::BIRTH, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(57, L"birth_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"birth_end") this->OnBirthEnd();
					});
			}
		}

		// COCOON_PRE 애니메이션
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::COCOON_PRE, (Direction)dir, base + L"Cocoon_spider_queen_cocoon.png", 0, 0, 7, 56, px, py, false, 0.02f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::COCOON_PRE, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(44, L"cocoon_pre_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"cocoon_pre_end") this->OnCocoonPreEnd();
					});
			}
		}

		// COCOON 상태: SmallEgg 리소스 사용
		const ResourcePathUtils::ObjectResourceDef* eggData = pRM->GetObjectResourceInfo(GOID_BUILDING_SPIDER_SMALLEGG);
		if (eggData) {
			std::wstring eggBase = eggData->baseDir + L"\\";
			for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
				// Idle
				m_animator->RegisterAnimation((int)SpiderQueenState::COCOON, (Direction)dir, eggBase + L"Egg_spider_cocoon_small_Idle.png", 0, 0, 1, 1, px, py, true, 0.02f);
				// Hit
				m_animator->RegisterAnimation((int)SpiderQueenState::COCOON_HIT, (Direction)dir, eggBase + L"Hit\\Egg_spider_cocoon_cocoon_small_hit.png", 0, 0, 7, 33, px, py, false, 0.02f);
			}
		}

		// TAUNT 애니메이션
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::TAUNT, (Direction)dir, base + L"Queen_spider_queen_taunt.png", 0, 0, 7, 50, px, py, false, 0.03f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::TAUNT, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(49, L"taunt_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"taunt_end")
					{
						m_bHasTaunted = true;
						m_bCanChase = false;
						ChangeState((int)SpiderQueenState::IDLE);
					}
					});
			}
		}

		// POOP_PRE 애니메이션
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::POOP_PRE, (Direction)dir, base + L"Queen_spider_queen_poop_pre.png", 0, 0, 7, 13, px, py, false, 0.03f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::POOP_PRE, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(12, L"poop_pre_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"poop_pre_end") this->ChangeState((int)SpiderQueenState::POOP_LOOP);
					});
			}
		}

		// POOP_LOOP 애니메이션 (알 3개 낳기)
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::POOP_LOOP, (Direction)dir, base + L"Queen_spider_queen_poop_loop.png", 0, 0, 7, 50, px, py, false, 0.03f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::POOP_LOOP, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(10, L"poop_egg");
				clip->AddEventFrame(25, L"poop_egg");
				clip->AddEventFrame(40, L"poop_egg");
				clip->AddEventFrame(49, L"poop_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"poop_egg") this->OnPoopEgg();
					else if (eventName == L"poop_end") this->OnPoopEnd();
					});
			}
		}

		// FX 등록
// 1. Heal Buff FX (루프)
		m_healFxAnimator->RegisterAnimation(0, DIR_DOWN, base + L"Queen_heal_fx_heal_buff.png", 0, 0, 7, 65, px, py, true, 0.02f);

		// 2. Spawn Out FX (단발)
		m_spawnOutFxAnimator->RegisterAnimation(0, DIR_DOWN, base + L"FX_splash_spiderweb_idle.png", 0, 0, 7, 45, px, py, false, 0.02f);
	}
	ChangeState(m_state);
	m_healFxAnimator->SetActive(false);
	m_spawnOutFxAnimator->SetActive(false);

	if (m_attackCollider) {
		UpdateAttackBoxByDirection(DIR_DOWN);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Boss_SpiderQueen::RenderDebugOverlay()
{
	Combatant::RenderDebugOverlay();
}

void Boss_SpiderQueen::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	switch ((SpiderQueenState)m_state)
	{
	case SpiderQueenState::TAUNT:
		return;

	case SpiderQueenState::COCOON_PRE:
	case SpiderQueenState::BIRTH:
		// 애니메이션 이벤트로 상태 전이하므로 별도 로직 없음
		break;

	case SpiderQueenState::COCOON:
	case SpiderQueenState::COCOON_HIT:
		m_cocoonTimer += deltaTime;
		m_healTickTimer += deltaTime;
		if (m_spawnOnHitCooldown > 0.0f) m_spawnOnHitCooldown -= deltaTime;

		// 1초마다 5% 회복
		if (m_healTickTimer >= 1.0f)
		{
			m_hp += static_cast<int>(m_maxHp * 0.05f);
			if (m_hp > m_maxHp) m_hp = m_maxHp;
			m_healTickTimer = 0.0f;
		}

		// 피격 애니메이션 종료 후 복귀
		if (m_state == (int)SpiderQueenState::COCOON_HIT && m_animator->IsAnimationDone())
		{
			ChangeState((int)SpiderQueenState::COCOON);
		}

		// 10초 후 종료
		if (m_cocoonTimer >= 10.0f)
		{
			EndCocoonPhase();
		}
		break;

	case SpiderQueenState::COMBO_ATTACK:
		// 콤보 공격 중에는 별도 AI 로직 없음 (애니메이션 이벤트로 처리)
		break;

	case SpiderQueenState::POOP_PRE:
	case SpiderQueenState::POOP_LOOP:
		// 알 낳기 중에는 별도 AI 로직 없음
		break;

	default:
		if (m_comboAttackCooldown > 0.0f) m_comboAttackCooldown -= deltaTime;
		if (m_poopCooldown > 0.0f) m_poopCooldown -= deltaTime;
		Monster::UpdateAI(deltaTime);
		break;
	}
}

void Boss_SpiderQueen::UpdateMovement(float deltaTime)
{
	if (!IsEnabled()) return;

	switch ((SpiderQueenState)m_state)
	{
	case SpiderQueenState::COCOON:
	case SpiderQueenState::BIRTH:
	case SpiderQueenState::COCOON_PRE:
	case SpiderQueenState::TAUNT:
	case SpiderQueenState::COMBO_ATTACK:
	case SpiderQueenState::POOP_PRE:
	case SpiderQueenState::POOP_LOOP:
		break;
	default:
		Monster::UpdateMovement(deltaTime);
		break;
	}
}

int Boss_SpiderQueen::UpdateIdle(float deltaTime)
{
	int nextState = Monster::UpdateIdle(deltaTime);

	if (nextState == (int)SpiderQueenState::CHASE && !m_bHasTaunted)
	{
		return (int)SpiderQueenState::TAUNT;
	}

	return nextState;
}

int Boss_SpiderQueen::UpdateWalk(float deltaTime)
{
	int nextState = Monster::UpdateWalk(deltaTime);

	if (nextState == (int)SpiderQueenState::CHASE && !m_bHasTaunted)
	{
		return (int)SpiderQueenState::TAUNT;
	}

	return nextState;
}

int Boss_SpiderQueen::UpdateChase(float deltaTime)
{
	if (m_poopCooldown <= 0.0f)
	{
		m_poopCooldown = 20.0f; // 사용 후 20초 쿨타임
		return (int)SpiderQueenState::POOP_PRE;
	}

	if (m_comboAttackCooldown <= 0.0f && m_distToPlayerSq <= m_attackRange * m_attackRange * 2.25f) // 약간 더 먼 거리에서도 발동 가능
	{
		m_comboCount = 0;
		m_comboAttackCooldown = 8.0f; // 사용 후 8초 쿨타임
		return (int)SpiderQueenState::COMBO_ATTACK;
	}

	return Monster::UpdateChase(deltaTime);
}

void Boss_SpiderQueen::OnAttackHit() { if (m_state == (int)SpiderQueenState::ATTACK) ProcessAttackHit(m_damage); }

void Boss_SpiderQueen::OnAttackEnd()
{
	if (m_state != (int)SpiderQueenState::ATTACK) return;

	HandleAttackEndSuperArmor();

	ChangeState((int)SpiderQueenState::CHASE);
}

void Boss_SpiderQueen::OnComboAttackHit()
{
	if (m_state != (int)SpiderQueenState::COMBO_ATTACK) return;

	// 플레이어 방향으로 전진 (약 40픽셀)
	if (transform)
	{
		float moveDist = 40.0f;
		float nx = transform->GetX() + m_dirToPlayer.X * moveDist;
		float ny = transform->GetY() + m_dirToPlayer.Y * moveDist;
		transform->SetPosition(nx, ny);
		ClampPositionToMapBounds();
	}

	ProcessAttackHit(m_damage);
}

void Boss_SpiderQueen::OnComboAttackEnd()
{
	if (m_state != (int)SpiderQueenState::COMBO_ATTACK) return;

	m_comboCount++;
	if (m_comboCount < 3)
	{
		// 애니메이션 재시작 (콤보 연결)
		ChangeState((int)SpiderQueenState::COMBO_ATTACK, true);
	}
	else
	{
		m_comboCount = 0;
		HandleAttackEndSuperArmor();
		ChangeState((int)SpiderQueenState::CHASE);
	}
}

void Boss_SpiderQueen::OnPoopEgg()
{
	if (m_state != (int)SpiderQueenState::POOP_LOOP) return;

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager || !transform) return;

	// 무작위 3개 알 생성 (이벤트 콜백이 3번 호출됨)
	float angle = (rand() / (float)RAND_MAX) * 6.283185f;
	float dist = 100.0f + (rand() / (float)RAND_MAX) * 100.0f;
	float ex = transform->GetX() + cosf(angle) * dist;
	float ey = transform->GetY() + sinf(angle) * dist;

	// 알 ID 랜덤 선택 (Small, Normal, Tall 중 하나)
	GameObjectID eggIDs[] = { GOID_BUILDING_SPIDER_SMALLEGG, GOID_BUILDING_SPIDER_NORMALEGG, GOID_BUILDING_SPIDER_TALLEGG };
	GameObjectID selectedID = eggIDs[rand() % 3];

	Building* eggObj = objectManager->CreateBuilding(selectedID, ex, ey);
	if (eggObj)
	{
		SpiderEgg* egg = dynamic_cast<SpiderEgg*>(eggObj);
		if (egg)
		{
			// 보스가 생성한 알은 주기적으로 거미를 스폰하도록 설정
			egg->SetPeriodicSpawn(true, 5.0f);
		}
	}
}

void Boss_SpiderQueen::OnPoopEnd()
{
	if (m_state != (int)SpiderQueenState::POOP_LOOP) return;
	HandleAttackEndSuperArmor();
	ChangeState((int)SpiderQueenState::CHASE);
}

void Boss_SpiderQueen::OnHitEnd()
{
	if (m_state != (int)SpiderQueenState::HIT) return;
	ChangeState((int)SpiderQueenState::IDLE);
}

void Boss_SpiderQueen::OnCocoonPreEnd()
{
	ChangeState((int)SpiderQueenState::COCOON);
}

void Boss_SpiderQueen::OnBirthEnd()
{
	ChangeState((int)SpiderQueenState::IDLE);
	if (m_spawnOutFxAnimator) m_spawnOutFxAnimator->SetActive(false);
}

bool Boss_SpiderQueen::OnInteraction(GameObject* obj) { return Entity::OnInteraction(obj); }

void Boss_SpiderQueen::Damaged(int damage)
{
	// 진입/탈출 연출 중 무적 처리
	if (m_state == (int)SpiderQueenState::COCOON_PRE || m_state == (int)SpiderQueenState::BIRTH)
		return;

	if (m_state == (int)SpiderQueenState::COCOON || m_state == (int)SpiderQueenState::COCOON_HIT)
	{
		// 고치 상태에서 피격 시 연출 및 시간 단축
		if (m_state != (int)SpiderQueenState::COCOON_HIT)
		{
			ChangeState((int)SpiderQueenState::COCOON_HIT);
		}

		// 피격당 고치 지속 시간 0.5초 단축 (타이머를 증가시켜 종료 시점에 빨리 도달하게 함)
		m_cocoonTimer += 0.5f;

		// 피격 시 일정 확률로 거미 소환 (고치 보호 로직)
		if (m_spawnOnHitCooldown <= 0.0f)
		{
			SummonSpider(1);
			m_spawnOnHitCooldown = 1.0f;
		}
		return;
	}

	Monster::Damaged(damage);
	if (IsDead())
	{
		m_hp = 0; ChangeState((int)SpiderQueenState::DEATH); m_isDead = true;
		OutputDebugStringW(L"Boss_SpiderQueen: 보스가 처치되었습니다\n");
		return;
	}

	// HP 50% 이하일 때 고치 페이즈 발동 (1회)
	if (!m_hasTriggeredCocoon && m_hp <= m_maxHp * 0.5f)
	{
		StartCocoonPhase();
		return;
	}

	if (m_hp <= m_maxHp * 0.5f && m_bossPhase == 1)
	{
		m_bossPhase = 2; OutputDebugStringW(L"Boss_SpiderQueen: 보스 페이즈가 2단계로 전환!\n");
	}

	if (CheckSuperArmorHit()) return;

	ChangeState((int)SpiderQueenState::HIT);
}

void Boss_SpiderQueen::StartCocoonPhase()
{
	m_hasTriggeredCocoon = true;
	m_cocoonTimer = 0.0f;
	m_healTickTimer = 0.0f;
	m_isHealing = true;

	ChangeState((int)SpiderQueenState::COCOON_PRE);

	// 고치 진입 시 거미 소환
	SummonSpider();

	if (m_healFxAnimator) {
		m_healFxAnimator->SetActive(true);
		// m_healFxAnimator->SetState(0, DIR_DOWN);
	}

	OutputDebugStringW(L"Boss_SpiderQueen: 고치 상태 돌입! 거미를 소환하고 회복을 시작합니다.\n");
}

void Boss_SpiderQueen::EndCocoonPhase()
{
	m_isHealing = false;
	if (m_healFxAnimator) m_healFxAnimator->SetActive(false);

	ChangeState((int)SpiderQueenState::BIRTH);

	if (m_spawnOutFxAnimator) {
		m_spawnOutFxAnimator->SetActive(true);
		// m_spawnOutFxAnimator->SetState(0, DIR_DOWN);
	}

	OutputDebugStringW(L"Boss_SpiderQueen: 고치에서 나옵니다!\n");
}

void Boss_SpiderQueen::SummonSpider(int count)
{
	ObjectManager* objMgr = ObjectManager::GetInstance();
	if (!objMgr || !transform) return;

	int spawnCount = (count > 0) ? count : (3 + (rand() % 3));
	float spawnRadius = 150.0f;

	for (int i = 0; i < spawnCount; ++i)
	{
		float angle = (rand() / (float)RAND_MAX) * 6.283185f;
		float dist = (rand() / (float)RAND_MAX) * spawnRadius;
		float sx = transform->GetX() + cosf(angle) * dist;
		float sy = transform->GetY() + sinf(angle) * dist;

		GameObjectID spiderID = (rand() % 2 == 0) ? GOID_MONSTER_SPIDER : GOID_MONSTER_WARRIOR_SPIDER;

		Entity* spiderObj = objMgr->CreateEntity(spiderID, sx, sy);
		if (spiderObj)
		{
			Spider* spider = dynamic_cast<Spider*>(spiderObj);
			if (spider)
			{
				// 생성된 위치가 맵 밖일 경우를 대비해 보정
				spider->ClampPositionToMapBounds();

				// 플레이어를 즉시 타겟팅하여 추격하게 함
				Player* player = objMgr->GetPlayer();
				if (player) spider->SetAggroTarget(player);
			}
		}
	}
	OutputDebugStringW((L"Boss_SpiderQueen: 거미 " + std::to_wstring(spawnCount) + L"마리 소환 완료\n").c_str());
}
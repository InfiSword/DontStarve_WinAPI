#include "99_Default/pch.h"
#include "Boss_SpiderQueen.h"
#include "Spider.h"
#include "../Player/Player.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../../01_Manager/RenderManager/RenderManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/GameProgressManager/GameProgressManager.h"
#include "../../../01_Manager/SceneManager/SceneManager.h"
#include "../../../03_Animation/Animator.h"
#include "../../../03_Animation/AnimationClip.h"
#include "../../../03_Animation/SpriteSheet.h"
#include "../../Component/Transform/Transform.h"
#include "../../Component/Collider/BoxCollider.h"

Boss_SpiderQueen::Boss_SpiderQueen(GameObjectID id, float x, float y, float pivotX, float pivotY, Direction dir,
                                   const std::wstring& baseDir, const std::wstring& imageName, ColliderType colliderType)
	: Monster(id, x, y, pivotX, pivotY, dir, baseDir, imageName, colliderType)
	, m_bossPhase(1)
	, m_specialAttackCooldown(0.0f)
	, m_idleTimer(0.0f)
	, m_idleDuration(2.0f)
	, m_attackCollider(nullptr)
    , m_hasTriggeredCocoon(false)
    , m_cocoonTimer(0.0f)
    , m_healTickTimer(0.0f)
    , m_isHealing(false)
    , m_healFxAnimator(nullptr)
    , m_spawnOutFxAnimator(nullptr)
{
	m_hp = 1000;
	m_maxHp = m_hp;
	m_type = GO_TYPE_MONSTER;
	m_walkSpeed = 50.0f;
	m_attackRange = 70.0f;
	m_attackCooldown = 1.5f;
	m_attackHitFrame = 28;
	m_damage = 25;
	m_attackBoxWidth = 70;
	m_attackBoxHeight = 50;
}

Boss_SpiderQueen::~Boss_SpiderQueen() {}

void Boss_SpiderQueen::Init()
{
	Monster::Init();
	SetupAggro(AggroType::ALWAYS, 0.0f, 0.0f);
	SetupAttackBox(m_attackBoxWidth, m_attackBoxHeight);

	ChangeState((int)SpiderQueenState::IDLE);
	m_bossPhase = 1;
	m_specialAttackCooldown = 0.0f;

	if (!m_animator) m_animator = AddComponent<Animator>();
    
    // FX Animator 추가
    if (!m_healFxAnimator) m_healFxAnimator = AddComponent<Animator>();
    if (!m_spawnOutFxAnimator) m_spawnOutFxAnimator = AddComponent<Animator>();

	ResourceManager* pRM = ResourceManager::GetInstance();
	const ResourcePathUtils::ObjectResourceDef* objData = pRM->GetObjectResourceInfo(GOID_MONSTER_QUEEN_SPIDER);
	if (objData) {
		std::wstring base = objData->baseDir + L"\\";
		float px = transform->GetPivotX();
		float py = transform->GetPivotY();

		std::wstring idlePath = base + L"Queen_spider_queen_idle_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) m_animator->RegisterAnimation((int)SpiderQueenState::IDLE, (Direction)dir, idlePath, 0, 0, 4, 50, px, py, true, 0.02f);

		std::wstring walkPath = base + L"Walk_spider_queen_walk_loop_side.png";
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) m_animator->RegisterAnimation((int)SpiderQueenState::CHASE, (Direction)dir, walkPath, 0, 0, 7, 65, px, py, true, 0.02f);

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
			m_animator->RegisterAnimation((int)SpiderQueenState::BIRTH, (Direction)dir, base + L"Queen_spider_queen_enter.png", 0, 0, 7, 65, px, py, false, 0.02f);
			AnimationClip* clip = m_animator->GetAnimationClip((int)SpiderQueenState::BIRTH, (Direction)dir);
			if (clip) {
				clip->AddEventFrame(64, L"birth_end");
				clip->SetEventCallback([this](int frameIndex, const std::wstring& eventName) {
					if (eventName == L"birth_end") this->OnBirthEnd();
					});
			}
		}
		
		// COCOON_PRE 애니메이션
		for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) {
			m_animator->RegisterAnimation((int)SpiderQueenState::COCOON_PRE, (Direction)dir, base + L"Cocoon_spider_queen_cocoon.png", 0, 0, 7, 45, px, py, false, 0.02f);
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
        for (int dir = DIR_UP; dir <= DIR_RIGHT; dir++) m_animator->RegisterAnimation((int)SpiderQueenState::TAUNT, (Direction)dir, base + L"Queen_spider_queen_taunt.png", 0, 0, 7, 50, px, py, false, 0.03f);

        // FX 등록
        // 1. Heal Buff FX (루프)
        m_healFxAnimator->RegisterAnimation(0, DIR_DOWN, base + L"Queen_heal_fx_heal_buff.png", 0, 0, 7, 65, px, py, true, 0.02f);
        
        // 2. Spawn Out FX (단발)
        m_spawnOutFxAnimator->RegisterAnimation(0, DIR_DOWN, base + L"FX_splash_spiderweb_idle.png", 0, 0, 7, 45, px, py, false, 0.02f);
	}
	m_animator->SetState((int)m_state, this->transform->GetDirection());
    m_healFxAnimator->SetActive(false);
    m_spawnOutFxAnimator->SetActive(false);

	m_attackCollider = AddComponent<BoxCollider>();
	if (m_attackCollider) {
		UpdateAttackBoxByDirection(DIR_DOWN);
		m_attackCollider->SetColliderEnabled(false);
	}
}

void Boss_SpiderQueen::UpdateAI(float deltaTime)
{
	if (!IsEnabled() || !transform || !m_animator) return;

	switch ((SpiderQueenState)m_state)
	{
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

	default:
		// 항상 추격 패턴 적용 (IDLE/CHASE/ATTACK 등)
		UpdateAI_AlwaysChase(deltaTime, (int)SpiderQueenState::CHASE, (int)SpiderQueenState::ATTACK, (int)SpiderQueenState::IDLE);
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
        break;
	case SpiderQueenState::CHASE: MoveTowardPlayer(deltaTime, m_walkSpeed, (int)SpiderQueenState::CHASE, (int)SpiderQueenState::IDLE); break;
	case SpiderQueenState::IDLE: m_animator->SetState((int)SpiderQueenState::IDLE, transform->GetDirection()); break;
	}
}

void Boss_SpiderQueen::OnAttackHit() { if (m_state == (int)SpiderQueenState::ATTACK) ProcessAttackHit(m_damage); }

void Boss_SpiderQueen::OnAttackEnd() { if (m_state == (int)SpiderQueenState::ATTACK) ChangeState((int)SpiderQueenState::CHASE); }	

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
		return;
	}

	Entity::Damaged(damage);
	if (!IsDead()) ChangeState((int)SpiderQueenState::HIT);

    // HP 50% 이하일 때 고치 페이즈 발동 (1회)
    if (!m_hasTriggeredCocoon && m_hp <= m_maxHp * 0.5f)
    {
        StartCocoonPhase();
        return; 
    }

	if (m_hp <= m_maxHp * 0.5f && m_bossPhase == 1) { m_bossPhase = 2; OutputDebugStringW(L"Boss_SpiderQueen: 보스 페이즈가 2단계로 전환!\n"); }

	if (m_hp <= 0) {
		m_hp = 0; ChangeState((int)SpiderQueenState::DEATH); m_isDead = true;
		OutputDebugStringW(L"Boss_SpiderQueen: 보스가 처치되었습니다\n");
	}

	if (!IsDead() && IsEnabled()) m_attackTarget = ObjectManager::GetInstance()->GetPlayer();
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
        m_healFxAnimator->SetState(0, DIR_DOWN);
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
        m_spawnOutFxAnimator->SetState(0, DIR_DOWN);
    }

    OutputDebugStringW(L"Boss_SpiderQueen: 고치에서 나옵니다!\n");
}

void Boss_SpiderQueen::SummonSpider()
{
    ObjectManager* objMgr = ObjectManager::GetInstance();
    if (!objMgr || !transform) return;

    int spawnCount = 3 + (rand() % 3); 
    float spawnRadius = 150.0f;

    for (int i = 0; i < spawnCount; ++i)
    {
        float angle = (rand() / (float)RAND_MAX) * 6.283185f;
        float dist = (rand() / (float)RAND_MAX) * spawnRadius;
        float sx = transform->GetX() + cosf(angle) * dist;
        float sy = transform->GetY() + sinf(angle) * dist;

        GameObjectID spiderID = (rand() % 2 == 0) ? GOID_MONSTER_SPIDER : GOID_MONSTER_WARRIOR_SPIDER;
        
        GameObject* spiderObj = objMgr->CreateGameObject(spiderID, sx, sy, nullptr, true);
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

void Boss_SpiderQueen::RenderDebugOverlay()
{
	if (!transform) return;
	CameraManager* cameraManager = CameraManager::GetInstance();
	RenderManager* renderManager = RenderManager::GetInstance();
	if (!cameraManager || !renderManager) return;

	Gdiplus::PointF screenCenter = cameraManager->WorldToScreen(transform->GetX(), transform->GetY());
	float rWander = m_wanderRadius;
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - rWander, screenCenter.Y - rWander, rWander * 2.0f, rWander * 2.0f), Gdiplus::Color(100, 200, 100, 255), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);
	renderManager->AddDrawEllipseCommand(Gdiplus::RectF(screenCenter.X - m_aggroRadius, screenCenter.Y - m_aggroRadius, m_aggroRadius * 2.0f, m_aggroRadius * 2.0f), Gdiplus::Color(255, 255, 0), 1.0f, LAYER_DEBUG_OVERLAY, 9998.0f);

	if (m_state == (int)SpiderQueenState::ATTACK && m_attackCollider) {
		UpdateAttackBoxByDirection(transform->GetDirection());
		RECT worldRect = m_attackCollider->GetWorldBoundingBox();
		Gdiplus::PointF topLeft = cameraManager->WorldToScreen((float)worldRect.left, (float)worldRect.top);
		Gdiplus::PointF bottomRight = cameraManager->WorldToScreen((float)worldRect.right, (float)worldRect.bottom);
		renderManager->AddDrawRectCommand(Gdiplus::RectF(topLeft.X, topLeft.Y, bottomRight.X - topLeft.X, bottomRight.Y - topLeft.Y), Gdiplus::Color(255, 0, 0), 2.0f, LAYER_DEBUG_OVERLAY, 9999.0f);
	}
}

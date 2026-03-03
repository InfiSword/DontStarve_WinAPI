#include "99_Default/pch.h"
#include "SpiderEgg.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../03_Animation/Animator.h"
#include "../../03_Animation/AnimationClip.h"
#include "../../02_GameObject/Component/Sprite/SpriteRenderer.h"
#include "../../02_GameObject/Component/Transform/Transform.h"
#include "../../02_GameObject/Component/Collider/BoxCollider.h"
#include "../Entity/Monster/Spider.h"
#include "../Entity/Player/Player.h"
#include "../Item/Tool/Tool.h"

SpiderEgg::SpiderEgg(GameObjectID id, float x, float y, float pivotX, float pivotY,
	Direction _dir, const std::wstring& resourcePath,
	const std::wstring& imageName, int hp)
	: Building(id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, hp)
	, m_eggStage(EggStage::Sac)
	, m_isPlayingGrowth(false)
	, m_isPlayingHit(false)
	, m_spawnRadius(200.0f)
{
	if (id == GOID_BUILDING_SPIDER_SACEGG) m_eggStage = EggStage::Sac;
	else if (id == GOID_BUILDING_SPIDER_SMALLEGG) m_eggStage = EggStage::Small;
	else if (id == GOID_BUILDING_SPIDER_NORMALEGG) m_eggStage = EggStage::Medium;
	else if (id == GOID_BUILDING_SPIDER_TALLEGG) m_eggStage = EggStage::Large;
	OutputDebugStringW(L"SpiderEgg: 생성자 호출\n");
}

SpiderEgg::~SpiderEgg()
{
	OutputDebugStringW(L"SpiderEgg: 소멸자 호출\n");
}

void SpiderEgg::Init()
{
	Building::Init();

	Transform* transform = GetComponent<Transform>();
	if (!transform) {
		OutputDebugStringW(L"SpiderEgg: Transform component not found!\n");
		return;
	}

	if (!m_animator) {
		m_animator = AddComponent<Animator>();
	}
	if (!m_animator || !transform) return;

	ResourceManager* pRM = ResourceManager::GetInstance();
	const ResourcePathUtils::ObjectResourceDef* data = pRM->GetObjectResourceInfo(m_id);
	if (!data) return;

	std::wstring base = data->baseDir;
	if (!base.empty() && base.back() != L'\\' && base.back() != L'/') {
		base += L"\\";
	}

	float px = transform->GetPivotX();
	float py = transform->GetPivotY();

	// Idle (loop=true). 프레임 레이아웃: 0,0 = 자동 계산
	m_animator->RegisterAnimation(EGG_STATE_IDLE_SMALL, DIR_DOWN, base + L"Egg_spider_cocoon_small_Idle.png",
		0, 0, 1, 1, px, py, true, 0.04f);
	m_animator->RegisterAnimation(EGG_STATE_IDLE_MEDIUM, DIR_DOWN, base + L"Egg_spider_cocoon_medium_Idle.png",
		0, 0, 1, 1, px, py, true, 0.04f);
	m_animator->RegisterAnimation(EGG_STATE_IDLE_LARGE, DIR_DOWN, base + L"Egg_spider_cocoon_large_Idle.png",
		0, 0, 1, 1, px, py, true, 0.04f);

	// Hit (loop=false)
	m_animator->RegisterAnimation(EGG_STATE_HIT_SMALL, DIR_DOWN, base + L"Hit\\Egg_spider_cocoon_cocoon_small_hit.png",
		0, 0, 7, 33, px, py, false, 0.04f);
	m_animator->RegisterAnimation(EGG_STATE_HIT_MEDIUM, DIR_DOWN, base + L"Hit\\Egg_spider_cocoon_cocoon_medium_hit.png",
		0, 0, 7, 35, px, py, false, 0.04f);
	m_animator->RegisterAnimation(EGG_STATE_HIT_LARGE, DIR_DOWN, base + L"Hit\\Egg_spider_cocoon_cocoon_large_hit.png",
		0, 0, 7, 43, px, py, false, 0.04f);

	// Grow (loop=false)
	m_animator->RegisterAnimation(EGG_STATE_GROW_SAC_TO_SMALL, DIR_DOWN, base + L"Grow\\Egg_spider_cocoon_grow_sac_to_small.png",
		0, 0, 7, 29, px, py, false, 0.04f);
	m_animator->RegisterAnimation(EGG_STATE_GROW_SMALL_TO_MEDIUM, DIR_DOWN, base + L"Grow\\Egg_spider_cocoon_grow_small_to_medium.png",
		0, 0, 7, 43, px, py, false, 0.04f);
	m_animator->RegisterAnimation(EGG_STATE_GROW_MEDIUM_TO_LARGE, DIR_DOWN, base + L"Grow\\Egg_spider_cocoon_grow_medium_to_large.png",
		0, 0, 7, 43, px, py, false, 0.04f);

	// Burst (loop=false)
	m_animator->RegisterAnimation(EGG_STATE_BURST_LARGE, DIR_DOWN, base + L"Burst\\Egg_spider_cocoon_cocoon_large_burst.png",
		0, 0, 7, 43, px, py, false, 0.04f);

	// 현재 단계에 맞는 Idle로 시작
	int idleState = EGG_STATE_IDLE_SMALL;
	if (m_eggStage == EggStage::Medium) idleState = EGG_STATE_IDLE_MEDIUM;
	else if (m_eggStage == EggStage::Large) idleState = EGG_STATE_IDLE_LARGE;
	else if (m_eggStage == EggStage::Sac) idleState = EGG_STATE_IDLE_SMALL;
	m_animator->SetState(idleState, transform->GetDirection());

	// Sac 상태: 아무것도 렌더링하지 않음.
	if (spriteRenderer) {
		if (m_eggStage == EggStage::Sac) spriteRenderer->SetActive(false);
		else spriteRenderer->SetActive(true);
	}

	OutputDebugStringW(L"SpiderEgg: Init 완료\n");
}

void SpiderEgg::LateInit()
{
	OutputDebugStringW(L"SpiderEgg: LateInit 호출\n");
}

void SpiderEgg::Update(float deltaTime)
{
	Building::Update(deltaTime);

	// Sac 상태: 성장 애니 재생 중이 아니면 SpriteRenderer 비활성(아무것도 안 그림). Grow() 시에만 표시.
	if (spriteRenderer) {
		bool visible = (m_eggStage != EggStage::Sac) || m_isPlayingGrowth;
		if(!visible) return;
	}

	Transform* transform = GetComponent<Transform>();
	if (!m_animator || !transform) return;

	// 성장 애니메이션 재생 종료 시 단계 전환 후 Idle로 복귀
	if (m_isPlayingGrowth && m_animator->IsAnimationDone()) {
		if (m_eggStage == EggStage::Sac) m_eggStage = EggStage::Small;
		else if (m_eggStage == EggStage::Small) m_eggStage = EggStage::Medium;
		else if (m_eggStage == EggStage::Medium) m_eggStage = EggStage::Large;
		m_isPlayingGrowth = false;
		int idleState = EGG_STATE_IDLE_SMALL;
		if (m_eggStage == EggStage::Medium) idleState = EGG_STATE_IDLE_MEDIUM;
		else if (m_eggStage == EggStage::Large) idleState = EGG_STATE_IDLE_LARGE;
		m_animator->SetState(idleState, transform->GetDirection());
	}

	// Hit 애니메이션 재생 종료 시 Idle로 복귀
	if (m_isPlayingHit && m_animator->IsAnimationDone()) {
		m_isPlayingHit = false;
		int idleState = EGG_STATE_IDLE_SMALL;
		if (m_eggStage == EggStage::Medium) idleState = EGG_STATE_IDLE_MEDIUM;
		else if (m_eggStage == EggStage::Large) idleState = EGG_STATE_IDLE_LARGE;
		m_animator->SetState(idleState, transform->GetDirection());
	}
}

void SpiderEgg::LateUpdate()
{
	Building::LateUpdate();
}

void SpiderEgg::Release()
{
	m_animator = nullptr;
	Building::Release();
}

bool SpiderEgg::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}

void SpiderEgg::Damaged(int damage)
{
	OutputDebugStringW((L"SpiderEgg: Damaged - 데미지: " + std::to_wstring(damage) + L"\n").c_str());

	// 거미 스폰 (피격 시)
	SpawnSpiders();

	m_hp -= damage;
	if (m_hp <= 0) {
		m_hp = 0;
		m_buildingState = BuildingState::DESTROYED;
		OutputDebugStringW(L"SpiderEgg: 파괴됨\n");
	}
	else {
		if (m_hp <= m_maxHp / 2) {
			m_buildingState = BuildingState::DAMAGED;
			OutputDebugStringW(L"SpiderEgg: 손상됨\n");
		}
		
		// Hit 애니메이션 재생
		if (m_animator) {
			Transform* transform = GetComponent<Transform>();
			if (transform) {
				m_isPlayingHit = true;
				int hitState = EGG_STATE_HIT_SMALL;
				if (m_eggStage == EggStage::Medium) hitState = EGG_STATE_HIT_MEDIUM;
				else if (m_eggStage == EggStage::Large) hitState = EGG_STATE_HIT_LARGE;

				// 만약 이미 Hit 애니메이션이 재생 중이라면 처음부터 다시 재생되도록 처리
				if (m_animator->GetClip() == m_animator->GetAnimationClip(hitState, transform->GetDirection()))
				{
					m_animator->Stop();
					m_animator->Play();
				}
				else
				{
					m_animator->SetState(hitState, transform->GetDirection());
				}
			}
		}
	}
}

void SpiderEgg::SpawnSpiders()
{
	Transform* eggTransform = GetComponent<Transform>();
	if (!eggTransform) return;

	float eggX = eggTransform->GetX();
	float eggY = eggTransform->GetY();

	// 거미집 단계에 따라 스폰 수 결정
	int spawnCount = 0;
	if (m_eggStage == EggStage::Small) spawnCount = 1;
	else if (m_eggStage == EggStage::Medium) spawnCount = 2;
	else if (m_eggStage == EggStage::Large) spawnCount = 3;

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;

	for (int i = 0; i < spawnCount; ++i)
	{
		// 거미집 주변 무작위 위치 계산
		float angle = (rand() / (float)RAND_MAX) * 6.283185307f;  // 0 ~ 2*PI
		float distance = 50.0f + (rand() / (float)RAND_MAX) * 100.0f;  // 50 ~ 150 픽셀 거리
		float spawnX = eggX + cosf(angle) * distance;
		float spawnY = eggY + sinf(angle) * distance;

		// 거미 생성 (Large 단계면 전사 거미 스폰 확률 추가)
		GameObjectID spiderID = GOID_MONSTER_SPIDER;
		if (m_eggStage == EggStage::Large && (rand() % 2 == 0)) {
			spiderID = GOID_MONSTER_WARRIOR_SPIDER;
		}

		GameObject* spiderObj = objectManager->CreateGameObject(spiderID, spawnX, spawnY, nullptr, true);
		if (spiderObj)
		{
			Spider* spider = dynamic_cast<Spider*>(spiderObj);
			if (spider)
			{
				// 거미에게 소속 거미집 설정
				spider->SetHomeEgg(this, m_spawnRadius);

				// 플레이어 발견 시 타겟팅 (Egg가 공격받았을 때 나오는 거미는 플레이어를 즉시 타겟팅)
				GameObject* player = objectManager->GetPlayer();
				if (player) {
					spider->SetAggroTarget(player);
				}

				OutputDebugStringW((L"SpiderEgg: 거미 스폰 성공 - 위치: (" + 
					std::to_wstring(spawnX) + L", " + std::to_wstring(spawnY) + L")\n").c_str());
			}
		}
	}

	OutputDebugStringW((L"SpiderEgg: 거미 " + std::to_wstring(spawnCount) + L"마리 스폰 완료\n").c_str());
}

void SpiderEgg::Grow()
{
	if (!m_animator) return;
	Transform* transform = GetComponent<Transform>();
	spriteRenderer->SetActive(true);

	if (m_eggStage == EggStage::Sac) {
		m_isPlayingGrowth = true;
		m_animator->SetState(EGG_STATE_GROW_SAC_TO_SMALL, transform->GetDirection());
	}
	else if (m_eggStage == EggStage::Small) {
		m_isPlayingGrowth = true;
		m_animator->SetState(EGG_STATE_GROW_SMALL_TO_MEDIUM, transform->GetDirection());
	}
	else if (m_eggStage == EggStage::Medium) {
		m_isPlayingGrowth = true;
		m_animator->SetState(EGG_STATE_GROW_MEDIUM_TO_LARGE, transform->GetDirection());
	}
}

void SpiderEgg::SetTimeState(BuildingState buildingState)
{
	m_buildingState = buildingState;
}

BuildingState SpiderEgg::GetTimeState() const
{
	return m_buildingState;
}

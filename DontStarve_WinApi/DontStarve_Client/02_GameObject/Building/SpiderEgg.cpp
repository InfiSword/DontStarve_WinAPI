#include "99_Default/pch.h"
#include "SpiderEgg.h"
#include "../../01_Manager/CameraManager/CameraManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../01_Manager/DataManager/DataManager.h"
#include "../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../01_Manager/SoundManager/SoundManager.h"
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
	: Building(id, x, y, pivotX, pivotY, _dir, resourcePath, imageName, hp, COLLIDER_BOX, true, true)
	, m_eggStage(EggStage::Sac)
	, m_isPlayingHit(false)
	, m_spawnRadius(200.0f)
	, m_amountOfSpidersToSpawn(0)
	, m_remainingSpiders(0)
	, m_totalSpawnedCount(0)
	, m_disappearTimer(2.0f)
	, m_isDisappearing(false)
	, m_isPeriodicSpawner(false)
	, m_periodicSpawnTimer(0.0f)
	, m_periodicSpawnInterval(5.0f)
	, m_baseX(0.0f)
	, m_baseY(0.0f)
	, m_shakeDuration(0.5f)
	, m_shakeAmount(14.0f)
	, m_shakeSpeed(40.0f)
	, m_isShaking(false)
{

	switch (id)
	{
	case GOID_BUILDING_SPIDER_SACEGG:
		m_eggStage = EggStage::Sac;
		m_amountOfSpidersToSpawn = 0;
		break;
	case GOID_BUILDING_SPIDER_SMALLEGG:
		m_eggStage = EggStage::Small;
		m_hp = 100;
		m_maxHp = m_hp;
		m_amountOfSpidersToSpawn = 1;
		break;
	case GOID_BUILDING_SPIDER_NORMALEGG:
		m_eggStage = EggStage::Medium;
		m_hp = 200;
		m_maxHp = m_hp;
		m_amountOfSpidersToSpawn = 2;
		break;
	case GOID_BUILDING_SPIDER_TALLEGG:
		m_eggStage = EggStage::Large;
		m_hp = 300;
		m_maxHp = m_hp;
		m_amountOfSpidersToSpawn = 3;
		break;
	default:
		break;
	}
	m_remainingSpiders = m_amountOfSpidersToSpawn;
	OutputDebugStringW(L"SpiderEgg: 생성자 호출\n");
}

SpiderEgg::~SpiderEgg()
{
	OutputDebugStringW(L"SpiderEgg: 소멸자 호출\n");
}

void SpiderEgg::Init()
{
	Building::Init();

	m_animator = AddComponent<Animator>(m_spriteRenderer);

	DataManager* pRM = DataManager::GetInstance();
	const ResourcePathUtils::ObjectResourceDef* data = pRM->GetObjectResourceInfo(m_id);


	if (!data) return;

	std::wstring base = data->baseDir;
	if (!base.empty() && base.back() != L'\\' && base.back() != L'/') {
		base += L"\\";
	}

	float px = data->pivotX;
	float py = data->pivotY;

	// Idle (loop=true)
	m_animator->RegisterAnimation(EGG_STATE_IDLE_SMALL, DIR_DOWN, base + L"Egg_spider_cocoon_small_Idle.png",
		0, 0, 1, 1, px, py, true, 0.04f);
	m_animator->RegisterAnimation(EGG_STATE_IDLE_MEDIUM, DIR_DOWN, base + L"Egg_spider_cocoon_medium_Idle.png",
		0, 0, 1, 1, px, py, true, 0.04f);
	m_animator->RegisterAnimation(EGG_STATE_IDLE_LARGE, DIR_DOWN, base + L"Egg_spider_cocoon_large_Idle.png",
		0, 0, 1, 1, px, py, true, 0.04f);

	// 현재 단계에 맞는 Idle로 시작
	int idleState = EGG_STATE_IDLE_SMALL;
	if (m_eggStage == EggStage::Medium) idleState = EGG_STATE_IDLE_MEDIUM;
	else if (m_eggStage == EggStage::Large) idleState = EGG_STATE_IDLE_LARGE;
	else if (m_eggStage == EggStage::Sac) idleState = EGG_STATE_IDLE_SMALL;
	ChangeState(idleState);

	// Sac 상태: 아무것도 렌더링하지 않음.
	if (m_spriteRenderer) {
		if (m_eggStage == EggStage::Sac) m_spriteRenderer->SetActive(false);
		else m_spriteRenderer->SetActive(true);
	}

	if (m_transform)
	{
		m_baseX = m_transform->GetX();
		m_baseY = m_transform->GetY();
	}

	PreSpawnSpiders();

	OutputDebugStringW(L"SpiderEgg: Init 완료\n");
}

void SpiderEgg::LateInit()
{
}

void SpiderEgg::Update(float deltaTime)
{
	if (m_isDisappearing) {
		m_disappearTimer -= deltaTime;
		if (m_disappearTimer <= 0.0f) {
			ObjectManager::GetInstance()->RemoveGameObject(this);
			return;
		}
	}

	Building::Update(deltaTime);

	// 주기적 스폰 업데이트
	if (m_isPeriodicSpawner && !m_isDisappearing && m_buildingState != BuildingState::DESTROYED)
	{
		m_periodicSpawnTimer += deltaTime;
		if (m_periodicSpawnTimer >= m_periodicSpawnInterval)
		{
			m_periodicSpawnTimer = 0.0f;
			SpawnSpiders();
		}
	}

	// Sac 상태 가시성 처리
	if (m_spriteRenderer) {
		bool visible = (m_eggStage != EggStage::Sac) || m_isPlayingHit;
		if (m_spriteRenderer->IsEnabled() != visible) m_spriteRenderer->SetActive(visible);
	}

	if (!m_animator || !m_transform) return;

	// Hit 애니메이션 종료 처리
	if (m_isPlayingHit && m_animator->IsAnimationDone())
	{
		m_isPlayingHit = false;
		int idleState = EGG_STATE_IDLE_SMALL;
		if (m_eggStage == EggStage::Medium) idleState = EGG_STATE_IDLE_MEDIUM;
		else if (m_eggStage == EggStage::Large) idleState = EGG_STATE_IDLE_LARGE;
		ChangeState(idleState);
	}
}

void SpiderEgg::LateUpdate()
{
	Building::LateUpdate();
}

void SpiderEgg::Release()
{
	for (Spider* s : m_poolSpiders) {
		if (s) {
			ObjectManager::GetInstance()->RemoveGameObject(s);
		}
	}
	m_poolSpiders.clear();

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

	if (m_isShaking && m_transform) {
		m_transform->SetPosition(m_baseX, m_baseY);
	}
	else if (m_transform) {
		m_baseX = m_transform->GetX();
		m_baseY = m_transform->GetY();
	}

	// 거미 스폰 (남아있는 거미가 있을 때만)
	if (m_remainingSpiders > 0) {
		SpawnSpiders();
	}

	m_hp -= damage;
	if (m_hp <= 0) {
		m_hp = 0;
		m_buildingState = BuildingState::DESTROYED;

		SoundManager::GetInstance()->PlaySFX(L"Resource/Sound/SpiderSound/SpiderEggRemove.wav");

		// 파괴될 때 남은 거미가 있다면 모두 스폰
		while (m_remainingSpiders > 0) {
			SpawnSpiders();
		}

		if (m_transform) m_transform->SetPosition(m_baseX, m_baseY);
		m_isShaking = false;

		ObjectManager::GetInstance()->RemoveGameObject(this);
	}
	else {
		StopAllCoroutines();
		m_isShaking = true;

		float baseX = m_baseX;
		float baseY = m_baseY;
		float elapsed = 0.0f;
		float duration = m_shakeDuration;
		float amount = m_shakeAmount;
		float speed = m_shakeSpeed;
		Transform* tr = m_transform;

		StartCoroutine([=](float dt) mutable -> bool 
		{
			elapsed += dt;

			if (elapsed >= duration) {
				if (tr) tr->SetPosition(baseX, baseY);
				m_isShaking = false;
				return false;
			}

			if (tr) {
				float currentAmount = amount * (1.0f - (elapsed / duration));
				float offsetX = sinf(elapsed * speed) * currentAmount;
				tr->SetPosition(baseX + offsetX, baseY);
			}
			return true;
			});

		if (m_animator && !m_isPlayingHit)
		{
			m_isPlayingHit = true;
		}
	}
}

void SpiderEgg::PreSpawnSpiders()
{
	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!objectManager) return;

	while (m_poolSpiders.size() < (size_t)m_remainingSpiders) {
		GameObjectID spiderID = GOID_MONSTER_SPIDER;
		if (m_eggStage == EggStage::Large && (rand() % 100 < 30)) {
			spiderID = GOID_MONSTER_WARRIOR_SPIDER;
		}

		GameObject* spiderObj = objectManager->CreateObject(spiderID, 0.0f, 0.0f);
		if (spiderObj) {
			Spider* spider = dynamic_cast<Spider*>(spiderObj);
			if (spider) {
				spider->SetActive(false);
				spider->SetHomeEgg(this, m_spawnRadius);
				m_poolSpiders.push_back(spider);
			}
		}
	}
}

void SpiderEgg::SpawnSpiders()
{
	if (m_totalSpawnedCount >= 4) return;

	// 일반 모드에서는 남은 거미가 없으면 중단
	if (!m_isPeriodicSpawner && m_remainingSpiders <= 0) return;

	ObjectManager* objectManager = ObjectManager::GetInstance();
	if (!m_transform || !objectManager) return;

	Spider* spider = nullptr;

	// 풀에 거미가 있으면 사용 (PreSpawn 등으로 미리 생성된 경우)
	if (!m_poolSpiders.empty()) {
		spider = m_poolSpiders.back();
		m_poolSpiders.pop_back();
	}
	else {
		// 풀이 비어있으면 새로 생성 (주기적 스폰용)
		GameObjectID spiderID = GOID_MONSTER_SPIDER;
		if (m_eggStage == EggStage::Large && (rand() % 100 < 30)) spiderID = GOID_MONSTER_WARRIOR_SPIDER;
		else if (m_eggStage == EggStage::Medium && (rand() % 100 < 15)) spiderID = GOID_MONSTER_WARRIOR_SPIDER;

		GameObject* spiderObj = objectManager->CreateObject(spiderID, m_transform->GetX(), m_transform->GetY());
		if (spiderObj) {
			spider = dynamic_cast<Spider*>(spiderObj);
		}
	}

	if (!spider) return;

	float angle = (rand() / (float)RAND_MAX) * 6.283185307f;
	float dist = 60.0f + (rand() / (float)RAND_MAX) * 60.0f;
	float sx = m_transform->GetX() + cosf(angle) * dist;
	float sy = m_transform->GetY() + sinf(angle) * dist;

	Transform* spiderTr = spider->GetComponent<Transform>();
	if (spiderTr) {
		spiderTr->SetPosition(sx, sy);
	}

	spider->SetActive(true);
	spider->ClampPositionToMapBounds();

	GameObject* player = objectManager->GetPlayer();
	if (player) spider->SetAggroTarget(player);

	if (m_remainingSpiders > 0) m_remainingSpiders--;
	m_totalSpawnedCount++;

	if (m_totalSpawnedCount >= 4) {
		m_isDisappearing = true;
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

void SpiderEgg::SetEggStage(EggStage stage)
{
	m_eggStage = stage;

	// 거미 수 업데이트
	switch (m_eggStage) {
	case EggStage::Small: m_remainingSpiders = 1; break;
	case EggStage::Medium: m_remainingSpiders = 2; break;
	case EggStage::Large: m_remainingSpiders = 3; break;
	case EggStage::Sac: m_remainingSpiders = 0; break;
	}

	// 애니메이터 상태 업데이트 (이미 초기화된 경우)
	if (m_animator) {
		int idleState = EGG_STATE_IDLE_SMALL;
		if (m_eggStage == EggStage::Medium) idleState = EGG_STATE_IDLE_MEDIUM;
		else if (m_eggStage == EggStage::Large) idleState = EGG_STATE_IDLE_LARGE;
		ChangeState(idleState);
	}

	PreSpawnSpiders();
}

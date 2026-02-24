#include "99_Default/pch.h"
#include "../../../01_Manager/CameraManager/CameraManager.h"
#include "../../../01_Manager/ObjectManager/ObjectManager.h"
#include "../../../01_Manager/ResourceManager/ResourceManager.h"
#include "../../Component/Sprite/SpriteRenderer.h"
#include "../../Component/Transform/Transform.h"
#include "Rock.h"

Rock::Rock(GameObjectID id, float x, float y, float pivotX, float pivotY, const std::wstring& resourcePath, const std::wstring& imageName)
	: Entity(GOBJ_NATURAL_ENVIR, id, x, y, pivotX, pivotY, DIR_DOWN, resourcePath, imageName, true, true),
	m_hp(120), m_state(RockState::ROCK_INTACT)
{
	if (id == GOID_NORMAL_ROCK) {
		maxHp = 120;
		m_hp = 120;
		SetDropItem(GOID_ITEM_NORMAL_ROCK, 2);
	}
	else if (id == GOID_GOLD_ROCK) {
		maxHp = 200;
		m_hp = 200;
		SetDropItem(GOID_ITEM_GOLD_ROCK, 1);
	}
	else {
		maxHp = 120;
		m_hp = 120;
	}
}

Rock::~Rock() {}

void Rock::Init()
{
	Entity::Init();

	const ResourcePathUtils::ObjectResourceDef* objData = ResourceManager::GetInstance()->GetObjectResourceInfo(GetID());
	if (!objData || objData->imageName.empty()) return;

	const std::wstring& baseDir = objData->baseDir;
	const std::wstring& imageName = objData->imageName;
	size_t pos = imageName.find_last_of(L'-');
	if (pos == std::wstring::npos) return;

	std::wstring prefix = imageName.substr(0, pos + 1);
	std::wstring fileName0 = prefix + L"0.png";
	std::wstring fileName1 = prefix + L"1.png";
	std::wstring fileName2 = prefix + L"2.png";

	ResourceManager* pRM = ResourceManager::GetInstance();
	std::wstring path0 = ResourcePathUtils::BuildResourcePath(baseDir, fileName0);
	std::wstring path1 = ResourcePathUtils::BuildResourcePath(baseDir, fileName1);
	std::wstring path2 = ResourcePathUtils::BuildResourcePath(baseDir, fileName2);

	m_spriteIntact = pRM->LoadSprite(path0);
	m_spriteCracked = pRM->LoadSprite(path1);
	m_spriteBroken = pRM->LoadSprite(path2);

	if (spriteRenderer && m_spriteIntact)
		spriteRenderer->SetSprite(m_spriteIntact);
}

void Rock::LateInit()
{
}

void Rock::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	Entity::Update(deltaTime);
}

void Rock::LateUpdate()
{
}

void Rock::Release()
{
	m_spriteIntact.reset();
	m_spriteCracked.reset();
	m_spriteBroken.reset();

	Entity::Release();
}

bool Rock::OnInteraction(GameObject* obj)
{
	return Entity::OnInteraction(obj);
}


void Rock::Damaged(int damage)
{
	m_hp -= damage;

	if (m_hp <= 0) {
		m_state = ROCK_BROKEN;
		m_isDead = true;
		Die();
		return;
	}

	int threshold70 = maxHp * 70 / 100;
	int threshold30 = maxHp * 30 / 100;
	if (m_hp <= threshold30)
		m_state = ROCK_BROKEN;
	else if (m_hp <= threshold70)
		m_state = ROCK_CRACKED;
	else
		m_state = ROCK_INTACT;

	if (spriteRenderer) {
		if (m_state == ROCK_INTACT && m_spriteIntact)
			spriteRenderer->SetSprite(m_spriteIntact);
		else if (m_state == ROCK_CRACKED && m_spriteCracked)
			spriteRenderer->SetSprite(m_spriteCracked);
		else if (m_state == ROCK_BROKEN && m_spriteBroken)
			spriteRenderer->SetSprite(m_spriteBroken);
	}
}

void Rock::Die()
{
	float tx = transform ? transform->GetX() : 0.0f;
	float ty = transform ? transform->GetY() : 0.0f;
	ObjectManager* objMgr = ObjectManager::GetInstance();
	const float spreadRadius = 20.0f;

	GameObjectID myID = GetID();
	if (myID == GOID_GOLD_ROCK) {
		// 금 바위: 금 1개 + 일반 돌 1개
		float angle0 = 0.0f;
		float angle1 = 3.14159f;
		float ox0 = cosf(angle0) * spreadRadius;
		float oy0 = sinf(angle0) * spreadRadius;
		float ox1 = cosf(angle1) * spreadRadius;
		float oy1 = sinf(angle1) * spreadRadius;
		objMgr->CreateGameObject(GOID_ITEM_GOLD_ROCK, tx + ox0, ty + oy0, nullptr, true);
		objMgr->CreateGameObject(GOID_ITEM_NORMAL_ROCK, tx + ox1, ty + oy1, nullptr, true);
	}
	else {
		// 일반 바위: GetDropItemID/GetDropItemCount 사용 (돌 2개)
		GameObjectID dropItemID = GetDropItemID();
		int dropCount = GetDropItemCount();
		if (dropItemID != GOID_NONE && dropCount > 0) {
			for (int i = 0; i < dropCount; ++i) {
				float angle = (float)i / dropCount * 2.0f * 3.14159f;
				float offsetX = cosf(angle) * spreadRadius;
				float offsetY = sinf(angle) * spreadRadius;
				objMgr->CreateGameObject(dropItemID, tx + offsetX, ty + offsetY, nullptr, true);
			}
		}
	}
	objMgr->RemoveGameObject(this);
}

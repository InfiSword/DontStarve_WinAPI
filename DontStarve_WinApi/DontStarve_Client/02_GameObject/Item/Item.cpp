#include "99_Default/pch.h"
#include "Item.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../Component/Transform/Transform.h"
#include "../Component/Sprite/SpriteRenderer.h"

void Item::RegisterResources(ResourceManager* rm)
{
	if (!rm) return;
	GameObjectData d;
	d.type = GOBJ_ITEM;
	d.pivotX = 0.5f;
	d.pivotY = 1.0f;
	d.objectAssetBaseDirectory = L"Resource/Objects/ingredient";
	d.assetImageName = L"cutgrass01-0.png";   d.id = GOID_ITEM_CUT_NORMAL_GRASS;  rm->RegisterObjectResource(GOID_ITEM_CUT_NORMAL_GRASS, d);
	d.assetImageName = L"rocks01-0.png";      d.id = GOID_ITEM_NORMAL_ROCK;       rm->RegisterObjectResource(GOID_ITEM_NORMAL_ROCK, d);
	d.assetImageName = L"twigs01-0.png";      d.id = GOID_ITEM_NORMAL_TWIGS;      rm->RegisterObjectResource(GOID_ITEM_NORMAL_TWIGS, d);
	d.assetImageName = L"Tree1_log.png";      d.id = GOID_ITEM_NORMAL_TREE_LOG;   rm->RegisterObjectResource(GOID_ITEM_NORMAL_TREE_LOG, d);
	d.assetImageName = L"Gold_Item.png";      d.id = GOID_ITEM_GOLD_ROCK;         rm->RegisterObjectResource(GOID_ITEM_GOLD_ROCK, d);
	d.assetImageName = L"rope01-0.png";       d.id = GOID_ITEM_ROPE;              rm->RegisterObjectResource(GOID_ITEM_ROPE, d);
	d.assetImageName = L"cutstone01-0.png";  d.id = GOID_ITEM_CUT_NORMAL_STONE;  rm->RegisterObjectResource(GOID_ITEM_CUT_NORMAL_STONE, d);
	d.assetImageName = L"meat-0.png";         d.id = GOID_ITEM_MEAT;              rm->RegisterObjectResource(GOID_ITEM_MEAT, d);
	d.assetImageName = L"Berry.png";           d.id = GOID_ITEM_BERRY;             rm->RegisterObjectResource(GOID_ITEM_BERRY, d);
}

Item::Item(GameObjectType type, GameObjectID id, const std::wstring& name, const std::wstring& desc,
	const std::wstring& resourcePath, const std::wstring& imagePath,
	float x, float y, float pivotX, float pivotY, Direction dir, bool isActive, bool isInteractive)
    : GameObject(type, id, resourcePath, imagePath, isActive, isInteractive),
      transform(nullptr), spriteRenderer(nullptr)
{
    m_name = name;
    m_description = desc;
    
    // Transform 컴포넌트 추가
    Transform* trans = AddComponent<Transform>();
    trans->SetPosition(x, y);
    trans->SetPivot(pivotX, pivotY);
    trans->SetDirection(dir);
    
    // SpriteRenderer 컴포넌트 추가
    SpriteRenderer* sprite = AddComponent<SpriteRenderer>();
    sprite->SetLayer(LAYER_WORLD_OBJECT);
    if (!imagePath.empty()) {
		ResourceManager* pRM = ResourceManager::GetInstance();
		const GameObjectData* data = pRM->GetObjectResourceInfo(id);
		std::wstring fullPath = data ? pRM->BuildResourcePath(data->objectAssetBaseDirectory, L"", imagePath) : L"";

		if (!fullPath.empty()) {
			if (auto handle = pRM->LoadSprite(fullPath)) {
				sprite->SetSprite(handle);
			}
		}
    }
}

Item::~Item() 
{
  
}

void Item::Init()
{
    GameObject::Init();
    
    // Transform 컴포넌트 캐싱
    transform = GetComponent<Transform>();
    spriteRenderer = GetComponent<SpriteRenderer>();
}

void Item::Update(float deltaTime)
{
	// 부모 클래스의 Update() 호출하여 컴포넌트 업데이트
	GameObject::Update(deltaTime);
}

void Item::Release()
{
	// Item 전용 정리 작업
	transform = nullptr;
	spriteRenderer = nullptr;
	
	// 부모 클래스의 Release() 호출하여 컴포넌트 정리
	GameObject::Release();
}




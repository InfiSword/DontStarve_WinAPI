#include "99_Default/pch.h"
#include "Item.h"
#include "../../01_Manager/InventoryManager/InventoryManager.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"
#include "../Component/Transform/Transform.h"
#include "../Component/Sprite/SpriteRenderer.h"

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
		std::wstring fullPath = pRM->BuildObjectResourcePath(id, L"", imagePath);

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




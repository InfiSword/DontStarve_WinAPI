#include "99_Default/pch.h"
#include "Item.h"
#include "../Component/Transform/Transform.h"
#include "../Component/Sprite/SpriteRenderer.h"
#include "../../01_Manager/ResourceManager/ResourceManager.h"

Item::Item(GameObjectID id, const std::wstring& name, const std::wstring& desc,
           const std::wstring& baseDir, const std::wstring& imageName,
           float x, float y, float pivotX, float pivotY, Direction _dir, bool isActive, bool isInteractive)
    : Entity(id, x, y, pivotX, pivotY, _dir, baseDir, imageName, isActive, isInteractive),
      m_itemName(name), m_description(desc)
{
	m_type = GO_TYPE_ITEM;
}

Item::~Item()
{
}

void Item::Init()
{
    Entity::Init();
}

void Item::Release()
{
    Entity::Release();
}
